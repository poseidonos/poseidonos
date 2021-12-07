/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "telemetry_manager_service.h"

#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include <string>

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
TelemetryManagerService::TelemetryManagerService(void)
{
    // TODO read telemetry config
    string server_address("0.0.0.0:50051");

    // new grpc server setting
    telemetryManagerServerThread = new std::thread(&TelemetryManagerService::CreateTelemetryServer, this, server_address);

    server = nullptr;
    serverAddress = server_address;
    enabledService = true;

    metricList = new MetricPublishRequest;
    curMetricListHead = 0;
}

TelemetryManagerService::~TelemetryManagerService(void)
{
    // close grpc server
    server->Shutdown();

    POS_TRACE_INFO((int)POS_EVENT_ID::TELEMETRY_DISABLED,
        "Close Telemetry manger & shut down gRPC Server");

    delete metricList;
}

void
TelemetryManagerService::CreateTelemetryServer(std::string address)
{
    ::grpc::ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(this);

    server = builder.BuildAndStart();
    server->Wait();
}

void
TelemetryManagerService::_IncreaseMetricListHead(void)
{
    curMetricListHead++;

    if(curMetricListHead >= MAX_SAVE_METRIC_COUNT)
    {
        POS_TRACE_INFO((int)POS_EVENT_ID::TELEMETRY_ERROR_MSG,
        "overlap telemety metric");
    }

    curMetricListHead = curMetricListHead % MAX_SAVE_METRIC_COUNT;
}

int
TelemetryManagerService::_FindMetric(string metricName)
{
    int ret = NOT_FOUND;

    auto it = metricMap.find(metricName);
    if (it != metricMap.end())
    {
        ret = it->second;
    }

    return ret;
}

// Publisher -> Manager
::grpc::Status
TelemetryManagerService::MetricPublish(::grpc::ServerContext* context, const ::MetricPublishRequest* request, ::MetricPublishResponse* response)
{
    ::grpc::Status ret = ::grpc::Status::OK;
    int totalReceivedMetricCnt = 0;

    int requesetedMetricsSize = request->metrics_size();

    for (int iterator = 0; iterator < requesetedMetricsSize; iterator++)
    {
        const Metric& metric = request->metrics(iterator);

        int metricIdx = _FindMetric(metric.name());

        if (metricIdx == NOT_FOUND)
        {
            Metric* newMetric = metricList->add_metrics();

            metricMap.insert({metric.name(), curMetricListHead});
            newMetric->CopyFrom(metric);

            continue;
        }

        MetricTypes type = metric.type();
        Metric* metricFound = metricList->mutable_metrics(metricIdx);

        switch (type)
        {
            case MetricTypes::COUNTER:
            {
                int counterValue = metricFound->countervalue() + metric.countervalue();
                metricFound->CopyFrom(metric);

                metricFound->set_countervalue(counterValue);
                break;
            }

            case MetricTypes::GUAGE:
            {
                metricFound->CopyFrom(metric);
                break;
            }

            default:
            {
                POS_TRACE_WARN(EID(TELEMETRY_ERROR_MSG), "no avaliable space");
                const string errorMsg = "no avaliable space";
                return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, errorMsg);
                break;
            }
        }
    }

    response->set_totalreceivedmetrics(totalReceivedMetricCnt);

    return ret;
}

// Collector -> Manager
::grpc::Status
TelemetryManagerService::MetricCollect(::grpc::ServerContext* context, const ::MetricCollectRequest* request, ::MetricCollectResponse* response)
{
    ::grpc::Status ret = ::grpc::Status::OK;

    for (int iter = 0; iter < metricList->metrics_size(); iter++)
    {
        Metric* metric = response->add_metrics();
        metric->CopyFrom(metricList->metrics(iter));
    }

    metricMap.clear();
    curMetricListHead = 0;

    return ret;
}

bool
TelemetryManagerService::StartService(void)
{
    enabledService = true;
    POS_TRACE_INFO(EID(TELEMETRY_DEBUG_MSG), "[Telemetry Manager] Start Telemetry Manager");
    return true;
}
bool
TelemetryManagerService::StopService(void)
{
    enabledService = false;
    POS_TRACE_INFO(EID(TELEMETRY_DEBUG_MSG), "[Telemetry Manager] Stop Telemetry Manager");
    return true;
}

std::string
TelemetryManagerService::GetServerAddr(void)
{
    return serverAddress;
}

} // namespace pos

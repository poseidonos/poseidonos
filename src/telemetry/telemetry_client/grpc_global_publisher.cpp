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
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
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

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/telemetry/telemetry_client/grpc_global_publisher.h"
#include "src/telemetry/telemetry_manager/telemetry_manager_service.h"

namespace pos
{
GrpcGlobalPublisher::GrpcGlobalPublisher(void)
{
    std::string serverAddr = TelemetryManagerServiceSingletone::Instance()->GetServerAddr();
    std::shared_ptr<Channel> channel = grpc::CreateChannel(serverAddr, grpc::InsecureChannelCredentials());
    telemetryManager = TelemetryManager::NewStub(channel);
}

GrpcGlobalPublisher::~GrpcGlobalPublisher(void)
{
}

int
GrpcGlobalPublisher::PublishToServer(MetricUint32& metric)
{
    PublishRequest cliPublishReq;
    // set data
    TelemetryGeneralMetric gmReq;
    gmReq.set_id(metric.GetId());
    gmReq.set_time(metric.GetTimeString());
    gmReq.set_value(metric.GetValue());

    return _SendMessage(cliPublishReq);
}

int
GrpcGlobalPublisher::PublishToServer(MetricString& metric)
{
    ////// TODO for String type value
    // TODO: need new type of ProtoBuf for string value
    PublishRequest cliPublishReq;
    // set data
    TelemetryGeneralMetricString gmReq;
    gmReq.set_id(metric.GetId());
    gmReq.set_time(metric.GetTimeString());
    gmReq.set_value(metric.GetValue());

    return _SendMessage(cliPublishReq);
}

int
GrpcGlobalPublisher::_SendMessage(PublishRequest& cliPublishReq)
{
    // TODO: TelemetryGeneralMetric -> PublishRequest
    PublishResponse cliPublishRes;
    ClientContext cliContext;

    const string errorMsg = "[TelemetryClient] gRPC Publishing Error";
    Status status = ::grpc::Status(StatusCode::INVALID_ARGUMENT, errorMsg);
    status = telemetryManager->publish(&cliContext, cliPublishReq, &cliPublishRes);
    if (status.ok() != true)
    {
        POS_TRACE_ERROR(EID(TELEMETRY_CLIENT_ERROR), "[TelemetryClient] Failed to send PublishRequest by gRPC, errorcode:{}, errormsg:{}", status.error_code(), status.error_message());
        return -1;
    }
    return 0;
}

} // namespace pos

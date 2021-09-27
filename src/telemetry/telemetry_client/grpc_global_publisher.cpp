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
#include <google/protobuf/timestamp.pb.h>

namespace pos
{
GrpcGlobalPublisher::GrpcGlobalPublisher(TelemetryManagerService* telemetryManager_, std::shared_ptr<grpc::Channel> channel_)
{
    telemetryManager = telemetryManager_;
    if (telemetryManager == nullptr)
    {
        telemetryManager = TelemetryManagerServiceSingletone::Instance();
    }
    std::string serverAddr = telemetryManager->GetServerAddr();
    std::shared_ptr<grpc::Channel> channel = channel_;
    if (channel == nullptr)
    {
        channel = grpc::CreateChannel(serverAddr, grpc::InsecureChannelCredentials());
    }
    tmStub = TelemetryManager::NewStub(channel);
}

GrpcGlobalPublisher::~GrpcGlobalPublisher(void)
{
}

int
GrpcGlobalPublisher::PublishToServer(MetricUint32& metric)
{
    PublishRequest* cliPublishReq = new PublishRequest;
    TelemetryGeneralMetric* gmReq = new TelemetryGeneralMetric();
    google::protobuf::Timestamp* tt = new google::protobuf::Timestamp();
    tt->set_seconds(metric.GetTime());
    gmReq->set_allocated_time(tt);
    gmReq->set_id(metric.GetId());
    gmReq->set_value(metric.GetValue());

    cliPublishReq->set_allocated_generalmetric(gmReq);
    int ret = _SendMessage(cliPublishReq);
    delete cliPublishReq;
    return ret;
}

int
GrpcGlobalPublisher::PublishToServer(MetricString& metric)
{
    PublishRequest* cliPublishReq = new PublishRequest;
    TelemetryGeneralMetricString* gmReq = new TelemetryGeneralMetricString();
    google::protobuf::Timestamp* tt = new google::protobuf::Timestamp();
    tt->set_seconds(metric.GetTime());
    gmReq->set_allocated_time(tt);
    gmReq->set_id(metric.GetId());
    gmReq->set_value(metric.GetValue());

    cliPublishReq->set_allocated_generalmetricstring(gmReq);
    int ret = _SendMessage(cliPublishReq);
    delete cliPublishReq;
    return ret;
}

int
GrpcGlobalPublisher::_SendMessage(PublishRequest* cliPublishReq)
{
    PublishResponse cliPublishRes;
    grpc::ClientContext cliContext;

    grpc::Status status = tmStub->publish(&cliContext, *cliPublishReq, &cliPublishRes);
    if (status.ok() != true)
    {
        POS_TRACE_ERROR(EID(TELEMETRY_CLIENT_ERROR), "[TelemetryClient] Failed to send PublishRequest by gRPC, errorcode:{}, errormsg:{}", status.error_code(), status.error_message());
        return -1;
    }
    else
    {
        if (cliPublishRes.successful() == false)
        {
            POS_TRACE_ERROR(EID(TELEMETRY_CLIENT_ERROR), "[TelemetryClient] TelemetryManager responsed with packet data error");
            return -1;
        }
    }
    return 0;
}

} // namespace pos

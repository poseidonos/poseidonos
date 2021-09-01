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
    string server_address("0.0.0.0:50051");

    // new grpc server setting
    telemetryManagerServerThread = new std::thread(&TelemetryManagerService::CreateTelemetryServer, this, server_address);

    server = nullptr;
    serverAddress = server_address;
    enabledService = true;
}

TelemetryManagerService::~TelemetryManagerService(void)
{
    // close grpc server
    server->Shutdown();
}

void
TelemetryManagerService::CreateTelemetryServer(std::string address)
{
    ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(this);

    server = builder.BuildAndStart();
    server->Wait();
}

::grpc::Status
TelemetryManagerService::configure(
    ::grpc::ServerContext* context,
    const ConfigureMetadataRequest* request,
    ConfigureMetadataResponse* response)
{
    ::grpc::Status ret = ::grpc::Status::OK;

    if (enabledService == true)
    {
        response->set_successful(true);
        response->set_collect_latency_ms(1);
        POS_TRACE_DEBUG(EID(TELEMETRY_DEBUG_MSG), "configure has been called");
    }
    else
    {
        response->set_successful(false);
        response->set_collect_latency_ms(1);

        POS_TRACE_DEBUG(EID(TELEMETRY_DISABLED), "Disabled Telemetry Manager");
        const string errorMsg = "Disabled Telemetry Manager";
        ret = ::grpc::Status(StatusCode::UNIMPLEMENTED, errorMsg);
    }

    return ret;
}

::grpc::Status
TelemetryManagerService::publish(
    ::grpc::ServerContext* context,
    const PublishRequest* request,
    PublishResponse* response)
{
    ::grpc::Status ret = ::grpc::Status::OK;

    if (enabledService == true)
    {
        response->set_successful(true);
        POS_TRACE_DEBUG(EID(TELEMETRY_DEBUG_MSG), "publish has been called");
    }
    else
    {
        response->set_successful(false);

        POS_TRACE_DEBUG(EID(TELEMETRY_DISABLED), "Disabled Telemetry Manager");
        const string errorMsg = "Disabled Telemetry Manager";
        ret = ::grpc::Status(StatusCode::UNIMPLEMENTED, errorMsg);
    }

    return ret;
}

::grpc::Status
TelemetryManagerService::collect(
    ::grpc::ServerContext* context,
    const ::CollectRequest* request,
    ::CollectResponse* response)
{
    ConfigureMetadataRequest* collectReq = new ConfigureMetadataRequest();
    ::grpc::Status ret = ::grpc::Status::OK;

    if (enabledService == true)
    {
        collectReq->set_git_hash("some-git");
        collectReq->set_host_name("some-host");
        collectReq->set_host_type("some-type");
        collectReq->set_ip_addr("some-ip");
        collectReq->set_application_name("some-app");
    }
    else
    {
        collectReq->set_git_hash("");
        collectReq->set_host_name("");
        collectReq->set_host_type("");
        collectReq->set_ip_addr("");
        collectReq->set_application_name("");

        POS_TRACE_DEBUG(EID(TELEMETRY_DISABLED), "Disabled Telemetry Manager");
        const string errorMsg = "Disabled Telemetry Manager";
        ret = ::grpc::Status(StatusCode::UNIMPLEMENTED, errorMsg);
    }

    response->set_allocated_metadata(collectReq);

    POS_TRACE_DEBUG(EID(TELEMETRY_DEBUG_MSG), "collect has been called");
    return ret;
}

::grpc::Status
TelemetryManagerService::enable(
    ::grpc::ServerContext* context,
    const ::EnableRequest* request,
    ::EnableResponse* response)
{
    ::grpc::Status ret = ::grpc::Status::OK;

    if (enabledService == true)
    {
        response->set_successful(true);
        POS_TRACE_DEBUG(EID(TELEMETRY_DEBUG_MSG), "enable has been called");
    }
    else
    {
        response->set_successful(false);

        POS_TRACE_DEBUG(EID(TELEMETRY_DISABLED), "Disabled Telemetry Manager");
        const string errorMsg = "Disabled Telemetry Manager";
        ret = ::grpc::Status(StatusCode::UNIMPLEMENTED, errorMsg);
    }

    return ret;
}

::grpc::Status
TelemetryManagerService::disable(
    ::grpc::ServerContext* context,
    const ::DisableRequest* request,
    ::DisableResponse* response)
{
    ::grpc::Status ret = ::grpc::Status::OK;

    if (enabledService == true)
    {
        response->set_successful(true);
        POS_TRACE_DEBUG(EID(TELEMETRY_DEBUG_MSG), "disable has been called");
    }
    else
    {
        response->set_successful(false);

        POS_TRACE_DEBUG(EID(TELEMETRY_DISABLED), "Disabled Telemetry Manager");
        const string errorMsg = "Disabled Telemetry Manager";
        ret = ::grpc::Status(StatusCode::UNIMPLEMENTED, errorMsg);
    }

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

} // namespace pos
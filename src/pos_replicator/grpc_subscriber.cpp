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

#include "grpc_subscriber.h"

#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include <string>
#include <thread>

#include "grpc_service/grpc_health.h"
#include "src/include/grpc_server_socket_address.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/master_context/config_manager.h"

namespace pos
{
GrpcSubscriber::GrpcSubscriber(ConfigManager* configManager)
{
    std::string address;
    int ret = configManager->GetValue("replicator", "ha_subscriber_address",
        static_cast<void*>(&address), CONFIG_TYPE_STRING);
    if (ret != 0)
    {
        POS_TRACE_INFO(EID(HA_DEBUG_MSG),
            "Failed to read grpc subscriber address from config file, Address will be set defined in the \"grpc_server_socket_address.h\"");
        address = GRPC_HA_SUB_SERVER_SOCKET_ADDRESS;
    }

    healthChecker = new GrpcHealth();
    new std::thread(&GrpcSubscriber::RunServer, this, address);
    POS_TRACE_INFO(EID(HA_DEBUG_MSG), "Replicator subscriber has been initialized successfully");
}

GrpcSubscriber::~GrpcSubscriber(void)
{
    haGrpcServer->Shutdown();
    if (healthChecker != nullptr)
    {
        delete healthChecker;
        healthChecker = nullptr;
    }

    POS_TRACE_INFO(EID(HA_DEBUG_MSG), "POS GrpcServer has been destructed");
}

void
GrpcSubscriber::RunServer(std::string address)
{
    ::grpc::ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(healthChecker);
    POS_TRACE_INFO(EID(HA_DEBUG_MSG), "Registering HealthCheck service and Starting GrpcServer at {}", address);

    haGrpcServer = builder.BuildAndStart();
    std::cout << "Grpc Replicator server listening on " << address << std::endl;
    haGrpcServer->Wait();
}
} // namespace pos

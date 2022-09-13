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

#include "grpc_publisher.h"

#include <memory>
#include <string>

#include "src/include/grpc_server_socket_address.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/master_context/config_manager.h"

namespace std
{
class thread;
}

namespace pos
{
GrpcPublisher::GrpcPublisher(std::shared_ptr<grpc::Channel> channel_, ConfigManager* configManager)
{
    std::string serverAddr;
    int ret = configManager->GetValue("replicator", "ha_publisher_address",
        static_cast<void*>(&serverAddr), CONFIG_TYPE_STRING);
    if (ret != 0)
    {
        POS_TRACE_INFO(static_cast<int>(EID(HA_DEBUG_MSG)),
            "Failed to read grpc publisher address from config file, Address will be set defined in the \"grpc_server_socket_address.h\"");
        serverAddr = GRPC_HA_PUB_SERVER_SOCKET_ADDRESS;
    }

    std::shared_ptr<grpc::Channel> channel = channel_;
    if (channel == nullptr)
    {
        channel = grpc::CreateChannel(serverAddr, grpc::InsecureChannelCredentials());
    }

    stub = ::replicator_rpc::ReplicatorIo::NewStub(channel);
    POS_TRACE_INFO(EID(HA_DEBUG_MSG), "Replicator publisher has been initialized with the channel newly established on {}", serverAddr);
}

GrpcPublisher::~GrpcPublisher(void)
{
    POS_TRACE_INFO(EID(HA_DEBUG_MSG), "GrpcPublisher has been destructed");
}

int
GrpcPublisher::PushHostWrite(uint64_t rba, uint64_t size, string volumeName,
    string arrayName, void* buf, uint64_t& lsn)
{
    ::grpc::ClientContext cliContext;
    replicator_rpc::PushHostWriteRequest* request = new replicator_rpc::PushHostWriteRequest;
    replicator_rpc::PushHostWriteResponse response;
    request->set_array_name(arrayName);
    request->set_volume_name(volumeName);
    request->set_rba(rba);
    request->set_num_blocks(size);

    /*
    [To do buffer process]    
    for (int iter = 0; iter < size; iter++)
    {
        replicator_rpc::Chunk* dataChunk = request->add_data();
        request->CopyFrom(buf);
    }
*/

    grpc::Status status = stub->PushHostWrite(&cliContext, *request, &response);

    if (status.ok() == false)
    {
        POS_TRACE_WARN(EID(HA_INVALID_RETURN_LSN), "Fail PushHostWrite");
        return EID(HA_INVALID_RETURN_LSN);
    }
    lsn = response.lsn();

    return EID(SUCCESS);
}

int
GrpcPublisher::CompleteUserWrite(uint64_t lsn, string volumeName, string arrayName)
{
    ::grpc::ClientContext cliContext;
    replicator_rpc::CompleteWriteRequest* request = new replicator_rpc::CompleteWriteRequest;
    replicator_rpc::CompleteWriteResponse response;

    request->set_lsn(lsn);
    request->set_array_name(arrayName);
    request->set_volume_name(volumeName);

    grpc::Status status = stub->CompleteWrite(&cliContext, *request, &response);

    if (status.ok() == false)
    {
        return EID(HA_COMPLETION_FAIL);
    }

    return EID(SUCCESS);
}

int
GrpcPublisher::CompleteWrite(uint64_t lsn, string volumeName, string arrayName)
{
    ::grpc::ClientContext cliContext;
    replicator_rpc::CompleteWriteRequest* request = new replicator_rpc::CompleteWriteRequest;
    replicator_rpc::CompleteWriteResponse response;

    request->set_lsn(lsn);
    request->set_array_name(arrayName);
    request->set_volume_name(volumeName);

    grpc::Status status = stub->CompleteWrite(&cliContext, *request, &response);

    if (status.ok() == false)
    {
        return EID(HA_COMPLETION_FAIL);
    }

    return EID(SUCCESS);
}

int
GrpcPublisher::CompleteRead(uint64_t lsn, uint64_t size, string volumeName, string arrayName, void* buf)
{
    ::grpc::ClientContext cliContext;
    replicator_rpc::CompleteReadRequest* request = new replicator_rpc::CompleteReadRequest;
    replicator_rpc::CompleteReadResponse response;

    request->set_lsn(lsn);
    request->set_array_name(arrayName);
    request->set_volume_name(volumeName);
    // to do set buffer

    grpc::Status status = stub->CompleteRead(&cliContext, *request, &response);

    if (status.ok() == false)
    {
        return EID(HA_COMPLETION_FAIL);
    }

    return EID(SUCCESS);
}
} // namespace pos

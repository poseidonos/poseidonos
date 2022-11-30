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

#pragma once

#include <grpc++/grpc++.h>

#include <list>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "proto/generated/cpp/replicator_rpc.grpc.pb.h"
#include "proto/generated/cpp/replicator_rpc.pb.h"
#include "src/helper/json/json_helper.h"
#include "src/include/grpc_server_socket_address.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
class MockReplicatorServer final : public replicator_rpc::ReplicatorIoService::Service
{
public:
    MockReplicatorServer(void);
    MockReplicatorServer(std::string _address);
    ~MockReplicatorServer(void);

    void RunServer(std::string address);

    virtual ::grpc::Status CompleteRead(
        ::grpc::ServerContext* context,
        const ::replicator_rpc::CompleteReadRequest* request,
        ::replicator_rpc::CompleteReadResponse* response) override;
    virtual ::grpc::Status CompleteWrite(
        ::grpc::ServerContext* context,
        const ::replicator_rpc::CompleteWriteRequest* request,
        ::replicator_rpc::CompleteWriteResponse* response) override;
    virtual ::grpc::Status PushHostWrite(
        ::grpc::ServerContext* context,
        const ::replicator_rpc::PushHostWriteRequest* request,
        ::replicator_rpc::PushHostWriteResponse* response) override;
    virtual ::grpc::Status PushDirtyLog(
        ::grpc::ServerContext* context,
        const ::replicator_rpc::PushDirtyLogRequest* request,
        ::replicator_rpc::PushDirtyLogResponse* response) override;
    virtual ::grpc::Status TransferDirtyLog(
        ::grpc::ServerContext* context,
        const ::replicator_rpc::TransferDirtyLogRequest* request,
        ::replicator_rpc::TransferDirtyLogResponse* response) override;
    virtual ::grpc::Status TransferHostWrite(
        ::grpc::ServerContext* context,
        const ::replicator_rpc::TransferHostWriteRequest* request,
        ::replicator_rpc::TransferHostWriteResponse* response) override;

private:
    std::unique_ptr<::grpc::Server> server;
    std::string address;
};
} // namespace pos

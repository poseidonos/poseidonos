// pos_rpc publisher, volume management

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

#include "dummy_ha_server.h"

#include <thread>
#include <string>
#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

namespace pos
{
DummyHaServer::DummyHaServer(void)
{
    // new grpc server setting
    string address(GRPC_HA_PUB_SERVER_SOCKET_ADDRESS);

    // new std::thread(&DummyHaServer::RunServer, this, address);
}

DummyHaServer::~DummyHaServer(void)
{
    dummyHAServer->Shutdown();
}


void
DummyHaServer::RunServer(std::string address)
{
    ::grpc::ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(this);

    dummyHAServer = builder.BuildAndStart();
    dummyHAServer->Wait();
}

::grpc::Status
DummyHaServer::CompleteRead(
    ::grpc::ServerContext* context, const ::replicator_rpc::CompleteReadRequest* request,::replicator_rpc::CompleteReadResponse* response)
{
    return ::grpc::Status::OK;
}

::grpc::Status
DummyHaServer::CompleteWrite(
    ::grpc::ServerContext* context, const ::replicator_rpc::CompleteWriteRequest* request,
    ::replicator_rpc::CompleteWriteResponse* response)
{
    return ::grpc::Status::OK;
}

::grpc::Status
DummyHaServer::PushHostWrite(
    ::grpc::ServerContext* context, const ::replicator_rpc::PushHostWriteRequest* request,
    ::replicator_rpc::PushHostWriteResponse* response)
{
    // dummy respoane lsn = rba
    response->set_lsn(request->rba());
    return ::grpc::Status::OK;
}

::grpc::Status
DummyHaServer::PushDirtyLog(
    ::grpc::ServerContext* context, const ::replicator_rpc::PushDirtyLogRequest* request,
    ::replicator_rpc::PushDirtyLogResponse* response)
{
    return ::grpc::Status::OK;
}

::grpc::Status
DummyHaServer::TransferDirtyLog(
    ::grpc::ServerContext* context, const ::replicator_rpc::TransferDirtyLogRequest* request,
    ::replicator_rpc::TransferDirtyLogResponse* response)
{
    return ::grpc::Status::OK;
}

::grpc::Status
DummyHaServer::TransferHostWrite(
    ::grpc::ServerContext* context, const ::replicator_rpc::TransferHostWriteRequest* request,
    ::replicator_rpc::TransferHostWriteResponse* response)
{
    return ::grpc::Status::OK;
}

}
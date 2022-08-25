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

#include "dummy_ha_client.h"

namespace pos
{
DummyHaClient::DummyHaClient(std::shared_ptr<grpc::Channel> channel_)
{
    // new grpc server setting
    string serverAddr(GRPC_HA_SUB_SERVER_SOCKET_ADDRESS);

    std::shared_ptr<grpc::Channel> channel = channel_;
    if (channel == nullptr)
    {
        channel = grpc::CreateChannel(serverAddr, grpc::InsecureChannelCredentials());
    }

    PosIostub = ::pos_rpc::PosIo::NewStub(channel);

}

DummyHaClient::~DummyHaClient(void)
{
    
}

::grpc::Status
DummyHaClient::WriteBlocks(void)
{
    ::grpc::ClientContext cliContext;
    pos_rpc::WriteBlocksRequest* request = new pos_rpc::WriteBlocksRequest;
    pos_rpc::WriteBlocksResponse response;

    return PosIostub->WriteBlocks(&cliContext, *request, &response);
}

::grpc::Status
DummyHaClient::WriteHostBlocks(void)
{
    ::grpc::ClientContext cliContext;
    pos_rpc::WriteHostBlocksRequest* request = new pos_rpc::WriteHostBlocksRequest;
    pos_rpc::WriteHostBlocksResponse response;

    return PosIostub->WriteHostBlocks(&cliContext, *request, &response);
}

::grpc::Status
DummyHaClient::ReadBlocks(void)
{
    ::grpc::ClientContext cliContext;
    pos_rpc::ReadBlocksRequest* request = new pos_rpc::ReadBlocksRequest;
    pos_rpc::ReadBlocksResponse response;

    return PosIostub->ReadBlocks(&cliContext, *request, &response);

}

::grpc::Status
DummyHaClient::CompleteHostWrite(void)
{
    ::grpc::ClientContext cliContext;
    pos_rpc::CompleteHostWriteRequest* request = new pos_rpc::CompleteHostWriteRequest;
    pos_rpc::CompleteHostWriteResponse response;

    return PosIostub->CompleteHostWrite(&cliContext, *request, &response);
}

}
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

#include "protobuf_create_channel.h"

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
ProtoBufCreateChannel::ProtoBufCreateChannel(string serverAddress)
{
    std::shared_ptr<Channel> channel = grpc::CreateChannel(serverAddress, grpc::InsecureChannelCredentials());
    stub_ = TelemetryManager::NewStub(channel);
}
ProtoBufCreateChannel::~ProtoBufCreateChannel(void)
{
}
::grpc::Status
ProtoBufCreateChannel::PublishToServer(void)
{
    PublishRequest clientReq;
    PublishResponse clientRes;
    ClientContext clientContext;
    // set data ; To do

    Status status = stub_->publish(&clientContext, clientReq, &clientRes);
    return status;
}
::grpc::Status
ProtoBufCreateChannel::ConfigureToServer(void)
{
    ClientContext clientContext;
    ConfigureMetadataRequest clientReq;
    ConfigureMetadataResponse clientRes;

    clientReq.set_git_hash("some-git");
    clientReq.set_host_name("some-host");
    clientReq.set_host_type("some-type");
    clientReq.set_ip_addr("some-ip");
    clientReq.set_application_name("poseidon");

    Status status = stub_->configure(&clientContext, clientReq, &clientRes);
    return status;
}
::grpc::Status
ProtoBufCreateChannel::CollectToServer(void)
{
    ClientContext clientContext;
    CollectRequest clientReq;
    CollectResponse clientRes;

    Status status = stub_->collect(&clientContext, clientReq, &clientRes);
    return status;
}
::grpc::Status
ProtoBufCreateChannel::EnableToServer(void)
{
    ClientContext clientContext;
    EnableRequest clientReq;
    EnableResponse clientRes;

    Status status = stub_->enable(&clientContext, clientReq, &clientRes);
    return status;
}
::grpc::Status
ProtoBufCreateChannel::DisableToServer(void)
{
    ClientContext clientContext;
    DisableRequest clientReq;
    DisableResponse clientRes;

    Status status = stub_->disable(&clientContext, clientReq, &clientRes);
    return status;
}

} // namespace pos

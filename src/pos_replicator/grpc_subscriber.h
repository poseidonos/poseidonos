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

#pragma once

#include "src/helper/json/json_helper.h"
#include "src/logger/logger.h"
#include "src/include/pos_event_id.h"
#include "proto/generated/cpp/pos_rpc.grpc.pb.h"
#include "proto/generated/cpp/pos_rpc.pb.h"

#include <list>
#include <map>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace pos
{
class GrpcSubscriber final : public pos_rpc::PosIo::Service
{
public:
    GrpcSubscriber(void);
    ~GrpcSubscriber(void);

    virtual ::grpc::Status WriteBlocks(
        ::grpc::ServerContext* context, 
        const pos_rpc::WriteBlocksRequest* request, 
        pos_rpc::WriteBlocksResponse* response) override;

    virtual ::grpc::Status WriteHostBlocks(
        ::grpc::ServerContext* context, 
        const pos_rpc::WriteHostBlocksRequest* request, 
        pos_rpc::WriteHostBlocksResponse* response) override;

    virtual ::grpc::Status ReadBlocks(
        ::grpc::ServerContext* context, 
        const pos_rpc::ReadBlocksRequest* request, 
        pos_rpc::ReadBlocksResponse* response) override;

    virtual ::grpc::Status CompleteHostWrite(
        ::grpc::ServerContext* context, 
        const pos_rpc::CompleteHostWriteRequest* request, 
        pos_rpc::CompleteHostWriteResponse* response) override;

    void RunServer(std::string address);

private:
    ::grpc::Status _CheckArgumentValidityAndUpdateIndex(std::pair<std::string, int> arraySet,
            std::pair<std::string, int> volumeSet);

    std::unique_ptr<::grpc::Server> posIoGrpcServer;
};
}
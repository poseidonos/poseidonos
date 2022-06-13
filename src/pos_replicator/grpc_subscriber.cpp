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

#include <thread>
#include <string>
#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include "src/pos_replicator/posreplicator_manager.h"

namespace pos
{
GrpcSubscriber::GrpcSubscriber(void)
{
    // new grpc server setting
    string address(GRPC_HA_SUB_SERVER_IP);

    new std::thread(&GrpcSubscriber::RunServer, this, address);
    POS_TRACE_INFO(EID(HA_DEBUG_MSG), "posIo GrpcServer has been initialized. Server address : {}", address);
}

GrpcSubscriber::~GrpcSubscriber(void)
{
    posIoGrpcServer->Shutdown();
    POS_TRACE_INFO(EID(HA_DEBUG_MSG), "posIo GrpcServer has been destructed");
}


void
GrpcSubscriber::RunServer(std::string address)
{
    ::grpc::ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(this);

    posIoGrpcServer = builder.BuildAndStart();
    posIoGrpcServer->Wait();
}

::grpc::Status
GrpcSubscriber::_CheckArgumentValidityAndUpdateIndex(std::pair<std::string, int> arraySet,
    std::pair<std::string, int> volumeSet)
{
    int ret = PosReplicatorManagerSingleton::Instance()->ConvertNametoIdx(arraySet, volumeSet);

    if (ret != EID(SUCCESS))
    {
        POS_TRACE_WARN(ret, "recieve invalid argument");
        const string errorMsg = "recieve invalid argument";
        return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, errorMsg);
    }
    return ::grpc::Status::OK;
}

::grpc::Status
GrpcSubscriber::WriteHostBlocks(::grpc::ServerContext* context,
    const pos_rpc::WriteHostBlocksRequest* request, pos_rpc::WriteHostBlocksResponse* response)
{
    std::pair<std::string, int> arraySet(request->array_name(), HA_INVALID_ARRAY_IDX);
    std::pair<std::string, int> volumeSet(request->volume_name(), HA_INVALID_VOLUME_IDX);

    ::grpc::Status ret = _CheckArgumentValidityAndUpdateIndex(arraySet, volumeSet);

    if (ret.ok() == false)
    {
        return ret;
    }

    PosReplicatorManagerSingleton::Instance()->UserVolumeWriteSubmission(request->lsn(), arraySet.second, volumeSet.second);
    return ::grpc::Status::OK;
}

::grpc::Status
GrpcSubscriber::WriteBlocks(::grpc::ServerContext* context,
    const pos_rpc::WriteBlocksRequest* request, pos_rpc::WriteBlocksResponse* response)
{
    std::pair<std::string, int> arraySet(request->array_name(), HA_INVALID_ARRAY_IDX);
    std::pair<std::string, int> volumeSet(request->volume_name(), HA_INVALID_VOLUME_IDX);

    ::grpc::Status ret = _CheckArgumentValidityAndUpdateIndex(arraySet, volumeSet);

    if (ret.ok() == false)
    {
        return ret;
    }
    
    char buffer[4096];
    PosReplicatorManagerSingleton::Instance()->HAIOSubmission(IO_TYPE::WRITE, arraySet.second, volumeSet.second,
        request->rba(), request->num_blocks(), buffer);
    return ::grpc::Status::OK;
}

::grpc::Status
GrpcSubscriber::ReadBlocks(::grpc::ServerContext* context,
    const pos_rpc::ReadBlocksRequest* request, pos_rpc::ReadBlocksResponse* response)
{
    std::pair<std::string, int> arraySet(request->array_name(), HA_INVALID_ARRAY_IDX);
    std::pair<std::string, int> volumeSet(request->volume_name(), HA_INVALID_VOLUME_IDX);

    ::grpc::Status ret = _CheckArgumentValidityAndUpdateIndex(arraySet, volumeSet);

    if (ret.ok() == false)
    {
        return ret;
    }

    PosReplicatorManagerSingleton::Instance()->HAIOSubmission(IO_TYPE::READ, arraySet.second, volumeSet.second,
        request->rba(), request->num_blocks(), nullptr);
    return ::grpc::Status::OK;
}

::grpc::Status
GrpcSubscriber::CompleteHostWrite(::grpc::ServerContext* context, const pos_rpc::CompleteHostWriteRequest* request,
    pos_rpc::CompleteHostWriteResponse* response)
{
    std::pair<std::string, int> arraySet(request->array_name(), HA_INVALID_ARRAY_IDX);
    std::pair<std::string, int> volumeSet(request->volume_name(), HA_INVALID_VOLUME_IDX);

    ::grpc::Status ret = _CheckArgumentValidityAndUpdateIndex(arraySet, volumeSet);

    if (ret.ok() == false)
    {
        return ret;
    }
    PosReplicatorManagerSingleton::Instance()->CompleteUserIO(request->lsn(), arraySet.second, volumeSet.second);
    return ::grpc::Status::OK;    
}

}
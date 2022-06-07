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

#include "grpc_volume_management.h"

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/pos_replicator/posreplicator_manager.h"

namespace std
{
class thread;
}

namespace pos
{

GrpcVolumeManagement::GrpcVolumeManagement(std::shared_ptr<grpc::Channel> channel_, IVolumeEventManager* volMgr)
{
    // new grpc server setting
    string address("0.0.0.0:50051");

    new std::thread(&GrpcVolumeManagement::RunServer, this, address);
    POS_TRACE_INFO(EID(HA_DEBUG_MSG), "GrpcSubscriber has been initialized. Server address : {}", address);

    volEventManger = volMgr;
}

GrpcVolumeManagement::~GrpcVolumeManagement(void)
{
    posMgrGrpcServer->Shutdown();
    POS_TRACE_INFO(EID(HA_DEBUG_MSG), "PosMgr GrpcServer has been destructed");
}

void
GrpcVolumeManagement::RunServer(std::string address)
{
    ::grpc::ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(this);

    posMgrGrpcServer = builder.BuildAndStart();
    posMgrGrpcServer->Wait();
}

::grpc::Status
GrpcVolumeManagement::CreateArray(::grpc::ServerContext* context,
        const ::pos_rpc::CreateArrayRequest* request, ::pos_rpc::PosResponse* response)
{
    return ::grpc::Status::OK;
}
::grpc::Status
GrpcVolumeManagement::DeleteArray(::grpc::ServerContext* context,
        const ::pos_rpc::DeleteArrayRequest* request, ::pos_rpc::PosResponse* response)
{
    return ::grpc::Status::OK;
}

::grpc::Status
GrpcVolumeManagement::CreateVolume(::grpc::ServerContext* context,
        const ::pos_rpc::CreateVolumeRequest* request, ::pos_rpc::PosResponse* response)
{
    return ::grpc::Status::OK;
}

::grpc::Status
GrpcVolumeManagement::DeleteVolume(::grpc::ServerContext* context,
        const ::pos_rpc::DeleteVolumeRequest* request, ::pos_rpc::PosResponse* response)
{
    return ::grpc::Status::OK;
}

::grpc::Status
GrpcVolumeManagement::MountVolume(::grpc::ServerContext* context,
        const ::pos_rpc::MountVolumeRequest* request, ::pos_rpc::PosResponse* response)
{
    return ::grpc::Status::OK;
}

::grpc::Status
GrpcVolumeManagement::UnmountVolume(::grpc::ServerContext* context,
        const ::pos_rpc::UnmountVolumeRequest* request, ::pos_rpc::PosResponse* response)
{
    return ::grpc::Status::OK;
}

::grpc::Status
GrpcVolumeManagement::UpdateVoluemMeta(::grpc::ServerContext* context,
        const ::pos_rpc::UpdateVoluemMetaRequest* request, ::pos_rpc::PosResponse* response)
{
    string arrayName = request->array_name();

    int ret = volEventManger->SaveVolumeMeta();

    if (ret != EID(SUCCESS))
    {
        const string errorMsg = "VolumeMeta Update Fail";
        return ::grpc::Status(::grpc::StatusCode::UNAVAILABLE, errorMsg);
    }

    return ::grpc::Status::OK;
}

::grpc::Status
GrpcVolumeManagement::GetArrayList(::grpc::ServerContext* context,
        const ::pos_rpc::GetArrayListRequest* request, ::pos_rpc::ArrayListResponse* response)
{
    return ::grpc::Status::OK;
}

::grpc::Status
GrpcVolumeManagement::GetVolumeList(::grpc::ServerContext* context,
        const ::pos_rpc::GetVolumeListRequest* request, ::pos_rpc::VolumeListResponse* response)
{
    return ::grpc::Status::OK;
}

}
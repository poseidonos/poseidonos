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

#include "grpc_pos_management.h"

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/volume/volume_base.h"
#include "src/volume/volume_service.h"

namespace pos
{
::grpc::Status
GrpcPosManagement::UpdateVoluemMeta(::grpc::ServerContext* context,
    const ::pos_rpc::UpdateVoluemMetaRequest* request, ::pos_rpc::PosResponse* response)
{
    string arrayName = request->array_name();
    IVolumeEventManager* volEventManger =
        VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);

    POS_TRACE_INFO(EID(HA_DEBUG_MSG), "Get UpdateVoluemMeta from grpc client");
    if (nullptr == volEventManger)
    {
        const string errorMsg = "Approached " + arrayName + " is not in use";
        return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, errorMsg);
    }

    int ret = volEventManger->SaveVolumeMeta();

    if (ret != EID(SUCCESS))
    {
        const string errorMsg = "VolumeMeta Update Fail (return code: " + std::to_string(ret) + ")";
        return ::grpc::Status(::grpc::StatusCode::UNAVAILABLE, errorMsg);
    }

    return ::grpc::Status::OK;
}

::grpc::Status
GrpcPosManagement::GetVolumeList(::grpc::ServerContext* context,
    const ::pos_rpc::GetVolumeListRequest* request, ::pos_rpc::VolumeListResponse* response)
{
    string arrayName = request->array_name();
    POS_TRACE_DEBUG(EID(HA_DEBUG_MSG), "Get GetVolumeList from grpc client");

    IVolumeInfoManager* volumeManager =
        VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);
    if (nullptr == volumeManager)
    {
        const string errorMsg = "Approached " + arrayName + " is not in use";
        return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, errorMsg);
    }

    VolumeList* volList = volumeManager->GetVolumeList();

    int idx = -1;
    while (true)
    {
        VolumeBase* vol = volList->Next(idx);
        if (nullptr == vol)
        {
            break;
        }

        pos_rpc::VolumeInfo* info = response->add_infos();
        info->set_volume_name(vol->GetVolumeName());
        info->set_total_capacity(vol->GetTotalSize());
        info->set_used_capacity(vol->UsedSize());
    }

    return ::grpc::Status::OK;
}
} // namespace pos

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
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
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

#include "src/network/nvmf_target_event_subscriber.hpp"

#include "spdk/ibof.h"
#include "src/sys_event/volume_event_publisher.h"

namespace pos
{
NvmfTargetEventSubscriber::NvmfTargetEventSubscriber(NvmfVolume* vol, std::string arrayName)
: VolumeEvent("NvmfTarget", arrayName),
  volume(vol)
{
    VolumeEventPublisherSingleton::Instance()->RegisterNvmfTargetSubscriber(this, arrayName);
}

NvmfTargetEventSubscriber::~NvmfTargetEventSubscriber(void)
{
    VolumeEventPublisherSingleton::Instance()->RemoveSubscriber(this, arrayName);
}

void
NvmfTargetEventSubscriber::CopyVolumeInfo(char* destInfo,
    const char* srcInfo, int len)
{
    strncpy(destInfo, srcInfo, len);
    destInfo[len] = '\0';
}

bool
NvmfTargetEventSubscriber::VolumeCreated(string volName, int volId,
    uint64_t volSizeByte, uint64_t maxIops, uint64_t maxBw, string arrayName)
{
    struct ibof_volume_info* vInfo = new ibof_volume_info;
    if (vInfo)
    {
        CopyVolumeInfo(vInfo->name, volName.c_str(), sizeof(vInfo->name) - 1);
        vInfo->id = volId;
        vInfo->size_mb = volSizeByte / MIB_IN_BYTE;
        vInfo->iops_limit = maxIops * KIOPS;
        vInfo->bw_limit = maxBw;
        CopyVolumeInfo(vInfo->array_name, arrayName.c_str(), arrayName.size());
        volume->VolumeCreated(vInfo);
        return true;
    }
    return false;
}

bool
NvmfTargetEventSubscriber::VolumeDeleted(string volName, int volId,
    uint64_t volSizeByte, string arrayName)
{
    struct ibof_volume_info* vInfo = new ibof_volume_info;
    if (vInfo)
    {
        CopyVolumeInfo(vInfo->name, volName.c_str(), sizeof(vInfo->name) - 1);
        CopyVolumeInfo(vInfo->array_name, arrayName.c_str(), arrayName.size());
        vInfo->id = volId;
        volume->VolumeDeleted(vInfo);
        return true;
    }
    return false;
}

bool
NvmfTargetEventSubscriber::VolumeMounted(string volName, string subNqn, int volId,
    uint64_t volSizeByte, uint64_t maxIops, uint64_t maxBw, string arrayName)
{
    struct ibof_volume_info* vInfo = new ibof_volume_info;
    if (vInfo)
    {
        CopyVolumeInfo(vInfo->name, volName.c_str(), sizeof(vInfo->name) - 1);
        CopyVolumeInfo(vInfo->nqn, subNqn.c_str(), sizeof(vInfo->nqn) - 1);
        CopyVolumeInfo(vInfo->array_name, arrayName.c_str(), arrayName.size());
        vInfo->id = volId;
        vInfo->size_mb = volSizeByte / MIB_IN_BYTE;
        vInfo->iops_limit = maxIops * KIOPS;
        vInfo->bw_limit = maxBw;
        volume->VolumeMounted(vInfo);
        return true;
    }
    return false;
}

bool
NvmfTargetEventSubscriber::VolumeUnmounted(string volName, int volId, string arrayName)
{
    struct ibof_volume_info* vInfo = new ibof_volume_info;
    if (vInfo)
    {
        CopyVolumeInfo(vInfo->name, volName.c_str(), sizeof(vInfo->name) - 1);
        CopyVolumeInfo(vInfo->array_name, arrayName.c_str(), arrayName.size());
        vInfo->id = volId;
        volume->VolumeUnmounted(vInfo);
        return true;
    }
    return false;
}

bool
NvmfTargetEventSubscriber::VolumeLoaded(string volName, int id,
    uint64_t totalSize, uint64_t maxIops, uint64_t maxBw, string arrayName)
{
    return VolumeCreated(volName, id, totalSize, maxIops, maxBw, arrayName);
}

bool
NvmfTargetEventSubscriber::VolumeUpdated(string volName, int volId,
    uint64_t maxIops, uint64_t maxBw, string arrayName)
{
    struct ibof_volume_info* vInfo = new ibof_volume_info;
    if (vInfo)
    {
        CopyVolumeInfo(vInfo->name, volName.c_str(), sizeof(vInfo->name) - 1);
        CopyVolumeInfo(vInfo->array_name, arrayName.c_str(), arrayName.size());
        vInfo->id = volId;
        vInfo->iops_limit = maxIops * KIOPS;
        vInfo->bw_limit = maxBw;
        volume->VolumeUpdated(vInfo);
        return true;
    }
    return false;
}

void
NvmfTargetEventSubscriber::VolumeDetached(vector<int> volList, string arrayName)
{
    if (!volList.empty())
    {
        volume->VolumeDetached(volList, arrayName);
    }
    return;
}

} // namespace pos

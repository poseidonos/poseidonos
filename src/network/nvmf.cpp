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

#include "src/network/nvmf.h"

#include "src/include/pos_event_id.hpp"
#include "src/io/frontend_io/unvmf_io_handler.h"
#include "src/logger/logger.h"

namespace pos
{
Nvmf::Nvmf(std::string arrayName, int arrayId)
: Nvmf(arrayName, arrayId, VolumeEventPublisherSingleton::Instance(), nullptr)
{
}

Nvmf::Nvmf(std::string arrayName, int arrayId, VolumeEventPublisher* volumeEventPublisher, NvmfVolumePos* inputNvmfVolume)
: VolumeEvent("NvmfTarget", arrayName, arrayId),
  volumeEventPublisher(volumeEventPublisher)
{
    if (nullptr != inputNvmfVolume)
    {
        volume = inputNvmfVolume;
    }
    volumeEventPublisher->RegisterSubscriber(this, arrayName, arrayId);
}

Nvmf::~Nvmf(void)
{
    volumeEventPublisher->RemoveSubscriber(this, arrayName, arrayId);
}

int
Nvmf::Init(void)
{
    unvmf_io_handler handler = {.submit = UNVMfSubmitHandler,
        .complete = UNVMfCompleteHandler};
    SetuNVMfIOHandler(handler);

    volume = new NvmfVolumePos(ioHandler);
    return 0;
}

void
Nvmf::Dispose(void)
{
    if (volume != nullptr)
    {
        delete volume;
        volume = nullptr;
    }
}

void
Nvmf::Shutdown(void)
{
    Dispose();
}

void
Nvmf::SetuNVMfIOHandler(unvmf_io_handler handler)
{
    if (ioHandler.submit || ioHandler.complete)
    {
        POS_EVENT_ID eventId =
            POS_EVENT_ID::IONVMF_OVERRIDE_UNVMF_IO_HANDLER;
        POS_TRACE_WARN(static_cast<int>(eventId), PosEventId::GetString(eventId));
    }
    ioHandler.submit = handler.submit;
    ioHandler.complete = handler.complete;
}

void
Nvmf::_CopyVolumeInfo(char* destInfo,
    const char* srcInfo, int len)
{
    strncpy(destInfo, srcInfo, len);
    destInfo[len] = '\0';
}

bool
Nvmf::VolumeCreated(string volName, int volId,
    uint64_t volSizeByte, uint64_t maxIops, uint64_t maxBw, string arrayName, int arrayId)
{
    struct pos_volume_info* vInfo = new pos_volume_info;
    if (vInfo)
    {
        _CopyVolumeInfo(vInfo->name, volName.c_str(), sizeof(vInfo->name) - 1);
        vInfo->id = volId;
        vInfo->array_id = arrayId;
        vInfo->size_mb = volSizeByte / MIB_IN_BYTE;
        vInfo->iops_limit = maxIops * KIOPS;
        vInfo->bw_limit = maxBw;
        _CopyVolumeInfo(vInfo->array_name, arrayName.c_str(), arrayName.size());
        volume->VolumeCreated(vInfo);
        return true;
    }
    return false;
}

bool
Nvmf::VolumeDeleted(string volName, int volId,
    uint64_t volSizeByte, string arrayName, int arrayId)
{
    struct pos_volume_info* vInfo = new pos_volume_info;
    if (vInfo)
    {
        _CopyVolumeInfo(vInfo->name, volName.c_str(), sizeof(vInfo->name) - 1);
        _CopyVolumeInfo(vInfo->array_name, arrayName.c_str(), arrayName.size());
        vInfo->id = volId;
        vInfo->array_id = arrayId;
        volume->VolumeDeleted(vInfo);
        return true;
    }
    return false;
}

bool
Nvmf::VolumeMounted(string volName, string subNqn, int volId,
    uint64_t volSizeByte, uint64_t maxIops, uint64_t maxBw, string arrayName, int arrayId)
{
    struct pos_volume_info* vInfo = new pos_volume_info;
    if (vInfo)
    {
        _CopyVolumeInfo(vInfo->name, volName.c_str(), sizeof(vInfo->name) - 1);
        _CopyVolumeInfo(vInfo->nqn, subNqn.c_str(), sizeof(vInfo->nqn) - 1);
        _CopyVolumeInfo(vInfo->array_name, arrayName.c_str(), arrayName.size());
        vInfo->id = volId;
        vInfo->array_id = arrayId;
        vInfo->size_mb = volSizeByte / MIB_IN_BYTE;
        vInfo->iops_limit = maxIops * KIOPS;
        vInfo->bw_limit = maxBw;
        volume->VolumeMounted(vInfo);
        return true;
    }
    return false;
}

bool
Nvmf::VolumeUnmounted(string volName, int volId, string arrayName, int arrayId)
{
    struct pos_volume_info* vInfo = new pos_volume_info;
    if (vInfo)
    {
        _CopyVolumeInfo(vInfo->name, volName.c_str(), sizeof(vInfo->name) - 1);
        _CopyVolumeInfo(vInfo->array_name, arrayName.c_str(), arrayName.size());
        vInfo->id = volId;
        vInfo->array_id = arrayId;
        volume->VolumeUnmounted(vInfo);
        return true;
    }
    return false;
}

bool
Nvmf::VolumeLoaded(string volName, int id,
    uint64_t totalSize, uint64_t maxIops, uint64_t maxBw, string arrayName, int arrayId)
{
    return VolumeCreated(volName, id, totalSize, maxIops, maxBw, arrayName, arrayId);
}

bool
Nvmf::VolumeUpdated(string volName, int volId,
    uint64_t maxIops, uint64_t maxBw, string arrayName, int arrayId)
{
    struct pos_volume_info* vInfo = new pos_volume_info;
    if (vInfo)
    {
        _CopyVolumeInfo(vInfo->name, volName.c_str(), sizeof(vInfo->name) - 1);
        _CopyVolumeInfo(vInfo->array_name, arrayName.c_str(), arrayName.size());
        vInfo->id = volId;
        vInfo->array_id = arrayId;
        vInfo->iops_limit = maxIops * KIOPS;
        vInfo->bw_limit = maxBw;
        volume->VolumeUpdated(vInfo);
        return true;
    }
    return false;
}

void
Nvmf::VolumeDetached(vector<int> volList, string arrayName, int arrayId)
{
    volume->VolumeDetached(volList, arrayName);
    return;
}

} // namespace pos

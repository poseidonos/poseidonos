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
  volume(inputNvmfVolume),
  volumeEventPublisher(volumeEventPublisher)
{
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
Nvmf::Flush(void)
{
    // no-op
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

void
Nvmf::_CopyVolumeEventBase(pos_volume_info* vInfo, VolumeEventBase* volEventBase)
{
    _CopyVolumeInfo(vInfo->name, volEventBase->volName.c_str(), sizeof(vInfo->name) - 1);
    _CopyVolumeInfo(vInfo->uuid, volEventBase->uuid.c_str(), volEventBase->uuid.size());
    _CopyVolumeInfo(vInfo->nqn, volEventBase->subnqn.c_str(), sizeof(vInfo->nqn) - 1);
    vInfo->id = volEventBase->volId;
    vInfo->size_mb = volEventBase->volSizeByte / MIB_IN_BYTE;
}

void
Nvmf::_CopyVolumeEventPerf(pos_volume_info* vInfo, VolumeEventPerf* volEventPerf)
{
    vInfo->iops_limit = volEventPerf->maxiops * KIOPS;
    vInfo->bw_limit = volEventPerf->maxbw;
}

void
Nvmf::_CopyVolumeArrayInfo(pos_volume_info* vInfo, VolumeArrayInfo* volArrayInfo)
{
    vInfo->array_id = arrayId;
    _CopyVolumeInfo(vInfo->array_name, arrayName.c_str(), arrayName.size());
}

bool
Nvmf::VolumeCreated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    struct pos_volume_info* vInfo = new pos_volume_info;
    if (vInfo)
    {
        _CopyVolumeEventBase(vInfo, volEventBase);
        _CopyVolumeEventPerf(vInfo, volEventPerf);
        _CopyVolumeArrayInfo(vInfo, volArrayInfo);

        volume->VolumeCreated(vInfo);
        return true;
    }
    return false;
}

bool
Nvmf::VolumeDeleted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo)
{
    struct pos_volume_info* vInfo = new pos_volume_info;
    if (vInfo)
    {
        _CopyVolumeEventBase(vInfo, volEventBase);
        _CopyVolumeArrayInfo(vInfo, volArrayInfo);

        volume->VolumeDeleted(vInfo);
        return true;
    }
    return false;
}

bool
Nvmf::VolumeMounted(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    struct pos_volume_info* vInfo = new pos_volume_info;
    if (vInfo)
    {
        _CopyVolumeEventBase(vInfo, volEventBase);
        _CopyVolumeEventPerf(vInfo, volEventPerf);
        _CopyVolumeArrayInfo(vInfo, volArrayInfo);

        volume->VolumeMounted(vInfo);
        return true;
    }
    return false;
}

bool
Nvmf::VolumeUnmounted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo)
{
    struct pos_volume_info* vInfo = new pos_volume_info;
    if (vInfo)
    {
        _CopyVolumeEventBase(vInfo, volEventBase);
        _CopyVolumeArrayInfo(vInfo, volArrayInfo);

        volume->VolumeUnmounted(vInfo);
        return true;
    }
    return false;
}

bool
Nvmf::VolumeLoaded(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    return VolumeCreated(volEventBase, volEventPerf, volArrayInfo);
}

bool
Nvmf::VolumeUpdated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    struct pos_volume_info* vInfo = new pos_volume_info;
    if (vInfo)
    {
        _CopyVolumeEventBase(vInfo, volEventBase);
        _CopyVolumeEventPerf(vInfo, volEventPerf);
        _CopyVolumeArrayInfo(vInfo, volArrayInfo);

        volume->VolumeUpdated(vInfo);
        return true;
    }
    return false;
}

void
Nvmf::VolumeDetached(vector<int> volList, VolumeArrayInfo* volArrayInfo)
{
    volume->VolumeDetached(volList, volArrayInfo->arrayName);
    return;
}

} // namespace pos

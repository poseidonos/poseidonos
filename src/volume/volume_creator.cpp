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

#include <string>
#include "src/volume/volume_creator.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/network/nvmf_target.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/volume/volume.h"
#include "src/volume/volume_list.h"

namespace pos
{

VolumeCreator::VolumeCreator(VolumeList& volumeList, std::string arrayName, int arrayID, VolumeEventPublisher* volumeEventPublisher)
: VolumeInterface(volumeList, arrayName, arrayID, volumeEventPublisher)
{
    vol = nullptr;
}

VolumeCreator::~VolumeCreator(void)
{
}

void
VolumeCreator::_CheckRequestValidity(string name, uint64_t size)
{
    CheckVolumeName(name);

    if (volumeList.GetID(name) >= 0)
    {
        POS_TRACE_WARN(static_cast<int>(POS_EVENT_ID::VOL_NAME_DUPLICATED),
                "Volume name is duplicated");
        throw static_cast<int>(POS_EVENT_ID::VOL_NAME_DUPLICATED);
    }

    _CheckVolumeSize(size);
}

void
VolumeCreator::_CreateVolume(string name, uint64_t size, uint64_t maxIops,
        uint64_t maxBw, uint64_t minIops, uint64_t minBw)
{
    vol = new Volume(arrayName, arrayID, name, size);
    if (vol == nullptr)
    {
        POS_TRACE_ERROR(static_cast<int>(POS_EVENT_ID::MEM_ALLOC_FAIL),
                "Fail to allocate memory");
        throw static_cast<int>(POS_EVENT_ID::MEM_ALLOC_FAIL);
    }

    _SetVolumeQos(vol, maxIops, maxBw, minIops, minBw);

    volumeList.Add(vol);
}

void
VolumeCreator::_NotificationVolumeEvent()
{
    _SetVolumeEventBase(vol);
    _SetVolumeEventPerf(vol);
    _SetVolumeArrayInfo();

    bool res = eventPublisher->NotifyVolumeCreated(&volumeEventBase, &volumeEventPerf, &volumeArrayInfo);

    if (res == false)
    {
        throw static_cast<int>(POS_EVENT_ID::DONE_WITH_ERROR);
    }
}

void
VolumeCreator::_SetUuid()
{
    NvmfTarget nvmfTarget;

    std::string uuid = nvmfTarget.GetPosBdevUuid(vol->ID, vol->GetArrayName());
    vol->SetUuid(uuid);
}

void
VolumeCreator::_RollbackCreatedVolume(int exceptionEvent)
{
    // [To do] cancel notificaiton
    try
    {
    if (vol != nullptr)
    {
        volumeList.Remove(vol->ID);
        delete vol;
    }
    }
    catch(int& exceptionEvent)
    {
        POS_TRACE_ERROR(static_cast<int>(POS_EVENT_ID::VOL_EVENT_ROLLBACK_FAIL),
            "Exception Fail POS EVENT ID : {}", exceptionEvent);
    }
}

int
VolumeCreator::Do(string name, uint64_t size, uint64_t maxIops,
        uint64_t maxBw, uint64_t minIops, uint64_t minBw)
{
    try
    {
        _CheckRequestValidity(name, size);
        _CreateVolume(name, size, maxIops, maxBw, minIops, minBw);

        _NotificationVolumeEvent();

        _SetUuid();

        int ret = _SaveVolumes();
        if (ret != static_cast<int>(POS_EVENT_ID::SUCCESS))
        {
            throw ret;
        }

        _PrintLogVolumeQos(vol, 0, 0, 0, 0);
    }
    catch (int& exceptionEvent)
    {
        _RollbackCreatedVolume(exceptionEvent);

        return exceptionEvent;
    }

    return static_cast<int>(POS_EVENT_ID::SUCCESS);
}

} // namespace pos

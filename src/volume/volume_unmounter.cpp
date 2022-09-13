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

#include "src/volume/volume_unmounter.h"

#include <string>

#include "src/event_scheduler/event_scheduler.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/volume/volume.h"
#include "src/volume/volume_list.h"

namespace pos
{

VolumeUnmounter::VolumeUnmounter(VolumeList& volumeList, std::string arrayName, int arrayID, VolumeEventPublisher* volumeEventPublisher, NvmfTarget* nvmfTarget_)
: VolumeInterface(volumeList, arrayName, arrayID, volumeEventPublisher)
{
    if (nvmfTarget_ == nullptr)
    {
        nvmfTarget = NvmfTargetSingleton::Instance();
    }
    else
    {
        nvmfTarget = nvmfTarget_;
    }
}

VolumeUnmounter::~VolumeUnmounter(void)
{
}

int
VolumeUnmounter::Do(string name)
{
    int ret = EID(VOL_NOT_FOUND);
    VolumeBase* vol = volumeList.GetVolume(name);
    if (vol == nullptr)
    {
        POS_TRACE_WARN(ret, "vol_name: {}", name);
        return ret;
    }

    vol->LockStatus();
    bool res = true;
    if (VolumeStatus::Unmounted != vol->GetStatus())
    {
        volumeList.WaitUntilIdleUserIo(vol->ID);
        _SetVolumeEventBase(vol);
        _SetVolumeEventPerf(vol);
        _SetVolumeArrayInfo();
        res = eventPublisher->NotifyVolumeUnmounted(&volumeEventBase, &volumeArrayInfo);
        ret = vol->Unmount();
    }
    else
    {
        ret = EID(UNMOUNT_VOL_ALREADY_UNMOUNTED);
        POS_TRACE_WARN(ret, "vol_name: {}", name);
    }
    vol->UnlockStatus();

    if (ret == EID(SUCCESS) && res == false)
    {
        POS_TRACE_WARN(EID(VOL_REQ_PROCESSED_BUT_ERROR_OCCURED),
            "vol_name:{}, array_name: {}", name, arrayName);
        return EID(VOL_REQ_PROCESSED_BUT_ERROR_OCCURED);
    }

    string subnqn = vol->GetSubnqn();
    nvmfTarget->RemoveSubsystemArrayName(subnqn);
    vol->SetSubnqn("");
    return ret;
}
} // namespace pos

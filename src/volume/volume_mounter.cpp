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

#include "src/volume/volume_mounter.h"

#include <string>

#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/volume/volume.h"
#include "src/volume/volume_list.h"

namespace pos
{

VolumeMounter::VolumeMounter(VolumeList& volumeList, std::string arrayName, int arrayID, VolumeEventPublisher* volumeEventPublisher, NvmfTarget* nvmfTarget_)
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

VolumeMounter::~VolumeMounter(void)
{
}

int
VolumeMounter::Do(string name, string subnqn)
{
    int ret = EID(UNMOUNT_VOL_DEBUG_MSG);
    VolumeBase* vol = volumeList.GetVolume(name);

    ret = _CheckIfExistVolume(vol);

    if (_CheckFailStatus(ret))
    {
        return ret;
    }

    ret = _CheckIfExistSubsystem(subnqn);

    if (_CheckFailStatus(ret))
    {
        return ret;
    }

    if (true == subnqn.empty())
    {
        subnqn = _GetSubsystemToMount(arrayName, arrayID);
        if ("" == subnqn)
        {
            ret = EID(MOUNT_VOL_SUBSYSTEM_NOT_FOUND);
            POS_TRACE_WARN(ret, "subnqn: {}, array_name: {}", subnqn, arrayName);
        }
    }

    if (_CheckFailStatus(ret))
    {
        return ret;
    }

    ret = _CheckAndSetSubsystemToArray(subnqn, arrayName);

    if (_CheckFailStatus(ret))
    {
        return ret;
    }

    ret = _MountVolume(vol, subnqn);

    if (_CheckFailStatus(ret))
    {
        return ret;
    }

    if (nvmfTarget->TryToAttachNamespace(subnqn, vol->ID, arrayName) == false)
    {
        ret = _RollBackVolumeMount(vol, subnqn);
    }
    else
    {
        vol->SetSubnqn(subnqn);
    }

    return ret;
}

bool
VolumeMounter::_CheckFailStatus(int checkValue)
{
    bool ret = true;

    if (checkValue == EID(SUCCESS))
    {
        ret = false;
    }

    return ret;
}

int
VolumeMounter::_CheckIfExistVolume(VolumeBase* vol)
{
    int ret = EID(SUCCESS);

    if (vol == nullptr)
    {
        ret = EID(VOL_NOT_FOUND);
        POS_TRACE_WARN(EID(VOL_NOT_FOUND), "null");
    }

    return ret;
}

int
VolumeMounter::_CheckIfExistSubsystem(string subnqn)
{
    int ret = EID(SUCCESS);

    if ((subnqn.empty() && nvmfTarget->CheckSubsystemExistance() == false) ||
        (!subnqn.empty() && nvmfTarget->FindSubsystem(subnqn) == nullptr))
    {
        ret = EID(MOUNT_VOL_SUBSYSTEM_NOT_FOUND);
        POS_TRACE_WARN(ret, "subnqn: {}, array_name: {}", subnqn, arrayName);
    }

    return ret;
}

string
VolumeMounter::_GetSubsystemToMount(string arrayName, int arrayID)
{
    string subnqn = "";
    struct spdk_nvmf_subsystem* subsystem = nvmfTarget->AllocateSubsystem(arrayName, arrayID);
    if (nullptr == subsystem)
    {
        return subnqn;
    }
    subnqn = nvmfTarget->GetVolumeNqn(subsystem);

    return subnqn;
}

int
VolumeMounter::_MountVolume(VolumeBase* vol, string subnqn)
{
    int ret = EID(SUCCESS);
    bool done = false;

    vol->LockStatus();

    if (VolumeStatus::Mounted != vol->GetStatus())
    {
        _SetVolumeEventBase(vol, subnqn);
        _SetVolumeEventPerf(vol);
        _SetVolumeArrayInfo();

        done = eventPublisher->NotifyVolumeMounted(&volumeEventBase, &volumeEventPerf, &volumeArrayInfo);

        if (done == true)
        {
            volumeList.InitializePendingIOCount(vol->ID, VolumeStatus::Mounted);
            ret = vol->Mount();
        }
        else
        {
            ret = EID(VOL_REQ_PROCESSED_BUT_ERROR_OCCURED);
            POS_TRACE_WARN(EID(VOL_REQ_PROCESSED_BUT_ERROR_OCCURED),
                "array_name: {}", arrayName);
        }
    }
    else
    {
        ret = EID(MOUNT_VOL_ALREADY_MOUNTED);
        POS_TRACE_WARN(ret, "vol_name: {}", vol->GetName());
    }

    vol->UnlockStatus();

    return ret;
}

int
VolumeMounter::_RollBackVolumeMount(VolumeBase* vol, string subnqn)
{
    int ret = EID(RESERVED);
    bool done = false;

    vol->LockStatus();

    _SetVolumeEventBase(vol);
    _SetVolumeArrayInfo();

    done = eventPublisher->NotifyVolumeUnmounted(&volumeEventBase, &volumeArrayInfo);

    if (done == false)
    {
        ret = EID(VOL_REQ_PROCESSED_BUT_ERROR_OCCURED);
        POS_TRACE_WARN(ret, "Failed to unmount volume during rollback mount: {}", vol->GetName());
    }

    ret = vol->Unmount();

    if (ret == EID(SUCCESS))
    {
        ret = EID(UNMOUNT_VOL_ALREADY_UNMOUNTED);
        POS_TRACE_WARN(ret, "vol_name: {}", vol->GetName());
    }

    ret = EID(MOUNT_VOL_UNABLE_TO_ATTACH_TO_NVMF);
    POS_TRACE_WARN(ret, "subnqn: {}", subnqn);

    vol->UnlockStatus();

    return ret;
}

int
VolumeMounter::_CheckAndSetSubsystemToArray(string subnqn, string volumeArrayName)
{
    int ret = EID(SUCCESS);
    string subnqnArrayName = nvmfTarget->GetSubsystemArrayName(subnqn);
    if (subnqnArrayName == "")
    {
        bool result = nvmfTarget->SetSubsystemArrayName(subnqn, volumeArrayName);
        if (false == result)
        {
            ret = EID(MOUNT_VOL_SUBSYSTEM_ALREADY_OCCUPIED);
            POS_TRACE_ERROR(ret, "array_name: {}, subnqn: {}", arrayName, subnqn);
        }
        else
        {
            int eventId = EID(IONVMF_SET_ARRAY_TO_SUBSYSTEM);
            POS_TRACE_INFO(eventId, "Successfully mapped array:{} to subsystem:{}", volumeArrayName, subnqn);
        }
    }
    else if (subnqnArrayName != volumeArrayName)
    {
        ret = EID(MOUNT_VOL_SUBSYSTEM_MISMATCH);
        POS_TRACE_ERROR(ret, "subsystem_array:{}, volume_array:{}", subnqnArrayName, volumeArrayName);
    }

    return ret;
}

} // namespace pos

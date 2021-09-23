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
    int ret = static_cast<int>(POS_EVENT_ID::VOL_UNMOUNTED);
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
            ret = static_cast<int>(POS_EVENT_ID::SUBSYSTEM_NOT_CREATED);
            POS_TRACE_WARN(ret, "No mataching subsystem exist for the volume in array:{}", arrayName);
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

    if (checkValue == static_cast<int>(POS_EVENT_ID::SUCCESS))
    {
        ret = false;
    }

    return ret;
}

int
VolumeMounter::_CheckIfExistVolume(VolumeBase* vol)
{
    int ret = static_cast<int>(POS_EVENT_ID::SUCCESS);

    if (vol == nullptr)
    {
        ret = static_cast<int>(POS_EVENT_ID::VOL_NOT_EXIST);

        POS_TRACE_WARN(ret, "The requested volume does not exist");
    }

    return ret;
}

int
VolumeMounter::_CheckIfExistSubsystem(string subnqn)
{
    int ret = static_cast<int>(POS_EVENT_ID::SUCCESS);

    if ((subnqn.empty() && nvmfTarget->CheckSubsystemExistance() == false) ||
        (!subnqn.empty() && nvmfTarget->FindSubsystem(subnqn) == nullptr))
    {
        ret = static_cast<int>(POS_EVENT_ID::SUBSYSTEM_NOT_CREATED);
        POS_TRACE_WARN(ret, "No subsystem:{} was created to attach the volume", subnqn);
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
    int ret = static_cast<int>(POS_EVENT_ID::SUCCESS);
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
            ret = static_cast<int>(POS_EVENT_ID::DONE_WITH_ERROR);
        }
    }
    else
    {
        ret = static_cast<int>(POS_EVENT_ID::VOL_ALD_MOUNTED);
        POS_TRACE_WARN(ret, "The volume already mounted: {}", vol->GetName());
    }

    vol->UnlockStatus();

    return ret;
}

int
VolumeMounter::_RollBackVolumeMount(VolumeBase* vol, string subnqn)
{
    int ret = static_cast<int>(POS_EVENT_ID::RESERVED);
    bool done = false;

    vol->LockStatus();

    _SetVolumeEventBase(vol);
    _SetVolumeArrayInfo();

    done = eventPublisher->NotifyVolumeUnmounted(&volumeEventBase, &volumeArrayInfo);

    if (done == false)
    {
        ret = static_cast<int>(POS_EVENT_ID::DONE_WITH_ERROR);
        POS_TRACE_WARN(ret, "Failed to unmount volume during rollback mount: {}", vol->GetName());
    }

    ret = vol->Unmount();

    if (ret == static_cast<int>(POS_EVENT_ID::SUCCESS))
    {
        ret = static_cast<int>(POS_EVENT_ID::VOL_ALD_UNMOUNTED);
        POS_TRACE_WARN(ret, "The volume already unmounted during rollback mount: {}", vol->GetName());
    }

    ret = static_cast<int>(POS_EVENT_ID::CANNOT_EXTEND_NSID);
    POS_TRACE_WARN(ret, "Can't extend more nsid to subsystem:{}", subnqn);

    vol->UnlockStatus();

    return ret;
}

int
VolumeMounter::_CheckAndSetSubsystemToArray(string subnqn, string volumeArrayName)
{
    int ret = static_cast<int>(POS_EVENT_ID::SUCCESS);
    string subnqnArrayName = nvmfTarget->GetSubsystemArrayName(subnqn);
    if (subnqnArrayName == "")
    {
        bool result = nvmfTarget->SetSubsystemArrayName(subnqn, volumeArrayName);
        if (false == result)
        {
            ret = static_cast<int>(POS_EVENT_ID::SUBSYSTEM_ALREADY_SET_ARRAY);
            POS_TRACE_ERROR(ret, "Fail to set array:{} to subsystem:{}. It already has configured an array", arrayName, subnqn);
        }
        else
        {
            POS_EVENT_ID eventId = POS_EVENT_ID::IONVMF_SET_ARRAY_TO_SUBSYSTEM;
            POS_TRACE_INFO(static_cast<int>(eventId), PosEventId::GetString(eventId), volumeArrayName, subnqn);
        }
    }
    else if (subnqnArrayName != volumeArrayName)
    {
        ret = static_cast<int>(POS_EVENT_ID::SUBSYSTEM_ARRAY_DOES_NOT_MATCH_VOLUME_ARRAY);
        POS_TRACE_ERROR(ret, "Subsystem's array:{} doesn't match volume's array:{}", subnqnArrayName, volumeArrayName);
    }

    return ret;
}

} // namespace pos

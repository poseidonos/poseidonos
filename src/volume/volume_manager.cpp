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

#include "src/volume/volume_manager.h"

#include <string>
#include <vector>

#include "spdk/nvmf.h"
#include "src/array/array.h"
#include "src/include/ibof_event_id.h"
#include "src/include/memory.h"
#include "src/io/general_io/rba_state_manager.h"
#include "src/logger/logger.h"
#include "src/mapper/mapper.h"
#include "src/network/nvmf_target.hpp"
#include "src/sys_event/volume_event_publisher.h"
#include "src/sys_info/space_info.h"
#include "volume_name_policy.h"
#if defined QOS_ENABLED_FE
#include "src/qos/qos_manager.h"
#endif

namespace ibofos
{
void
VolumeManager::Initialize(void)
{
    initialized = true;
    _LoadVolumes("");
}

void
VolumeManager::Dispose(void)
{
    initialized = false;
    DetachVolumes();
    volumes.Clear();
}

uint64_t
VolumeManager::EntireVolumeSize(void)
{
    uint64_t total_size = 0;

    int vol_cnt = volumes.Count();
    if (vol_cnt > 0)
    {
        int idx = -1;
        while (true)
        {
            VolumeBase* vol = volumes.Next(idx);
            if (vol == nullptr)
                break;

            total_size += vol->TotalSize();
        }
    }

    return total_size;
}

int
VolumeManager::GetVolumeSize(int volId, uint64_t& volSize)
{
    VolumeBase* vol = volumes.GetVolume(volId);
    if (vol != nullptr)
    {
        volSize = vol->TotalSize();
        return static_cast<int>(IBOF_EVENT_ID::SUCCESS);
    }

    volSize = 0;
    IBOF_TRACE_WARN(IBOF_EVENT_ID::VOL_NOT_EXIST, "The requested volume does not exist");
    return static_cast<int>(IBOF_EVENT_ID::VOL_NOT_EXIST);
}

int
VolumeManager::Create(string name, uint64_t size, string array, uint64_t maxiops, uint64_t maxbw)
{
    if (initialized == false)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::ARRAY_NOT_MOUNTED, "Array is not mounted");
        return static_cast<int>(IBOF_EVENT_ID::ARRAY_NOT_MOUNTED);
    }

    if (stopped == true)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::SYSTEM_FAULT,
            "Cannot be performed in the STOP state");
        return static_cast<int>(IBOF_EVENT_ID::SYSTEM_FAULT);
    }

    if (ArraySingleton::Instance()->GetArrayName() != array)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::ARRAY_WRONG_NAME,
            "array requested does not exist");
        return static_cast<int>(IBOF_EVENT_ID::ARRAY_WRONG_NAME);
    }

    VolumeNamePolicy namePolicy;
    int ret = namePolicy.CheckVolumeName(name);
    if (ret != static_cast<int>(IBOF_EVENT_ID::SUCCESS))
    {
        return ret;
    }

    if (volumes.GetID(name) >= 0)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::NAME_DUPLICATED, "Volume name is duplicated");
        return static_cast<int>(IBOF_EVENT_ID::NAME_DUPLICATED);
    }

    ret = _CheckVolumeSize(size);
    if (ret != static_cast<int>(IBOF_EVENT_ID::SUCCESS))
    {
        return ret;
    }

    VolumeBase* vol = new Volume(name, size);
    if (vol == nullptr)
    {
        IBOF_TRACE_ERROR(IBOF_EVENT_ID::SIZE_NOT_ALIGNED, "Fail to allocate memory");
        return static_cast<int>(IBOF_EVENT_ID::MEM_ALLOC_FAIL);
    }

    ret = _SetVolumeQoS(vol, maxiops, maxbw);
    if (ret != static_cast<int>(IBOF_EVENT_ID::SUCCESS))
    {
        delete vol;
        return ret; // error_create_vol_failed;
    }

    ret = volumes.Add(vol);
    if (ret != static_cast<int>(IBOF_EVENT_ID::SUCCESS))
    {
        delete vol;
        return ret; // error_create_vol_failed;
    }

    ret = SaveVolumes();
    if (ret != static_cast<int>(IBOF_EVENT_ID::SUCCESS))
    {
        volumes.Remove(vol->ID);
        return ret;
    }

    IBOF_TRACE_DEBUG(IBOF_EVENT_ID::SUCCESS, "Volume meta saved successfully");

    bool res = VolumeEventPublisherSingleton::Instance()->NotifyVolumeCreated(name, vol->ID, size, vol->MaxIOPS(), vol->MaxBW());

    if (res == false)
    {
        return static_cast<int>(IBOF_EVENT_ID::DONE_WITH_ERROR);
    }

    return static_cast<int>(IBOF_EVENT_ID::SUCCESS);
}

int
VolumeManager::Delete(string name, string array)
{
    if (stopped == true)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::SYSTEM_FAULT,
            "Cannot be performed in the STOP state");
        return static_cast<int>(IBOF_EVENT_ID::SYSTEM_FAULT);
    }

    if (ArraySingleton::Instance()->GetArrayName() != array)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::ARRAY_WRONG_NAME,
            "array requested does not exist");
        return static_cast<int>(IBOF_EVENT_ID::ARRAY_WRONG_NAME);
    }

    VolumeBase* vol = volumes.GetVolume(name);
    if (vol == nullptr)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::VOL_NOT_EXIST, "The requested volume does not exist");
        return static_cast<int>(IBOF_EVENT_ID::VOL_NOT_EXIST);
    }

    if (vol->GetStatus() == VolumeStatus::Mounted)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::DEL_MOUNTED_VOL, "Unable to delete mounted volume");
        return static_cast<int>(IBOF_EVENT_ID::DEL_MOUNTED_VOL);
    }

    int volID = vol->ID;
    uint64_t volSize = vol->TotalSize();
    string subnqn = vol->GetSubnqn();
    vol->SetValid(false); // remove tempo.
    vol->SetSubnqn("");

    int ret = SaveVolumes();
    if (ret == static_cast<int>(IBOF_EVENT_ID::SUCCESS))
    {
        ret = volumes.Remove(volID);
        if (ret != static_cast<int>(IBOF_EVENT_ID::SUCCESS))
        {
            return ret;
        }
    }
    else
    {
        vol->SetValid(true); // undo remove
        vol->SetSubnqn(subnqn);
        return ret;
    }

    bool res = VolumeEventPublisherSingleton::Instance()->NotifyVolumeDeleted(
        name, volID, volSize);

    if (res == false)
    {
        return static_cast<int>(IBOF_EVENT_ID::DONE_WITH_ERROR);
    }

    return static_cast<int>(IBOF_EVENT_ID::SUCCESS);
}

int
VolumeManager::Mount(string name, string array, string subnqn)
{
    if (stopped == true)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::SYSTEM_FAULT,
            "Cannot be performed in the STOP state");
        return static_cast<int>(IBOF_EVENT_ID::SYSTEM_FAULT);
    }

    if (ArraySingleton::Instance()->GetArrayName() != array)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::ARRAY_WRONG_NAME,
            "array requested does not exist");
        return static_cast<int>(IBOF_EVENT_ID::ARRAY_WRONG_NAME);
    }

    int ret = static_cast<int>(IBOF_EVENT_ID::VOL_NOT_EXIST);
    VolumeBase* vol = volumes.GetVolume(name);

    if (vol == nullptr)
    {
        IBOF_TRACE_WARN(ret, "The requested volume does not exist");
        return ret;
    }
    NvmfTarget nvmfTarget;
    if ((subnqn.empty() && _SubsystemExists() == false) ||
        (!subnqn.empty() && nvmfTarget.FindSubsystem(subnqn) == nullptr))
    {
        ret = static_cast<int>(IBOF_EVENT_ID::SUBSYSTEM_NOT_CREATED);
        IBOF_TRACE_WARN(ret, "No subsystem:{} was created to attach the volume", subnqn);
        return ret;
    }

    if (subnqn.empty())
    {
        struct spdk_nvmf_subsystem* subsystem = nvmfTarget.AllocateSubsystem();
        subnqn = nvmfTarget.GetVolumeNqn(subsystem);
    }

    vol->LockStatus();
    bool res = true;
    if (VolumeStatus::Mounted != vol->GetStatus())
    {
        res = VolumeEventPublisherSingleton::Instance()->NotifyVolumeMounted(
            name, subnqn, vol->ID, vol->TotalSize(), vol->MaxIOPS(), vol->MaxBW());
        ret = vol->Mount();
    }
    else
    {
        ret = static_cast<int>(IBOF_EVENT_ID::VOL_ALD_MOUNTED);
        IBOF_TRACE_WARN(ret, "The volume already mounted: {}", name);
        vol->UnlockStatus();
        return ret;
    }

    if (ret == static_cast<int>(IBOF_EVENT_ID::SUCCESS) && res == false)
    {
        vol->UnlockStatus();
        return static_cast<int>(IBOF_EVENT_ID::DONE_WITH_ERROR);
    }

    if (nvmfTarget.TryToAttachNamespace(subnqn, vol->ID) == false)
    {
        res = VolumeEventPublisherSingleton::Instance()->NotifyVolumeUnmounted(
                name, vol->ID);
        if (res == false)
        {
            ret = static_cast<int>(IBOF_EVENT_ID::DONE_WITH_ERROR);
            IBOF_TRACE_WARN(ret, "Failed to unmount volume during rollback mount: {}", name);
        }
        ret = vol->Unmount();
        if (ret == static_cast<int>(IBOF_EVENT_ID::SUCCESS))
        {
            ret = static_cast<int>(IBOF_EVENT_ID::VOL_ALD_UNMOUNTED);
            IBOF_TRACE_WARN(ret, "The volume already unmounted during rollback mount: {}", name);
        }
    
        vol->UnlockStatus();
    
        ret = static_cast<int>(IBOF_EVENT_ID::CANNOT_EXTEND_NSID);
    	IBOF_TRACE_WARN(ret, "Can't extend more nsid to subsystem:{}", subnqn);
        return ret;
    }
    vol->SetSubnqn(subnqn);
    vol->UnlockStatus();

    return ret;
}

int
VolumeManager::Unmount(string name, string array)
{
    int ret = static_cast<int>(IBOF_EVENT_ID::VOL_NOT_EXIST);
    VolumeBase* vol = volumes.GetVolume(name);
    if (vol == nullptr)
    {
        IBOF_TRACE_WARN(ret, "The requested volume does not exist");
        return ret;
    }

    if (ArraySingleton::Instance()->GetArrayName() != array)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::ARRAY_WRONG_NAME,
            "array requested does not exist");
        return static_cast<int>(IBOF_EVENT_ID::ARRAY_WRONG_NAME);
    }

    vol->LockStatus();
    bool res = true;
    if (VolumeStatus::Unmounted != vol->GetStatus())
    {
        res = VolumeEventPublisherSingleton::Instance()->NotifyVolumeUnmounted(
            name, vol->ID);
        ret = vol->Unmount();
    }
    else
    {
        ret = static_cast<int>(IBOF_EVENT_ID::VOL_ALD_UNMOUNTED);
        IBOF_TRACE_WARN(ret, "The volume already unmounted: {}", name);
    }
    vol->UnlockStatus();

    if (ret == static_cast<int>(IBOF_EVENT_ID::SUCCESS) && res == false)
    {
        return static_cast<int>(IBOF_EVENT_ID::DONE_WITH_ERROR);
    }
    vol->SetSubnqn("");

    return ret;
}

int
VolumeManager::UpdateQoS(string name, string array, uint64_t maxiops, uint64_t maxbw)
{
    if (stopped == true)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::SYSTEM_FAULT,
            "Cannot be performed in the STOP state");
        return static_cast<int>(IBOF_EVENT_ID::SYSTEM_FAULT);
    }

    if (ArraySingleton::Instance()->GetArrayName() != array)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::ARRAY_WRONG_NAME,
            "array requested does not exist");
        return static_cast<int>(IBOF_EVENT_ID::ARRAY_WRONG_NAME);
    }

    VolumeBase* vol = volumes.GetVolume(name);
    int ret = _SetVolumeQoS(vol, maxiops, maxbw);
    if (ret != static_cast<int>(IBOF_EVENT_ID::SUCCESS))
    {
        return ret;
    }

    bool res = VolumeEventPublisherSingleton::Instance()->NotifyVolumeUpdated(vol->GetName(), vol->ID, vol->MaxIOPS(), vol->MaxBW());

    if (res == false)
    {
        return static_cast<int>(IBOF_EVENT_ID::DONE_WITH_ERROR);
    }

    return static_cast<int>(IBOF_EVENT_ID::SUCCESS);
}

int
VolumeManager::Rename(string oldname, string newname, string array)
{
    if (stopped == true)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::SYSTEM_FAULT,
            "Cannot be performed in the STOP state");
        return static_cast<int>(IBOF_EVENT_ID::SYSTEM_FAULT);
    }

    if (ArraySingleton::Instance()->GetArrayName() != array)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::ARRAY_WRONG_NAME,
            "array requested does not exist");
        return static_cast<int>(IBOF_EVENT_ID::ARRAY_WRONG_NAME);
    }

    VolumeBase* vol = volumes.GetVolume(oldname);
    if (vol == nullptr)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::VOL_NOT_EXIST, "The requested volume does not exist");
        return static_cast<int>(IBOF_EVENT_ID::VOL_NOT_EXIST);
    }

    if (volumes.GetID(newname) >= 0)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::NAME_DUPLICATED, "Volume name is duplicated");
        return static_cast<int>(IBOF_EVENT_ID::NAME_DUPLICATED);
    }

    VolumeNamePolicy namePolicy;
    int ret = namePolicy.CheckVolumeName(newname);
    if (ret != static_cast<int>(IBOF_EVENT_ID::SUCCESS))
    {
        return ret;
    }

    vol->Rename(newname);

    ret = SaveVolumes();
    if (ret != static_cast<int>(IBOF_EVENT_ID::SUCCESS))
    {
        vol->Rename(oldname);
        return ret;
    }

    return static_cast<int>(IBOF_EVENT_ID::SUCCESS);
}

int
VolumeManager::Resize(string name, string array, uint64_t newsize)
{
    if (stopped == true)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::SYSTEM_FAULT,
            "Cannot be performed in the STOP state");
        return static_cast<int>(IBOF_EVENT_ID::SYSTEM_FAULT);
    }

    if (ArraySingleton::Instance()->GetArrayName() != array)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::ARRAY_WRONG_NAME,
            "array requested does not exist");
        return static_cast<int>(IBOF_EVENT_ID::ARRAY_WRONG_NAME);
    }

    VolumeBase* vol = volumes.GetVolume(name);
    if (vol == nullptr)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::VOL_NOT_EXIST, "The requested volume does not exist");
        return static_cast<int>(IBOF_EVENT_ID::VOL_NOT_EXIST);
    }

    int ret = _CheckVolumeSize(newsize);
    if (ret != static_cast<int>(IBOF_EVENT_ID::SUCCESS))
    {
        return ret;
    }

    if (newsize <= vol->UsedSize())
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::SIZE_TOO_SMALL, "The requested volume size is less than already used");
        return static_cast<int>(IBOF_EVENT_ID::SIZE_TOO_SMALL);
    }

    uint64_t oldsize = vol->TotalSize();
    vol->Resize(newsize);

    ret = SaveVolumes();
    if (ret != static_cast<int>(IBOF_EVENT_ID::SUCCESS))
    {
        vol->Resize(oldsize);
        return ret;
    }

    return static_cast<int>(IBOF_EVENT_ID::SUCCESS);
}

int
VolumeManager::_CheckVolumeSize(uint64_t volSize)
{
    if (volSize % SZ_1MB != 0 || volSize == 0)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::SIZE_NOT_ALIGNED,
            "The requested size, {} is not aligned to MB", volSize);
        return static_cast<int>(IBOF_EVENT_ID::SIZE_NOT_ALIGNED);
    }

    if (SpaceInfo::IsEnough(volSize) == false)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::VOL_SIZE_EXCEEDED,
            "The requested volume size is larger than the remaining space");

        return static_cast<int>(IBOF_EVENT_ID::VOL_SIZE_EXCEEDED);
    }

    return static_cast<int>(IBOF_EVENT_ID::SUCCESS);
}

int
VolumeManager::_SetVolumeQoS(VolumeBase* vol, uint64_t maxiops, uint64_t maxbw)
{
    if (vol == nullptr)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::VOL_NOT_EXIST, "The requested volume does not exist");
        return static_cast<int>(IBOF_EVENT_ID::VOL_NOT_EXIST);
    }

    uint64_t orig_MaxIOPS = vol->MaxIOPS();
    uint64_t orig_MaxBW = vol->MaxBW();

    int ret = vol->SetMaxIOPS(maxiops);
    if (ret != static_cast<int>(IBOF_EVENT_ID::SUCCESS))
    {
        return ret;
    }

    ret = vol->SetMaxBW(maxbw);
    if (ret != static_cast<int>(IBOF_EVENT_ID::SUCCESS))
    {
        return ret;
    }

    ret = SaveVolumes();
    if (ret != static_cast<int>(IBOF_EVENT_ID::SUCCESS))
    {
        vol->SetMaxIOPS(orig_MaxIOPS);
        vol->SetMaxBW(orig_MaxBW);
        return ret;
    }

    if (maxiops != orig_MaxIOPS)
    {
        IBOF_TRACE_INFO(IBOF_EVENT_ID::SUCCESS, "Max iops is set on volume {} ({}->{})",
            vol->GetName(), orig_MaxIOPS, maxiops);
    }

    if (maxbw != orig_MaxBW)
    {
        IBOF_TRACE_INFO(IBOF_EVENT_ID::SUCCESS, "Max bandwidth is set on volume {} ({}->{})",
            vol->GetName(), orig_MaxBW, maxbw);
    }

    return static_cast<int>(IBOF_EVENT_ID::SUCCESS);
}

int
VolumeManager::GetVolumeStatus(int volId)
{
    VolumeBase* vol = volumes.GetVolume(volId);

    if (vol == nullptr)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::VOL_NOT_EXIST, "The requested volume does not exist");
        return static_cast<int>(IBOF_EVENT_ID::VOL_NOT_EXIST);
    }

    VolumeStatus status = vol->GetStatus();
    return static_cast<int>(status);
}

int
VolumeManager::_LoadVolumes(string array)
{
    volumes.Clear();

    int ret = VolumeMetaIntf::LoadVolumes(volumes);
    if (ret != static_cast<int>(IBOF_EVENT_ID::SUCCESS))
    {
        return ret;
    }

    if (volumes.Count() > 0)
    {
        int idx = -1;
        bool res = true;

        while (true)
        {
            VolumeBase* vol = volumes.Next(idx);
            if (vol == nullptr)
                break;

            bool ret = VolumeEventPublisherSingleton::Instance()->NotifyVolumeLoaded(vol->GetName(), idx, vol->TotalSize(),
                vol->MaxIOPS(), vol->MaxBW());

            if (ret == false)
            {
                res = false;
            }
        }

        if (res == false)
        {
            return static_cast<int>(IBOF_EVENT_ID::DONE_WITH_ERROR);
        }
    }

    return static_cast<int>(IBOF_EVENT_ID::SUCCESS);
}

int
VolumeManager::IncreasePendingIOCount(int volId, uint32_t ioCountToSubmit)
{
    VolumeBase* vol = volumes.GetVolume(volId);
    if (unlikely(nullptr == vol))
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::VOL_NOT_EXIST,
            "The requested volume does not exist");
        return static_cast<int>(IBOF_EVENT_ID::VOL_NOT_EXIST);
    }
    vol->IncreasePendingIOCount(ioCountToSubmit);
    return static_cast<int>(IBOF_EVENT_ID::SUCCESS);
}

int
VolumeManager::DecreasePendingIOCount(int volId, uint32_t ioCountCompleted)
{
    VolumeBase* vol = volumes.GetVolume(volId);
    if (unlikely(nullptr == vol))
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::VOL_NOT_EXIST,
            "The requested volume does not exist");
        return static_cast<int>(IBOF_EVENT_ID::VOL_NOT_EXIST);
    }
    vol->DecreasePendingIOCount(ioCountCompleted);
    return static_cast<int>(IBOF_EVENT_ID::SUCCESS);
}

int
VolumeManager::SaveVolumes(void)
{
    return VolumeMetaIntf::SaveVolumes(volumes);
}

bool
VolumeManager::CheckVolumeIdle(int volId)
{
    bool volumeIdle = false;
    VolumeBase* vol = volumes.GetVolume(volId);
    if (vol == nullptr)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::VOL_NOT_EXIST,
            "The requested volume does not exist");
    }
    else
    {
        volumeIdle = vol->CheckIdle();
    }

    return volumeIdle;
}

void
VolumeManager::WaitUntilVolumeIdle(int volId)
{
    VolumeBase* vol = volumes.GetVolume(volId);
    if (vol == nullptr)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::VOL_NOT_EXIST,
            "The requested volume does not exist");
    }
    else
    {
        vol->WaitUntilIdle();
    }
}

int
VolumeManager::MountAll(void)
{
    if (stopped == true)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::SYSTEM_FAULT,
            "Cannot be performed in the STOP state");
        return static_cast<int>(IBOF_EVENT_ID::SYSTEM_FAULT);
    }

    int vol_cnt = volumes.Count();
    if (vol_cnt > 0)
    {
        int idx = -1;
        while (true)
        {
            VolumeBase* vol = volumes.Next(idx);
            if (vol == nullptr)
                break;

            if (vol->GetStatus() != VolumeStatus::Mounted)
            {
                int ret = Mount(vol->GetName(), vol->GetArray(), vol->GetSubnqn());
                if (ret != static_cast<int>(IBOF_EVENT_ID::SUCCESS))
                    return ret;
            }
        }
    }
    return static_cast<int>(IBOF_EVENT_ID::SUCCESS);
}

void
VolumeManager::DetachVolumes(void)
{
    int vol_cnt = volumes.Count();
    if (vol_cnt > 0)
    {
        vector<int> mountedVols;
        int idx = -1;
        while (true)
        {
            VolumeBase* vol = volumes.Next(idx);
            if (vol == nullptr)
            {
                break;
            }

            if (vol->GetStatus() != VolumeStatus::Unmounted)
            {
                mountedVols.push_back(vol->ID);
            }
        }
        VolumeEventPublisherSingleton::Instance()->NotifyVolumeDetached(mountedVols);
    }
}

int
VolumeManager::VolumeName(int volId, string& name)
{
    VolumeBase* vol = volumes.GetVolume(volId);

    if (vol != nullptr)
    {
        name = vol->GetName();
        return static_cast<int>(IBOF_EVENT_ID::SUCCESS);
    }

    IBOF_TRACE_WARN(IBOF_EVENT_ID::VOL_NOT_EXIST, "The requested volume does not exist");
    return static_cast<int>(IBOF_EVENT_ID::VOL_NOT_EXIST);
}

bool
VolumeManager::_SubsystemExists(void)
{
    struct spdk_nvmf_tgt* nvmf_tgt;
    struct spdk_nvmf_subsystem* subsystem;
    nvmf_tgt = spdk_nvmf_get_tgt("nvmf_tgt");
    subsystem = spdk_nvmf_subsystem_get_first(nvmf_tgt);
    while (subsystem != NULL)
    {
        if (spdk_nvmf_subsystem_get_type(subsystem) == SPDK_NVMF_SUBTYPE_NVME)
        {
            return true;
        }
        subsystem = spdk_nvmf_subsystem_get_next(subsystem);
    }
    return false;
}

int
VolumeManager::VolumeID(string name)
{
    return volumes.GetID(name);
}

int
VolumeManager::GetVolumeCount(void)
{
    return volumes.Count();
}

VolumeBase*
VolumeManager::GetVolume(int volId)
{
    return volumes.GetVolume(volId);
}

void
VolumeManager::StateChanged(StateContext prev, StateContext next)
{
    if (next.GetState() == State::STOP)
    {
        stopped = true;
        DetachVolumes();
    }
    else if (initialized == true)
    {
        stopped = false;
    }
}
#if defined QOS_ENABLED_FE
int
VolumeManager::UpdateVolumePolicy(std::string volName, qos_vol_policy volPolicy)
{
    uint32_t volId = VolumeID(volName);
    return QosManagerSingleton::Instance()->UpdateVolumePolicy(volId, volPolicy);
}
qos_vol_policy
VolumeManager::GetVolumePolicy(std::string volName)
{
    uint32_t volId = VolumeID(volName);
    return QosManagerSingleton::Instance()->GetVolumePolicy(volId);
}
#endif

} // namespace ibofos

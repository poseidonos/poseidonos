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

#include "src/volume/volume_service.h"
#include "src/volume/volume_manager.h"

#include <string>
#include <vector>

#include "src/include/array_mgmt_policy.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/qos/qos_manager.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/volume/volume_creator.h"
#include "src/volume/volume_base.h"
#include "src/volume/volume_deleter.h"
#include "src/volume/volume_detacher.h"
#include "src/volume/volume_mounter.h"
#include "src/volume/volume_loader.h"
#include "src/volume/volume_unmounter.h"
#include "src/volume/volume_meta_intf.h"
#include "src/volume/volume_renamer.h"
#include "src/volume/volume_qos_updater.h"

namespace pos
{

VolumeManager::VolumeManager(IArrayInfo* i, IStateControl* s)
:arrayInfo(i),
state(s)
{
    state->Subscribe(this, typeid(*this).name());
}

VolumeManager::~VolumeManager(void)
{
    state->Unsubscribe(this);
}

int
VolumeManager::Init(void)
{
    int result = 0;

    initialized = true;
    _LoadVolumes();

    result = VolumeServiceSingleton::Instance()->Register(arrayInfo->GetIndex(), this);

    return result;
}

VolumeList*
VolumeManager::GetVolumeList(void)
{
    return &volumes;
}

std::string
VolumeManager::GetStatusStr(VolumeStatus status)
{
    return VOLUME_STATUS_STR[status];
}

void
VolumeManager::Dispose(void)
{
    initialized = false;
    volumes.Clear();

    VolumeServiceSingleton::Instance()->Unregister(arrayInfo->GetIndex());
}

void
VolumeManager::Shutdown(void)
{
    Dispose();
}

void
VolumeManager::Flush(void)
{
    // no-op for IMountSequence
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
        return EID(SUCCESS);
    }

    volSize = 0;
    POS_TRACE_WARN(EID(VOL_NOT_FOUND), "volId: {}", volId);
    return EID(VOL_NOT_FOUND);
}

int
VolumeManager::Create(std::string name, uint64_t size, uint64_t maxIops, uint64_t maxBw)
{
    if (initialized == false)
    {
        int eid = EID(CREATE_VOL_CAN_ONLY_BE_WHILE_ONLINE);
        POS_TRACE_WARN(eid, "array_name: {}, vol_name: {}", arrayInfo->GetName(), name);
        return eid;
    }

    int ret = _CheckPrerequisite();
    if (ret != EID(SUCCESS))
    {
        return ret;
    }
    VolumeCreator volumeCreator(volumes, arrayInfo->GetName(), arrayInfo->GetIndex());
    // setting default values for miniops and minbw
    uint64_t defaultMinIops = 0;
    uint64_t defaultMinBw = 0;
    return volumeCreator.Do(name, size, maxIops, maxBw, defaultMinIops, defaultMinBw);
}

int
VolumeManager::Delete(std::string name)
{
    int ret = _CheckPrerequisite();
    if (ret != EID(SUCCESS))
    {
        return ret;
    }

    VolumeDeleter volumeDeleter(volumes, arrayInfo->GetName(), arrayInfo->GetIndex());
    return volumeDeleter.Do(name);
}

int
VolumeManager::Mount(std::string name, std::string subnqn)
{
    int ret = _CheckPrerequisite();
    if (ret != EID(SUCCESS))
    {
        return ret;
    }

    VolumeMounter volumeMounter(volumes, arrayInfo->GetName(), arrayInfo->GetIndex());
    return volumeMounter.Do(name, subnqn);
}

int
VolumeManager::Unmount(std::string name)
{
    int ret = _CheckPrerequisite();
    if (ret != EID(SUCCESS))
    {
        return ret;
    }

    VolumeUnmounter volumeUnmounter(volumes, arrayInfo->GetName(), arrayInfo->GetIndex());
    return volumeUnmounter.Do(name);
}

int
VolumeManager::UpdateQoS(std::string name, uint64_t maxIops, uint64_t maxBw, uint64_t minIops, uint64_t minBw)
{
    int ret = _CheckPrerequisite();
    if (ret != EID(SUCCESS))
    {
        return ret;
    }

    VolumeQosUpdater volumeQosUpdater(volumes, arrayInfo->GetName(), arrayInfo->GetIndex());
    return volumeQosUpdater.Do(name, maxIops, maxBw, minIops, minBw);
}

int
VolumeManager::Rename(std::string oldName, std::string newName)
{
    int ret = _CheckPrerequisite();
    if (ret != EID(SUCCESS))
    {
        return ret;
    }

    VolumeRenamer volumeRenamer(volumes, arrayInfo->GetName(), arrayInfo->GetIndex());
    return volumeRenamer.Do(oldName, newName);
}

int
VolumeManager::CheckVolumeValidity(int volId)
{
    VolumeBase* vol = volumes.GetVolume(volId);

    if (vol == nullptr)
    {
        POS_TRACE_INFO(EID(VOL_NOT_FOUND), "volId: {}", volId);
        return EID(VOL_NOT_FOUND);
    }

    return EID(SUCCESS);
}

int
VolumeManager::GetVolumeStatus(int volId)
{
    VolumeBase* vol = volumes.GetVolume(volId);

    if (vol == nullptr)
    {
        POS_TRACE_WARN(EID(VOL_NOT_FOUND), "volId: {}", volId);
        return EID(VOL_NOT_FOUND);
    }

    VolumeStatus status = vol->GetStatus();
    return static_cast<int>(status);
}

int
VolumeManager::_LoadVolumes(void)
{
    VolumeLoader volumeLoader(volumes, arrayInfo->GetName(), arrayInfo->GetIndex());
    return volumeLoader.Do();
}

int
VolumeManager::IncreasePendingIOCountIfNotZero(int volId, VolumeStatus volumeStatus, uint32_t ioCountToSubmit)
{
    VolumeBase* vol = volumes.GetVolume(volId);
    if (unlikely(nullptr == vol))
    {
        POS_TRACE_WARN(EID(VOL_NOT_FOUND), "volId: {}", volId);
        return EID(VOL_NOT_FOUND);
    }
    bool success = volumes.IncreasePendingIOCountIfNotZero(volId, volumeStatus, ioCountToSubmit);
    if (success)
    {
        return EID(SUCCESS);
    }
    POS_TRACE_WARN(EID(VOL_NOT_FOUND), "volId: {}", volId);
    return EID(VOL_NOT_FOUND);
}

int
VolumeManager::DecreasePendingIOCount(int volId, VolumeStatus volumeStatus, uint32_t ioCountCompleted)
{
    VolumeBase* vol = volumes.GetVolume(volId);
    if (unlikely(nullptr == vol))
    {
        POS_TRACE_WARN(EID(VOL_NOT_FOUND), "vol_id: {}", volId);
        return EID(VOL_NOT_FOUND);
    }
    volumes.DecreasePendingIOCount(volId, volumeStatus, ioCountCompleted);
    return EID(SUCCESS);
}

void
VolumeManager::DetachVolumes(void)
{
    VolumeDetacher volumeDetacher(volumes, arrayInfo->GetName(), arrayInfo->GetIndex());
    volumeDetacher.DoAll();
}

int
VolumeManager::VolumeName(int volId, std::string& name)
{
    VolumeBase* vol = volumes.GetVolume(volId);

    if (vol != nullptr)
    {
        name = vol->GetName();
        return EID(SUCCESS);
    }

    POS_TRACE_WARN(EID(VOL_NOT_FOUND), "vol_id: {}", volId);
    return EID(VOL_NOT_FOUND);
}

int
VolumeManager::VolumeID(std::string name)
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
VolumeManager::StateChanged(StateContext* prev, StateContext* next)
{
    if (next->ToStateType() == StateEnum::STOP)
    {
        stopped = true;
    }
    else if (initialized == true)
    {
        stopped = false;
    }
}

int
VolumeManager::_CheckPrerequisite(void)
{
    if (stopped == true)
    {
        POS_TRACE_WARN(EID(VOL_REQ_REJECTED_IN_BROKEN_ARRAY),
            "array_name: {}", GetArrayName());
        return EID(VOL_REQ_REJECTED_IN_BROKEN_ARRAY);
    }

    return EID(SUCCESS);
}

std::string
VolumeManager::GetArrayName(void)
{
    return arrayInfo->GetName();
}

} // namespace pos

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

#include "src/volume/volume_service.h"
#include "src/volume/volume_manager.h"

#include <string>
#include <vector>

#include "src/array_mgmt/array_manager.h"
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
#include "src/volume/volume_resizer.h"
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

    VolumeServiceSingleton::Instance()->Register(arrayInfo->GetName(), this);

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

    VolumeServiceSingleton::Instance()->Unregister(arrayInfo->GetName());
    VolumeEventPublisherSingleton::Instance()->RemoveArrayIdx(arrayInfo->GetName());
}

void
VolumeManager::Shutdown(void)
{
    Dispose();
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
        return (int)POS_EVENT_ID::SUCCESS;
    }

    volSize = 0;
    POS_TRACE_WARN((int)POS_EVENT_ID::VOL_NOT_EXIST,
            "The requested volume does not exist");
    return (int)POS_EVENT_ID::VOL_NOT_EXIST;
}

int
VolumeManager::Create(std::string name, uint64_t size, uint64_t maxIops, uint64_t maxBw)
{
    if (initialized == false)
    {
        POS_TRACE_WARN((int)POS_EVENT_ID::ARRAY_NOT_MOUNTED,
                "Array is not mounted");
        return (int)POS_EVENT_ID::ARRAY_NOT_MOUNTED;
    }

    int ret = _CheckPrerequisite();
    if (ret != (int)POS_EVENT_ID::SUCCESS)
    {
        return ret;
    }

    VolumeCreator volumeCreator(volumes, arrayInfo->GetName());
    return volumeCreator.Do(name, size, maxIops, maxBw);
}

int
VolumeManager::Delete(std::string name)
{
    int ret = _CheckPrerequisite();
    if (ret != (int)POS_EVENT_ID::SUCCESS)
    {
        return ret;
    }

    VolumeDeleter volumeDeleter(volumes, arrayInfo->GetName());
    return volumeDeleter.Do(name);
}

int
VolumeManager::Mount(std::string name, std::string subnqn)
{
    int ret = _CheckPrerequisite();
    if (ret != (int)POS_EVENT_ID::SUCCESS)
    {
        return ret;
    }

    VolumeMounter volumeMounter(volumes, arrayInfo->GetName());
    return volumeMounter.Do(name, subnqn);
}

int
VolumeManager::Unmount(std::string name)
{
    int ret = _CheckPrerequisite();
    if (ret != (int)POS_EVENT_ID::SUCCESS)
    {
        return ret;
    }

    VolumeUnmounter volumeUnmounter(volumes, arrayInfo->GetName());
    return volumeUnmounter.Do(name);
}

int
VolumeManager::UpdateQoS(std::string name, uint64_t maxIops, uint64_t maxBw)
{
    int ret = _CheckPrerequisite();
    if (ret != (int)POS_EVENT_ID::SUCCESS)
    {
        return ret;
    }

    VolumeQosUpdater volumeQosUpdater(volumes, arrayInfo->GetName());
    return volumeQosUpdater.Do(name, maxIops, maxBw);
}

int
VolumeManager::Rename(std::string oldName, std::string newName)
{
    int ret = _CheckPrerequisite();
    if (ret != (int)POS_EVENT_ID::SUCCESS)
    {
        return ret;
    }

    VolumeRenamer volumeRenamer(volumes, arrayInfo->GetName());
    return volumeRenamer.Do(oldName, newName);
}

int
VolumeManager::Resize(std::string name, uint64_t newSize)
{
    int ret = _CheckPrerequisite();
    if (ret != (int)POS_EVENT_ID::SUCCESS)
    {
        return ret;
    }

    VolumeResizer volumeResizer(volumes, arrayInfo->GetName());
    return volumeResizer.Do(name, newSize);
}

int
VolumeManager::GetVolumeStatus(int volId)
{
    VolumeBase* vol = volumes.GetVolume(volId);

    if (vol == nullptr)
    {
        POS_TRACE_WARN((int)POS_EVENT_ID::VOL_NOT_EXIST, "The requested volume does not exist");
        return (int)POS_EVENT_ID::VOL_NOT_EXIST;
    }

    VolumeStatus status = vol->GetStatus();
    return static_cast<int>(status);
}

int
VolumeManager::_LoadVolumes(void)
{
    VolumeLoader volumeLoader(volumes, arrayInfo->GetName());
    return volumeLoader.Do();
}

int
VolumeManager::IncreasePendingIOCountIfNotZero(int volId, VolumeStatus volumeStatus, uint32_t ioCountToSubmit)
{
    VolumeBase* vol = volumes.GetVolume(volId);
    if (unlikely(nullptr == vol))
    {
        POS_TRACE_WARN((int)POS_EVENT_ID::VOL_NOT_EXIST,
            "The requested volume does not exist");
        return (int)POS_EVENT_ID::VOL_NOT_EXIST;
    }
    bool success = volumes.IncreasePendingIOCountIfNotZero(volId, volumeStatus, ioCountToSubmit);
    if (success)
    {
        return static_cast<int>(POS_EVENT_ID::SUCCESS);
    }
    POS_TRACE_WARN(POS_EVENT_ID::VOL_NOT_EXIST,
        "The requested volume is already deleted");
    return static_cast<int>(POS_EVENT_ID::VOL_NOT_EXIST);
}

int
VolumeManager::DecreasePendingIOCount(int volId, VolumeStatus volumeStatus, uint32_t ioCountCompleted)
{
    VolumeBase* vol = volumes.GetVolume(volId);
    if (unlikely(nullptr == vol))
    {
        POS_TRACE_WARN((int)POS_EVENT_ID::VOL_NOT_EXIST,
            "The requested volume does not exist");
        return (int)POS_EVENT_ID::VOL_NOT_EXIST;
    }
    volumes.DecreasePendingIOCount(volId, volumeStatus, ioCountCompleted);
    return static_cast<int>(POS_EVENT_ID::SUCCESS);
}

void
VolumeManager::DetachVolumes(void)
{
    VolumeDetacher volumeDetacher(volumes, arrayInfo->GetName());
    volumeDetacher.DoAll();
}

int
VolumeManager::VolumeName(int volId, std::string& name)
{
    VolumeBase* vol = volumes.GetVolume(volId);

    if (vol != nullptr)
    {
        name = vol->GetName();
        return (int)POS_EVENT_ID::SUCCESS;
    }

    POS_TRACE_WARN((int)POS_EVENT_ID::VOL_NOT_EXIST,
            "The requested volume does not exist");
    return (int)POS_EVENT_ID::VOL_NOT_EXIST;
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
VolumeManager::UpdateVolumePolicy(std::string volName, qos_vol_policy volPolicy)
{
    uint32_t volId = VolumeID(volName);
    QosManager* qosManager = QosManagerSingleton::Instance();
    if (true == qosManager->IsFeQosEnabled())
    {
        return QosManagerSingleton::Instance()->UpdateVolumePolicy(volId, volPolicy);
    }
    else
    {
        return (int)POS_EVENT_ID::QOS_NOT_SUPPORTED;
    }
}

qos_vol_policy
VolumeManager::GetVolumePolicy(std::string volName)
{
    uint32_t volId = VolumeID(volName);
    qos_vol_policy volPolicy;
    QosManager* qosManager = QosManagerSingleton::Instance();
    if (true == qosManager->IsFeQosEnabled())
    {
        volPolicy = QosManagerSingleton::Instance()->GetVolumePolicy(volId);
    }
    return volPolicy;
}

int
VolumeManager::_CheckPrerequisite(void)
{
    if (stopped == true)
    {
        POS_TRACE_WARN((int)POS_EVENT_ID::SYSTEM_FAULT,
            "Cannot be performed in the STOP state");
        return (int)POS_EVENT_ID::SYSTEM_FAULT;
    }

    return (int)POS_EVENT_ID::SUCCESS;
}

} // namespace pos

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
#include "src/sys_info/space_info.h"
#include "src/qos/qos_manager.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"
#include "src/volume/volume_creator.h"
#include "src/volume/volume_base.h"
#include "src/volume/volume_deleter.h"
#include "src/volume/volume_detacher.h"
#include "src/volume/volume_meta_saver.h"
#include "src/volume/volume_mounter.h"
#include "src/volume/volume_loader.h"
#include "src/volume/volume_unmounter.h"
#include "src/volume/volume_meta_intf.h"
#include "src/volume/volume_renamer.h"
#include "src/volume/volume_replicate_property_updater.h"
#include "src/volume/volume_qos_updater.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"

namespace pos
{

VolumeManager::VolumeManager(IArrayInfo* i, IStateControl* s)
:arrayInfo(i),
state(s),
tp(nullptr)
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
    _ClearLock();
    _LoadVolumes();

    tp = new TelemetryPublisher(("Volume_Manager"));
    tp->AddDefaultLabel("array_name", arrayInfo->GetName());

    if (tp == nullptr)
    {
        tp = new TelemetryPublisher(("VolumeManager"));
        tp->AddDefaultLabel("array_name", arrayInfo->GetName());
        TelemetryClientSingleton::Instance()->RegisterPublisher(tp);
    }
    result = VolumeServiceSingleton::Instance()->Register(arrayInfo->GetIndex(), this);

    _PublishTelemetryArrayUsage();
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
    _ClearLock();

    VolumeServiceSingleton::Instance()->Unregister(arrayInfo->GetIndex());
    if (tp != nullptr)
    {
        TelemetryClientSingleton::Instance()->DeregisterPublisher(tp->GetName());
        delete tp;
        tp = nullptr;
    }
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

void
VolumeManager::_PublishTelemetryVolumeState(string name, VolumeStatus status)
{
    if (tp != nullptr)
    {
        POSMetric metric(TEL50020_VOL_VOLUME_STATE, POSMetricTypes::MT_GAUGE);
        metric.SetGaugeValue(status);
        metric.AddLabel("array_id", to_string(arrayInfo->GetIndex()));
        metric.AddLabel("volume_name", name);
        tp->PublishMetric(metric);
    }
}

void
VolumeManager::_PublishTelemetryVolumeCapacity(string name, uint64_t size)
{
    if (tp != nullptr)
    {
        POSMetric metric(TEL50021_VOL_VOLUME_TOTAL_CAPACITY, POSMetricTypes::MT_GAUGE);
        metric.SetGaugeValue(size);
        metric.AddLabel("array_id", to_string(arrayInfo->GetIndex()));
        metric.AddLabel("volume_name", name);
        tp->PublishMetric(metric);
    }
}

void
VolumeManager::_PublishTelemetryArrayUsage(void)
{
    if (tp != nullptr)
    {
        uint64_t arrayUsage = SpaceInfo::Remaining(arrayInfo->GetIndex());
        POSMetric metric(TEL60002_ARRAY_USAGE_BLK_CNT, POSMetricTypes::MT_GAUGE);
        metric.SetGaugeValue(arrayUsage);
        metric.AddLabel("array_id", to_string(arrayInfo->GetIndex()));
        tp->PublishMetric(metric);
    }
}

int
VolumeManager::Create(std::string name, uint64_t size, uint64_t maxIops, uint64_t maxBw, bool checkWalVolume, std::string uuid)
{
    int ret = _CheckPrerequisite();
    if (ret != EID(SUCCESS))
    {
        return ret;
    }

    unique_lock<mutex> eventLock(volumeEventLock, std::defer_lock);
    unique_lock<mutex> exceptionLock(volumeExceptionLock, std::defer_lock);

    ret = std::try_lock(exceptionLock, eventLock);

    if (ret != -1)
    {
        POS_TRACE_WARN(EID(CREATE_VOL_LOCK_FAIL), "failed try lock index : {} fail vol name: {}", ret, name);
        
        return EID(VOL_MGR_BUSY);
    }

    VolumeCreator volumeCreator(volumes, arrayInfo->GetName(), arrayInfo->GetIndex());
    // setting default values for miniops and minbw
    uint64_t defaultMinIops = 0;
    uint64_t defaultMinBw = 0;

    ret = volumeCreator.Do(name, size, maxIops, maxBw, defaultMinIops, defaultMinBw, uuid, checkWalVolume);

    if (EID(SUCCESS) == ret)
    {
        _PublishTelemetryVolumeState(name, VolumeStatus::Unmounted);
        _PublishTelemetryVolumeCapacity(name, size);
        _PublishTelemetryArrayUsage();
    }

    return ret;
}

int
VolumeManager::Delete(std::string name)
{
    int ret = _CheckPrerequisite();
    if (ret != EID(SUCCESS))
    {
        POS_TRACE_WARN(EID(DELETE_VOL_DEBUG_MSG  ), "try alloc fail vol name: {}", name);
        return ret;
    }

    unique_lock<mutex> eventLock(volumeEventLock, std::defer_lock);
    unique_lock<mutex> exceptionLock(volumeExceptionLock, std::defer_lock);

    ret = std::try_lock(exceptionLock, eventLock);

    if (ret != -1)
    {
        POS_TRACE_WARN(EID(DELETE_VOL_LOCK_FAIL), "failed try lock index : {} fail vol name: {}", ret, name);
        
        return EID(VOL_MGR_BUSY);
    }

    VolumeDeleter volumeDeleter(volumes, arrayInfo->GetName(), arrayInfo->GetIndex());
    ret = volumeDeleter.Do(name);
    
    if (EID(SUCCESS) == ret)
    {
        _PublishTelemetryVolumeState(name, VolumeStatus::Offline);
        _PublishTelemetryVolumeCapacity(name, 0);
        _PublishTelemetryArrayUsage();
    }

    return ret;
}

int
VolumeManager::CancelVolumeReplay(int volId)
{
    std::string volname;
    GetVolumeName(volId, volname);
    int ret = Delete(volname);

    return ret;
}

int
VolumeManager::Mount(std::string name, std::string subnqn)
{
    int ret = _CheckPrerequisite();
    if (ret != EID(SUCCESS))
    {
        return ret;
    }

    unique_lock<mutex> eventLock(volumeEventLock, std::defer_lock);
    unique_lock<mutex> exceptionLock(volumeExceptionLock, std::defer_lock);

    ret = std::try_lock(exceptionLock, eventLock);

    if (ret != -1)
    {
        POS_TRACE_WARN(EID(MOUNT_VOL_LOCK_FAIL), "failed try lock index : {} fail vol name: {}", ret, name);
        
        return EID(VOL_MGR_BUSY);
    }

    VolumeMounter volumeMounter(volumes, arrayInfo->GetName(), arrayInfo->GetIndex());
    ret = volumeMounter.Do(name, subnqn);

    if (EID(SUCCESS) == ret)
    {
        _PublishTelemetryVolumeState(name, VolumeStatus::Mounted);
    }

    return ret;
}

int
VolumeManager::Unmount(int volId)
{
    string name;
    int ret = GetVolumeName(volId, name);

    if (ret != EID(SUCCESS))
    {
        return ret;
    }

    return Unmount(name);
}

int
VolumeManager::Unmount(std::string name)
{
    int ret = _CheckPrerequisite();
    if (ret != EID(SUCCESS))
    {
        return ret;
    }

    unique_lock<mutex> eventLock(volumeEventLock, std::defer_lock);
    unique_lock<mutex> exceptionLock(volumeExceptionLock, std::defer_lock);

    ret = std::try_lock(exceptionLock, eventLock);

    if (ret != -1)
    {
        POS_TRACE_WARN(EID(UNMOUNT_VOL_LOCK_FAIL), "failed try lock index : {} fail vol name: {}", ret, name);
        
        return EID(VOL_MGR_BUSY);
    }

    VolumeUnmounter volumeUnmounter(volumes, arrayInfo->GetName(), arrayInfo->GetIndex());
    ret = volumeUnmounter.Do(name);

    if (EID(SUCCESS) == ret)
    {
        _PublishTelemetryVolumeState(name, VolumeStatus::Unmounted);
    }

    return ret;
}

int
VolumeManager::UpdateQoSProperty(std::string name, uint64_t maxIops, uint64_t maxBw, uint64_t minIops, uint64_t minBw)
{
    int ret = _CheckPrerequisite();
    if (ret != EID(SUCCESS))
    {
        return ret;
    }

    unique_lock<mutex> eventLock(volumeEventLock, std::defer_lock);
    unique_lock<mutex> exceptionLock(volumeExceptionLock, std::defer_lock);

    ret = std::try_lock(exceptionLock, eventLock);

    if (ret != -1)
    {
        POS_TRACE_WARN(EID(VOL_UPDATE_LOCK_FAIL), "failed try lock index : {} fail vol name: {}", ret, name);
        
        return EID(VOL_MGR_BUSY);
    }

    VolumeQosUpdater volumeQosUpdater(volumes, arrayInfo->GetName(), arrayInfo->GetIndex());
    ret = volumeQosUpdater.Do(name, maxIops, maxBw, minIops, minBw);

    return ret;
}

int
VolumeManager::UpdateVolumeReplicationState(std::string name, VolumeReplicationState state)
{
    int ret = _CheckPrerequisite();
    if (ret != EID(SUCCESS))
    {
        return ret;
    }

    unique_lock<mutex> eventLock(volumeEventLock, std::defer_lock);
    unique_lock<mutex> exceptionLock(volumeExceptionLock, std::defer_lock);

    ret = std::try_lock(exceptionLock, eventLock);

    if (ret != -1)
    {
        POS_TRACE_WARN(EID(VOL_UPDATE_LOCK_FAIL), "failed try lock index : {} fail vol name: {}", ret, name);
        
        return EID(VOL_MGR_BUSY);
    }

    VolumeReplicatePropertyUpdater volumeReplicatePropertyUpdater(volumes, arrayInfo->GetName(), arrayInfo->GetIndex());

    ret = volumeReplicatePropertyUpdater.Do(name, state);

    return ret;
}

int
VolumeManager::UpdateVolumeReplicationRoleProperty(std::string name, VolumeReplicationRoleProperty nodeProperty)
{
    int ret = _CheckPrerequisite();
    if (ret != EID(SUCCESS))
    {
        return ret;
    }

    unique_lock<mutex> eventLock(volumeEventLock, std::defer_lock);
    unique_lock<mutex> exceptionLock(volumeExceptionLock, std::defer_lock);

    ret = std::try_lock(exceptionLock, eventLock);

    if (ret != -1)
    {
        POS_TRACE_WARN(EID(VOL_UPDATE_LOCK_FAIL), "failed try lock index : {} fail vol name: {}", ret, name);
        
        return EID(VOL_MGR_BUSY);
    }

    VolumeReplicatePropertyUpdater volumeReplicatePropertyUpdater(volumes, arrayInfo->GetName(), arrayInfo->GetIndex());

    ret = volumeReplicatePropertyUpdater.Do(name, nodeProperty);

    return ret;
}

int
VolumeManager::Rename(std::string oldName, std::string newName)
{
    int ret = _CheckPrerequisite();
    if (ret != EID(SUCCESS))
    {
        return ret;
    }

    unique_lock<mutex> eventLock(volumeEventLock, std::defer_lock);
    unique_lock<mutex> exceptionLock(volumeExceptionLock, std::defer_lock);

    ret = std::try_lock(exceptionLock, eventLock);

    if (ret != -1)
    {
        POS_TRACE_WARN(EID(VOL_UPDATE_LOCK_FAIL), "failed try lock index : {} fail vol name: {}", ret, oldName);
        
        return EID(VOL_MGR_BUSY);
    }

    VolumeRenamer volumeRenamer(volumes, arrayInfo->GetName(), arrayInfo->GetIndex());
    ret = volumeRenamer.Do(oldName, newName);

    return ret;
}

int
VolumeManager::SaveVolumeMeta(void)
{
    int ret = _CheckPrerequisite();
    if (ret != EID(SUCCESS))
    {
        return ret;
    }

    unique_lock<mutex> eventLock(volumeEventLock, std::defer_lock);
    unique_lock<mutex> exceptionLock(volumeExceptionLock, std::defer_lock);

    ret = std::try_lock(exceptionLock, eventLock);

    if (ret != -1)
    {
        POS_TRACE_WARN(EID(VOL_UPDATE_LOCK_FAIL), "failed try lock index : {}", ret);
        
        return EID(VOL_MGR_BUSY);
    }

    VolumeMetaSaver volumeMetaSaver(volumes, arrayInfo->GetName(), arrayInfo->GetIndex());
    return volumeMetaSaver.Do();
}

int
VolumeManager::CheckVolumeValidity(std::string Name)
{
    VolumeBase* vol = volumes.GetVolume(Name);

    if (vol == nullptr)
    {
        POS_TRACE_INFO(EID(VOL_NOT_FOUND), "volId: {}", vol->ID);
        return EID(VOL_NOT_FOUND);
    }

    return EID(SUCCESS);
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
VolumeManager::GetVolumeReplicationState(int volId)
{
    VolumeBase* vol = volumes.GetVolume(volId);

    if (vol == nullptr)
    {
        POS_TRACE_WARN(EID(VOL_NOT_FOUND), "volId: {}", volId);
        return EID(VOL_NOT_FOUND);
    }

    VolumeReplicationState status = vol->GetReplicationState();
    return static_cast<int>(status);
}

int
VolumeManager::GetVolumeReplicationRoleProperty(int volId)
{
    VolumeBase* vol = volumes.GetVolume(volId);

    if (vol == nullptr)
    {
        POS_TRACE_WARN(EID(VOL_NOT_FOUND), "volId: {}", volId);
        return EID(VOL_NOT_FOUND);
    }

    VolumeReplicationRoleProperty status = vol->GetReplicateRoleProperty();
    return static_cast<int>(status);
}

int
VolumeManager::_LoadVolumes(void)
{
    VolumeLoader volumeLoader(volumes, arrayInfo->GetName(), arrayInfo->GetIndex());
    return volumeLoader.Do();
}

int
VolumeManager::IncreasePendingIOCountIfNotZero(int volId, VolumeIoType volumeIoType, uint32_t ioCountToSubmit)
{
    VolumeBase* vol = volumes.GetVolume(volId);
    if (unlikely(nullptr == vol))
    {
        POS_TRACE_WARN(EID(VOL_NOT_FOUND), "volId: {}", volId);
        return EID(VOL_NOT_FOUND);
    }
    bool success = volumes.IncreasePendingIOCountIfNotZero(volId, volumeIoType, ioCountToSubmit);
    if (success)
    {
        return EID(SUCCESS);
    }
    POS_TRACE_WARN(EID(VOL_NOT_FOUND), "volId: {}", volId);
    return EID(VOL_NOT_FOUND);
}

int
VolumeManager::DecreasePendingIOCount(int volId, VolumeIoType volumeIoType, uint32_t ioCountCompleted)
{
    VolumeBase* vol = volumes.GetVolume(volId);
    if (unlikely(nullptr == vol))
    {
        POS_TRACE_WARN(EID(VOL_NOT_FOUND), "vol_id: {}", volId);
        return EID(VOL_NOT_FOUND);
    }
    volumes.DecreasePendingIOCount(volId, volumeIoType, ioCountCompleted);
    return EID(SUCCESS);
}

void
VolumeManager::DetachVolumes(void)
{
    while(true)
    {
        if (volumeExceptionLock.try_lock() == true)
        {
            break;
        }
        usleep(1000);
    }

    VolumeDetacher volumeDetacher(volumes, arrayInfo->GetName(), arrayInfo->GetIndex());
    volumeDetacher.DoAll();
}

int
VolumeManager::GetVolumeName(int volId, std::string& name)
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
VolumeManager::GetVolumeID(std::string name)
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
    if (initialized == false)
    {
        int eid = EID(VOL_MGR_NOT_INITIALIZED);
        POS_TRACE_WARN(eid, "volume manager was not initialized");
        return eid;
    }

    if ((stopped == true) || (arrayInfo->GetState() == ArrayStateEnum::BROKEN))
    {
        POS_TRACE_WARN(EID(VOL_REQ_REJECTED_IN_BROKEN_ARRAY),
            "array_name: {}", GetArrayName());
        return EID(VOL_REQ_REJECTED_IN_BROKEN_ARRAY);
    }

    return EID(SUCCESS);
}

void
VolumeManager::_ClearLock(void)
{
    volumeExceptionLock.try_lock();
    volumeExceptionLock.unlock();
}

std::string
VolumeManager::GetArrayName(void)
{
    return arrayInfo->GetName();
}

} // namespace pos

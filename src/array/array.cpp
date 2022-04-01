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

#include "array.h"

#include <string>

#include "src/array/array_name_policy.h"
#include "src/array/interface/i_abr_control.h"
#include "src/array/rebuild/rebuild_handler.h"
#include "src/array/service/array_service_layer.h"
#include "src/device/device_manager.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/include/array_mgmt_policy.h"
#include "src/include/i_array_device.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/master_context/unique_id_generator.h"
#include "src/io_scheduler/io_dispatcher.h"

namespace pos
{
const int Array::LOCK_ACQUIRE_FAILED = -1;

Array::Array(string name, IArrayRebuilder* rbdr, IAbrControl* abr, IStateControl* iState)
: Array(name, rbdr, abr, new ArrayDeviceManager(DeviceManagerSingleton::Instance(), name),
      DeviceManagerSingleton::Instance(), new PartitionManager(), new ArrayState(iState),
      new PartitionServices(), EventSchedulerSingleton::Instance(), ArrayService::Instance())
{
}

Array::Array(string name, IArrayRebuilder* rbdr, IAbrControl* abr,
    ArrayDeviceManager* devMgr, DeviceManager* sysDevMgr, PartitionManager* ptnMgr, ArrayState* arrayState,
    PartitionServices* svc, EventScheduler* eventScheduler, ArrayServiceLayer* arrayService)
: state(arrayState),
  svc(svc),
  ptnMgr(ptnMgr),
  name_(name),
  devMgr_(devMgr) /*initialize with devMgr*/,
  sysDevMgr(sysDevMgr) /*assign with devMgr*/,
  rebuilder(rbdr),
  abrControl(abr),
  eventScheduler(eventScheduler),
  arrayService(arrayService)
{
    pthread_rwlock_init(&stateLock, nullptr);
}

Array::~Array(void)
{
    delete svc;
    delete ptnMgr;
    delete state;
    delete devMgr_;
}

int
Array::Load(void)
{
    POS_TRACE_INFO(EID(LOAD_ARRAY_DEBUG_MSG), "Trying to load Array({})", name_);
    pthread_rwlock_wrlock(&stateLock);
    int ret = _LoadImpl();
    pthread_rwlock_unlock(&stateLock);
    if (ret != 0)
    {
        if (ret == EID(LOAD_ARRAY_NVM_DOES_NOT_EXIST))
        {
            POS_TRACE_WARN(ret, "arrayname: {}", name_);
        }
        else
        {
            POS_TRACE_WARN(ret, "arrayname: {}", name_);
        }
    }
    else
    {
        POS_TRACE_INFO(EID(LOAD_ARRAY_DEBUG_MSG), "Array({}) loaded successfully", name_);
    }
    return ret;
}

int
Array::_LoadImpl(void)
{
    devMgr_->Clear();
    ArrayMeta meta;
    meta.arrayName = name_;
    int ret = abrControl->LoadAbr(meta);
    if (ret != 0)
    {
        return ret;
    }
    else
    {
        index_ = meta.id;
        uniqueId = meta.unique_id;
    }

    ret = devMgr_->Import(meta.devs);
    if (ret != 0)
    {
        state->SetDelete();
        return ret;
    }

    POS_TRACE_INFO(EID(LOAD_ARRAY_DEBUG_MSG), "Array({}) is loaded from ABR, metaRaid:{}, dataRaid:{}", name_, meta.metaRaidType, meta.dataRaidType);
    ret = _CreatePartitions(RaidType(meta.metaRaidType), RaidType(meta.dataRaidType));
    if (ret != 0)
    {
        return ret;
    }

    RaidState rs = ptnMgr->GetRaidState();
    state->EnableStatePublisher(uniqueId);
    state->SetLoad(rs);
    return ret;
}

int
Array::Create(DeviceSet<string> nameSet, string metaFt, string dataFt)
{
    POS_TRACE_INFO(EID(CREATE_ARRAY_DEBUG_MSG), "Trying to create array({})", name_);
    int ret = 0;
    bool needSpare = true;
    ArrayMeta meta;
    ArrayNamePolicy namePolicy;
    UniqueIdGenerator uIdGen;

    pthread_rwlock_wrlock(&stateLock);
    needSpare = RaidType(dataFt) != RaidTypeEnum::NONE && RaidType(dataFt) != RaidTypeEnum::RAID0;
    if (needSpare == false && nameSet.spares.size() > 0)
    {
        ret = EID(CREATE_ARRAY_RAID_DOES_NOT_SUPPORT_SPARE_DEV);
        POS_TRACE_INFO(ret, "RaidType: {}", dataFt);
        goto error;
    }

    ret = devMgr_->ImportByName(nameSet);
    if (ret != 0)
    {
        goto error;
    }

    ret = namePolicy.CheckArrayName(name_);
    if (ret != EID(SUCCESS))
    {
        goto error;
    }

    uniqueId = uIdGen.GenerateUniqueId();
    state->EnableStatePublisher(uniqueId);

    meta.arrayName = name_;
    meta.devs = devMgr_->ExportToMeta();
    meta.metaRaidType = metaFt;
    meta.dataRaidType = dataFt;
    meta.unique_id = uniqueId;
    ret = abrControl->CreateAbr(meta);
    if (ret != 0)
    {
        goto error;
    }
    else
    {
        index_ = meta.id;
    }

    ret = _Flush(meta);
    if (ret != 0)
    {
        goto error;
    }

    ret = _CreatePartitions(RaidType(meta.metaRaidType), RaidType(meta.dataRaidType));
    if (ret != 0)
    {
        goto error;
    }

    ptnMgr->FormatPartition(PartitionType::META_SSD, index_, IODispatcherSingleton::Instance());

    state->SetCreate();
    pthread_rwlock_unlock(&stateLock);
    POS_TRACE_INFO(EID(CREATE_ARRAY_DEBUG_MSG), "Array({}) is created successfully", name_);
    return 0;

error:
    devMgr_->Clear();
    pthread_rwlock_unlock(&stateLock);
    POS_TRACE_ERROR(ret, "Unable to create array({})", name_);
    return ret;
}

int
Array::Init(void)
{
    // TODO(hsung.yang) MOUNTSEQUENCE: rollback sequence for array mount
    // pthread_rwlock_wrlock(&stateLock);
    POS_TRACE_INFO(EID(MOUNT_ARRAY_DEBUG_MSG), "Trying to mount array({})", name_);

    int ret = state->IsMountable();
    if (ret != 0)
    {
        goto error;
    }

    ret = _RegisterService();
    if (ret != 0)
    {
        goto error;
    }

    state->SetMount();
    // pthread_rwlock_unlock(&stateLock);
    POS_TRACE_INFO(EID(MOUNT_ARRAY_DEBUG_MSG), "Array({}) is mounted successfully (id:{})", name_, index_);
    return ret;

error:
    _UnregisterService();
    // pthread_rwlock_unlock(&stateLock);
    POS_TRACE_ERROR(ret, "Unable to mount array({})", name_);
    return ret;
}

void
Array::Dispose(void)
{
    // pthread_rwlock_wrlock(&stateLock);
    POS_TRACE_INFO(EID(UNMOUNT_ARRAY_DEBUG_MSG), "Trying to unmount array({})", name_);
    _UnregisterService();
    state->SetUnmount();
    isWTEnabled = false;
    POS_TRACE_INFO(EID(UNMOUNT_ARRAY_DEBUG_MSG), "Array({}) is unmounted successfully", name_);
    // pthread_rwlock_unlock(&stateLock);
}

void
Array::Shutdown(void)
{
    POS_TRACE_INFO(EID(UNMOUNT_BROKEN_ARRAY_DEBUG_MSG), "Trying to shut down broken array({})", name_);
    _UnregisterService();
    state->SetShutdown();
    isWTEnabled = false;
    POS_TRACE_INFO(EID(UNMOUNT_BROKEN_ARRAY_DEBUG_MSG), "Broken array({}) shuts down successfully", name_);
}

void
Array::Flush(void)
{
    POS_TRACE_INFO(EID(UPDATE_ABR_DEBUG_MSG), "Flush: trying to update array({}) configuration", name_);
    int ret = _Flush();
    if (0 != ret)
    {
        POS_TRACE_ERROR(ret, "Flush: unable to update array({}) configuration", name_);
        return;
    }

    POS_TRACE_INFO(EID(UPDATE_ABR_DEBUG_MSG), "Flush: array({}) configuration is updated successfully", name_);
}

int
Array::Delete(void)
{
    POS_TRACE_INFO(EID(DELETE_ARRAY_DEBUG_MSG), "Trying to delete array({})", name_);
    pthread_rwlock_wrlock(&stateLock);
    int ret = state->IsDeletable();
    if (ret != 0)
    {
        goto error;
    }

    // Rebuild would not be finished when rebuild io have an error on broken array
    if (rebuilder->IsRebuilding(name_))
    {
        ret = EID(DELETE_ARRAY_DEBUG_MSG);
        goto error;
    }

    ret = state->WaitShutdownDone();
    if (ret != 0)
    {
        goto error;
    }

    _DeletePartitions();
    devMgr_->Clear();
    abrControl->DeleteAbr(name_);
    state->SetDelete();

    pthread_rwlock_unlock(&stateLock);
    POS_TRACE_INFO(EID(DELETE_ARRAY_DEBUG_MSG), "Array({}) is deleted successfully", name_);
    return 0;

error:
    pthread_rwlock_unlock(&stateLock);
    POS_TRACE_ERROR(ret, "Unable to delete array({})", name_);
    return ret;
}

int
Array::AddSpare(string devName)
{
    pthread_rwlock_rdlock(&stateLock);
    string raidType = GetDataRaidType();
    int ret = 0;
    bool needSpare = RaidType(raidType) != RaidTypeEnum::NONE && RaidType(raidType) != RaidTypeEnum::RAID0;
    if (needSpare == false)
    {
        ret = EID(ADD_SPARE_RAID_DOES_NOT_SUPPORT_SPARE_DEV);
        POS_TRACE_WARN(ret, "arrayName:{}, RaidType:{}", name_, raidType);
        pthread_rwlock_unlock(&stateLock);
        return ret;
    }

    ret = state->CanAddSpare();
    if (ret != 0)
    {
        pthread_rwlock_unlock(&stateLock);
        return ret;
    }

    DevName spareDevName(devName);
    UblockSharedPtr dev = sysDevMgr->GetDev(spareDevName);
    if (dev == nullptr)
    {
        pthread_rwlock_unlock(&stateLock);
        int eid = EID(ADD_SPARE_SSD_NAME_NOT_FOUND);
        POS_TRACE_WARN(eid, "devName: {}", devName);
        return eid;
    }

    string spareSN = dev->GetSN();
    string involvedArray = abrControl->FindArrayWithDeviceSN(spareSN);
    if (involvedArray != "")
    {
        pthread_rwlock_unlock(&stateLock);
        ret = EID(MBR_DEVICE_ALREADY_IN_ARRAY);
        POS_TRACE_ERROR(ret, "Unable to add spare device to array({}), it's already in other array({})", name_, involvedArray);
        return ret;
    }

    ret = devMgr_->AddSpare(devName);
    if (0 != ret)
    {
        pthread_rwlock_unlock(&stateLock);
        POS_TRACE_ERROR(ret, "Unable to add spare device to array({})", name_);
        return ret;
    }
    ret = _Flush();
    if (0 != ret)
    {
        pthread_rwlock_unlock(&stateLock);
        POS_TRACE_ERROR(ret, "Unable to add spare device to array({})", name_);
        return ret;
    }

    if (state->IsRebuildable())
    {
        EventSmartPtr event(new RebuildHandler(this, nullptr));
        eventScheduler->EnqueueEvent(event);
    }
    pthread_rwlock_unlock(&stateLock);
    POS_TRACE_INFO(EID(ADD_SPARE_DEBUG_MSG), "Spare device was successfully added to array({})", name_);

    return 0;
}

int
Array::RemoveSpare(string devName)
{
    pthread_rwlock_rdlock(&stateLock);

    int ret = state->CanRemoveSpare();
    if (ret != 0)
    {
        goto error;
    }
    ret = devMgr_->RemoveSpare(devName);
    if (0 != ret)
    {
        goto error;
    }
    ret = _Flush();
    if (0 != ret)
    {
        goto error;
    }

    pthread_rwlock_unlock(&stateLock);

    POS_TRACE_INFO(EID(REMOVE_SPARE_DEBUG_MSG),
        "The spare device {} removed from array({})", devName, name_);
    return 0;

error:
    pthread_rwlock_unlock(&stateLock);
    POS_TRACE_ERROR(ret, "Unable to remove spare device {} from array({})", devName, name_);
    return ret;
}

const PartitionLogicalSize*
Array::GetSizeInfo(PartitionType type)
{
    const PartitionLogicalSize* sizeInfo = nullptr;
    sizeInfo = ptnMgr->GetSizeInfo(type);
    return sizeInfo;
}

DeviceSet<string>
Array::GetDevNames(void)
{
    return devMgr_->ExportToName();
}

string
Array::GetName(void)
{
    return name_;
}

unsigned int
Array::GetIndex(void)
{
    return index_;
}

string
Array::GetMetaRaidType(void)
{
    return RaidType(ptnMgr->GetRaidType(PartitionType::META_SSD)).ToString();
}

string
Array::GetDataRaidType(void)
{
    return RaidType(ptnMgr->GetRaidType(PartitionType::USER_DATA)).ToString();
}

string
Array::GetCreateDatetime(void)
{
    return abrControl->GetCreatedDateTime(name_);
}

string
Array::GetUpdateDatetime(void)
{
    return abrControl->GetLastUpdatedDateTime(name_);
}

id_t
Array::GetUniqueId(void)
{
    return uniqueId;
}

ArrayStateType
Array::GetState(void)
{
    return state->GetState();
}

StateContext*
Array::GetStateCtx(void)
{
    return state->GetSysState();
}

uint32_t
Array::GetRebuildingProgress(void)
{
    return rebuilder->GetRebuildProgress(name_);
}

IArrayDevMgr*
Array::GetArrayManager(void)
{
    return devMgr_;
}

bool
Array::IsWriteThroughEnabled(void)
{
    return isWTEnabled;
}

int
Array::_Flush(void)
{
    ArrayMeta meta;
    meta.arrayName = GetName();
    meta.metaRaidType = GetMetaRaidType();
    meta.dataRaidType = GetDataRaidType();
    meta.devs = devMgr_->ExportToMeta();
    return _Flush(meta);
}

int
Array::_Flush(ArrayMeta& meta)
{
    POS_TRACE_INFO(EID(UPDATE_ABR_DEBUG_MSG), "Trying to save Array to MBR, name:{}, metaRaid:{}, dataRaid:{}",
        meta.arrayName, meta.metaRaidType, meta.dataRaidType);
    return abrControl->SaveAbr(meta);
}

int
Array::_CreatePartitions(RaidTypeEnum metaRaid, RaidTypeEnum dataRaid)
{
    DeviceSet<ArrayDevice*> devs = devMgr_->Export();
    ArrayDevice* nvm = nullptr;
    if (devs.nvm.size() > 0)
    {
        nvm = devs.nvm.front();
    }
    return ptnMgr->CreatePartitions(nvm, devs.data, metaRaid, dataRaid, svc);
}

void
Array::_DeletePartitions(void)
{
    ptnMgr->DeletePartitions();
}

bool
Array::IsRecoverable(IArrayDevice* target, UBlockDevice* uBlock)
{
    pthread_rwlock_wrlock(&stateLock);
    if (state->IsBroken() || !state->IsMounted())
    {
        pthread_rwlock_unlock(&stateLock);
        return false;
    }

    if (uBlock == nullptr                       // Translate / Covnert fail
        || target->GetUblock().get() != uBlock) // Detached device after address translation
    {
        pthread_rwlock_unlock(&stateLock);
        return true;
    }

    _DetachData(static_cast<ArrayDevice*>(target));
    bool ret = state->IsRecoverable();
    pthread_rwlock_unlock(&stateLock);

    return ret;
}

IArrayDevice*
Array::FindDevice(string devSn)
{
    ArrayDeviceType devType = ArrayDeviceType::NONE;
    ArrayDevice* dev = nullptr;
    tie(dev, devType) = devMgr_->GetDev(devSn);
    return dev;
}

int
Array::DetachDevice(UblockSharedPtr uBlock)
{
    ArrayDeviceType devType = ArrayDeviceType::NONE;
    ArrayDevice* dev = nullptr;
    string devName = uBlock->GetName();
    tie(dev, devType) = devMgr_->GetDev(uBlock);

    int eventId = 0;

    switch (devType)
    {
        case ArrayDeviceType::SPARE:
        {
            POS_TRACE_INFO(EID(ARRAY_EVENT_SPARE_SSD_DETACHED),
                    "Spare-device {} is detached from array({})", devName, name_);
            if (pthread_rwlock_trywrlock(&stateLock) == 0)
            {
                _DetachSpare(dev);
                pthread_rwlock_unlock(&stateLock);
            }
            else
            {
                return LOCK_ACQUIRE_FAILED;
            }

            break;
        }
        case ArrayDeviceType::DATA:
        {
            POS_TRACE_INFO(EID(ARRAY_EVENT_DATA_SSD_DETACHED),
                    "Data-device {} is detached from array({})", devName, name_);
            if (pthread_rwlock_trywrlock(&stateLock) == 0)
            {
                if (dev->GetState() != ArrayDeviceState::FAULT)
                {
                    _DetachData(dev);
                }
                pthread_rwlock_unlock(&stateLock);
            }
            else
            {
                return LOCK_ACQUIRE_FAILED;
            }
            break;
        }
        default:
        {
            eventId = EID(ARRAY_EVENT_NVM_DETACHED);
            POS_TRACE_ERROR(eventId,
                "Not allowed device {} is detached from array({})", devName, name_);
            break;
        }
    }

    return eventId;
}

void
Array::SetPreferences(bool isWT)
{
    isWTEnabled = isWT;
}

void
Array::MountDone(void)
{
    _CheckRebuildNecessity();
    int ret = _Flush();
    assert(ret == 0);
}

int
Array::CheckUnmountable(void)
{
    return state->IsUnmountable();
}

void
Array::_CheckRebuildNecessity(void)
{
    ArrayDevice* rebuildDevice = devMgr_->GetRebuilding();
    if (rebuildDevice != nullptr)
    {
        string devName = "no device";
        if (rebuildDevice->GetUblock() != nullptr)
        {
            devName = rebuildDevice->GetUblock()->GetSN();
            POS_TRACE_DEBUG(EID(REBUILD_ARRAY_DEBUG_MSG), "Resume Rebuild with rebuildDevice {}", devName);
            EventSmartPtr event(new RebuildHandler(this, rebuildDevice));
            eventScheduler->EnqueueEvent(event);
        }
        else
        {
            POS_TRACE_ERROR(EID(REBUILD_ARRAY_DEBUG_MSG), "Resume Rebuild without rebuildDevice");
        }
    }
    else
    {
        EventSmartPtr event(new RebuildHandler(this, nullptr));
        eventScheduler->EnqueueEvent(event);
    }
}

void
Array::_DetachSpare(ArrayDevice* target)
{
    UblockSharedPtr uBlock = target->GetUblock();
    if (uBlock == nullptr)
    {
        return;
    }

    int ret = devMgr_->RemoveSpare(uBlock->GetName());
    if (0 != ret)
    {
        return;
    }

    sysDevMgr->RemoveDevice(uBlock);
    delete (target);
    if (state->IsMounted())
    {
        ret = _Flush();
        if (0 != ret)
        {
            return;
        }
    }
}

void
Array::_DetachData(ArrayDevice* target)
{
    POS_TRACE_INFO(EID(ARRAY_EVENT_DATA_SSD_DETACHED),
        "Data device {} is detached from array({})", target->GetUblock()->GetName(), name_);

    ArrayDeviceState devState = target->GetState();
    if (devState == ArrayDeviceState::FAULT)
    {
        return;
    }

    bool needStopRebuild = target->GetState() == ArrayDeviceState::REBUILD;
    target->SetState(ArrayDeviceState::FAULT);
    RaidState rs = ptnMgr->GetRaidState();
    state->RaidStateUpdated(rs);
    sysDevMgr->RemoveDevice(target->GetUblock());
    target->SetUblock(nullptr);

    if (state->IsMounted())
    {
        int ret = _Flush();
        if (0 != ret)
        {
            return;
        }
    }

    if (needStopRebuild || state->IsBroken())
    {
        POS_TRACE_INFO(EID(ARRAY_EVENT_DATA_SSD_DETACHED),
            "Stop the rebuild due to detachment of the rebuild target device or Array broken");
        rebuilder->StopRebuild(name_);
    }
    else if (state->IsRebuildable())
    {
        EventSmartPtr event(new RebuildHandler(this, target));
        eventScheduler->EnqueueEvent(event);
    }
}

void
Array::_RebuildDone(RebuildResult result)
{
    POS_TRACE_DEBUG(EID(REBUILD_ARRAY_DEBUG_MSG),
        "Array({}) rebuild done. result:{}", name_, result.result);
    rebuilder->RebuildDone(result);
    pthread_rwlock_wrlock(&stateLock);
    if (result.result != RebuildState::PASS)
    {
        state->SetRebuildDone(false);
        pthread_rwlock_unlock(&stateLock);
        POS_TRACE_DEBUG(EID(REBUILD_ARRAY_DEBUG_MSG),
            "Array({}) rebuild done. but result:{} . Retry ", name_, result.result);
        EventSmartPtr event(new RebuildHandler(this, nullptr));
        eventScheduler->EnqueueEvent(event);
        return;
    }

    POS_TRACE_DEBUG(EID(REBUILD_ARRAY_DEBUG_MSG),
        "Array({}) rebuild done. as success.", name_, result.result);

    bool needRebuildAgain = false;
    if (result.target->GetState() != ArrayDeviceState::FAULT)
    {
        result.target->SetState(ArrayDeviceState::NORMAL);
    }
    else
    {
        needRebuildAgain = true;
        POS_TRACE_WARN(EID(REBUILD_ARRAY_DEBUG_MSG),
            "The rebuild {} is done, but it needs to rebuild again since the target device was detached during a rebuilding.",
            name_);
    }

    state->SetRebuildDone(true);
    int ret = _Flush();
    if (0 != ret)
    {
        POS_TRACE_ERROR(ret, "Unable to update the device state of array({})", name_);
    }
    if (needRebuildAgain)
    {
        EventSmartPtr event(new RebuildHandler(this, nullptr));
        eventScheduler->EnqueueEvent(event);
    }
    pthread_rwlock_unlock(&stateLock);
}

bool
Array::TriggerRebuild(ArrayDevice* target)
{
    POS_TRACE_DEBUG(EID(REBUILD_ARRAY_DEBUG_MSG), "Trigger Rebuild Start");
    bool retry = false;

    pthread_rwlock_wrlock(&stateLock);
    if (target == nullptr)
    {
        target = devMgr_->GetFaulty();
        if (target == nullptr)
        {
            pthread_rwlock_unlock(&stateLock);
            return retry;
        }
    }
    if (target->GetState() != ArrayDeviceState::FAULT || target->GetUblock() != nullptr)
    {
        POS_TRACE_DEBUG(POS_EVENT_ID::REBUILD_DEBUG_MSG,
            "Rebuild target device is not removed yet");
        pthread_rwlock_unlock(&stateLock);
        retry = true;
        return retry;
    }

    if (state->SetRebuild() == false)
    {
        POS_TRACE_WARN(POS_EVENT_ID::REBUILD_TRIGGER_FAIL,
            "Failed to trigger rebuild. Current array state is not rebuildable");
        pthread_rwlock_unlock(&stateLock);
        return retry;
    }
    // Degraded
    // System State Invoke Rebuilding
    int ret = devMgr_->ReplaceWithSpare(target);
    if (ret != 0)
    {
        state->SetRebuildDone(false);
        state->SetDegraded();
        POS_TRACE_WARN(POS_EVENT_ID::REBUILD_TRIGGER_FAIL,
            "Failed to trigger rebuild. spare device is not available");
        pthread_rwlock_unlock(&stateLock);
        return retry;
    }

    target->SetState(ArrayDeviceState::REBUILD);
    ret = _Flush();
    if (0 != ret)
    {
        POS_TRACE_WARN(POS_EVENT_ID::REBUILD_TRIGGER_FAIL,
            "Failed to trigger rebuild. Flush failed.");
        pthread_rwlock_unlock(&stateLock);
        return retry;
    }
    pthread_rwlock_unlock(&stateLock);

    POS_TRACE_DEBUG(EID(REBUILD_ARRAY_DEBUG_MSG), "Preparing Rebuild");
    IArrayRebuilder* arrRebuilder = rebuilder;
    string arrName = name_;
    uint32_t arrId = index_;
    RebuildComplete cb = std::bind(&Array::_RebuildDone, this, placeholders::_1);
    list<RebuildTarget*> tasks = svc->GetRebuildTargets();
    thread t([arrRebuilder, arrName, arrId, target, cb, tasks]()
    {
        list<RebuildTarget*> targets = tasks;
        arrRebuilder->Rebuild(arrName, arrId, target, cb, targets);
    });

    t.detach();
    POS_TRACE_DEBUG(EID(REBUILD_ARRAY_DEBUG_MSG), "Trigger Rebuild End");
    return retry;
}

bool
Array::ResumeRebuild(ArrayDevice* target)
{
    POS_TRACE_DEBUG(EID(REBUILD_ARRAY_DEBUG_MSG), "Resume Rebuild Start");

    pthread_rwlock_wrlock(&stateLock);
    assert(target != nullptr);

    if (state->SetRebuild() == false)
    {
        POS_TRACE_WARN(POS_EVENT_ID::REBUILD_TRIGGER_FAIL,
            "Failed to resume rebuild. Array({})'s state is not rebuildable", name_);
        pthread_rwlock_unlock(&stateLock);
        return false;
    }

    pthread_rwlock_unlock(&stateLock);

    POS_TRACE_DEBUG(POS_EVENT_ID::REBUILD_ARRAY_DEBUG_MSG, "Preparing Rebuild");
    IArrayRebuilder* arrRebuilder = rebuilder;
    string arrName = name_;
    uint32_t arrId = index_;
    RebuildComplete cb = std::bind(&Array::_RebuildDone, this, placeholders::_1);
    list<RebuildTarget*> tasks = svc->GetRebuildTargets();
    thread t([arrRebuilder, arrName, arrId, target, cb, tasks]()
    {
        list<RebuildTarget*> targets = tasks;
        arrRebuilder->Rebuild(arrName, arrId, target, cb, targets);
    });

    t.detach();
    POS_TRACE_DEBUG(EID(REBUILD_ARRAY_DEBUG_MSG), "Resume Rebuild End");
    return true;
}

int
Array::_RegisterService(void)
{
    int ret = arrayService->Setter()->Register(name_, index_,
        svc->GetTranslator(), svc->GetRecover(), this);
    if (ret == 0)
    {
        RaidTypeEnum metaRaid = ptnMgr->GetRaidType(PartitionType::META_SSD);
        if (metaRaid == RaidTypeEnum::RAID10)
        {
            if (devMgr_ != nullptr)
            {
                IOLockerSingleton::Instance()->Register(devMgr_->GetDataDevices());
            }
            else
            {
                ret = EID(MOUNT_ARRAY_UNABLE_TO_REGISTER_IOLOCKER);
            }
        }
    }
    if (ret != 0)
    {
        POS_TRACE_WARN(ret, "array_name: {}", name_);
    }
    return ret;
}

void
Array::_UnregisterService(void)
{
    arrayService->Setter()->Unregister(name_, index_);
    if (devMgr_ != nullptr)
    {
        IOLockerSingleton::Instance()->Unregister(devMgr_->GetDataDevices());
    }
}
} // namespace pos

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

#include "array.h"

#include <string>

#include "src/array/array_name_policy.h"
#include "src/array/interface/i_abr_control.h"
#include "src/array/rebuild/rebuild_handler.h"
#include "src/array/service/array_service_layer.h"
#include "src/device/device_manager.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/include/i_array_device.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
const int Array::LOCK_ACQUIRE_FAILED = -1;

Array::Array(string name, IArrayRebuilder* rbdr, IAbrControl* abr, IStateControl* iState)
: Array(name, rbdr, abr, new ArrayDeviceManager(DeviceManagerSingleton::Instance()), DeviceManagerSingleton::Instance(),
      new PartitionManager(name, abr), new ArrayState(iState), new ArrayInterface(), EventSchedulerSingleton::Instance())
{
}

Array::Array(string name, IArrayRebuilder* rbdr, IAbrControl* abr,
    ArrayDeviceManager* devMgr, DeviceManager* sysDevMgr, PartitionManager* ptnMgr, ArrayState* arrayState,
    ArrayInterface* arrayInterface, EventScheduler* eventScheduler)
: state(arrayState),
  intf(arrayInterface),
  ptnMgr(ptnMgr),
  name_(name),
  devMgr_(devMgr) /*initialize with devMgr*/,
  sysDevMgr(sysDevMgr) /*assign with devMgr*/,
  rebuilder(rbdr),
  abrControl(abr),
  eventScheduler(eventScheduler)
{
    pthread_rwlock_init(&stateLock, nullptr);
}

Array::~Array(void)
{
    delete intf;
    delete ptnMgr;
    delete state;
    delete devMgr_;
}

int
Array::Load()
{
    pthread_rwlock_wrlock(&stateLock);
    int ret = _LoadImpl();
    pthread_rwlock_unlock(&stateLock);
    if (ret != 0)
    {
        if (ret == (int)POS_EVENT_ID::ARRAY_DEVICE_NVM_NOT_FOUND)
        {
            POS_TRACE_ERROR(ret, "Failed to load array {}, check uram creation or pmem state", name_);
        }
        else
        {
            POS_TRACE_ERROR(ret, "Failed to load array {}", name_);
        }
    }
    else
    {
        POS_TRACE_INFO((int)POS_EVENT_ID::ARRAY_DEBUG_MSG, "Array {} loaded successfully", name_);
    }
    return ret;
}

int
Array::_LoadImpl()
{
    int ret = state->IsLoadable();
    if (ret != 0)
    {
        return ret;
    }

    devMgr_->Clear();
    _ResetMeta();
    ret = abrControl->LoadAbr(name_, meta_);
    if (ret != 0)
    {
        return ret;
    }

    uint32_t missingCnt = 0;
    uint32_t brokenCnt = 0;
    ret = devMgr_->Import(meta_.devs, missingCnt, brokenCnt);
    if (ret != 0)
    {
        state->SetDelete();
        return ret;
    }
    state->SetLoad(missingCnt, brokenCnt);
    return ret;
}
#ifdef _ADMIN_ENABLED
IArrayDevMgr*
Array::GetArrayManager(void)
{
    return devMgr_;
}
#endif
int
Array::Create(DeviceSet<string> nameSet, string dataRaidType)
{
    int ret = 0;
    pthread_rwlock_wrlock(&stateLock);
    ArrayNamePolicy namePolicy;
    ret = state->IsCreatable();
    if (ret != 0)
    {
        goto error;
    }

    ret = devMgr_->Import(nameSet);
    if (ret != 0)
    {
        goto error;
    }

    ret = namePolicy.CheckArrayName(name_);
    if (ret != (int)POS_EVENT_ID::SUCCESS)
    {
        goto error;
    }

    meta_.arrayName = name_;
    meta_.devs = devMgr_->ExportToMeta();

    if (dataRaidType != "RAID5")
    {
        ret = (int)POS_EVENT_ID::ARRAY_WRONG_FT_METHOD;
        goto error;
    }

    SetMetaRaidType("RAID1");
    SetDataRaidType(dataRaidType);

    ret = abrControl->CreateAbr(name_, meta_);
    if (ret != 0)
    {
        goto error;
    }

    ret = _Flush();
    if (ret != 0)
    {
        goto error;
    }
    _FormatMetaPartition();

    state->SetCreate();
    pthread_rwlock_unlock(&stateLock);
    POS_TRACE_INFO((int)POS_EVENT_ID::ARRAY_DEBUG_MSG, "Array {} was successfully created", name_);
    return 0;

error:
    devMgr_->Clear();
    pthread_rwlock_unlock(&stateLock);
    POS_TRACE_ERROR(ret, "failed to create array {}", name_);
    return ret;
}

int
Array::Init(void)
{
    // TODO_MOUNTSEQUENCE: rollback sequence for array mount
    // pthread_rwlock_wrlock(&stateLock);
    POS_TRACE_INFO((int)POS_EVENT_ID::ARRAY_DEBUG_MSG, "Array {} Init", name_);
    int ret = 0;
    if (state->Exists() == false)
    {
        ret = (int)POS_EVENT_ID::ARRAY_STATE_NOT_EXIST;
        POS_TRACE_WARN(ret, "Array {} is not exist", name_);
        goto error;
    }

    ret = _LoadImpl();
    if (ret != 0)
    {
        goto error;
    }

    ret = state->IsMountable();
    if (ret != 0)
    {
        goto error;
    }

    ret = _CreatePartitions();
    if (ret != 0)
    {
        goto error;
    }
    _RegisterService();
    state->SetMount();
    // pthread_rwlock_unlock(&stateLock);
    POS_TRACE_INFO((int)POS_EVENT_ID::ARRAY_DEBUG_MSG, "Array {} was successfully mounted", name_);
    return ret;

error:
    _UnregisterService();
    // pthread_rwlock_unlock(&stateLock);
    POS_TRACE_ERROR(ret, "Failed to mount array {}", name_);
    return ret;
}

void
Array::Dispose(void)
{
    // pthread_rwlock_wrlock(&stateLock);
    POS_TRACE_INFO((int)POS_EVENT_ID::ARRAY_DEBUG_MSG, "Dispose array {}", name_);
    _UnregisterService();
    _DeletePartitions();
    state->SetUnmount();
    // pthread_rwlock_unlock(&stateLock);
}

void
Array::Shutdown(void)
{
    POS_TRACE_INFO((int)POS_EVENT_ID::ARRAY_DEBUG_MSG, "Shutdown array {}", name_);
    _UnregisterService();
    _DeletePartitions();
    shutdownFlag = 1;
}

int
Array::Delete(void)
{
    pthread_rwlock_wrlock(&stateLock);
    int ret = state->IsDeletable();
    if (ret != 0)
    {
        goto error;
    }

    if (state->IsBroken())
    {
        int waitcount = 0;
        while(shutdownFlag == 0) // Broken State automatically triggers Shutdown to all array components
        {
            POS_TRACE_INFO((int)POS_EVENT_ID::ARRAY_DEBUG_MSG , "Wait for shutdown done");
            usleep(100000);
            waitcount++;

            if (waitcount > 50)
            {
                ret = (int)POS_EVENT_ID::ARRAY_SHUTDOWN_TAKES_TOO_LONG;
                goto error;
            }
        }
        shutdownFlag = 0;
    }

    devMgr_->Clear();
    ret = abrControl->DeleteAbr(name_, meta_);
    if (ret != 0)
    {
        goto error;
    }

    state->SetDelete();

    pthread_rwlock_unlock(&stateLock);
    POS_TRACE_INFO((int)POS_EVENT_ID::ARRAY_DEBUG_MSG, "Array {} was successfully deleted", name_);
    return 0;

error:
    pthread_rwlock_unlock(&stateLock);
    POS_TRACE_ERROR(ret, "Failed to delete array {}", name_);
    return ret;
}

int
Array::AddSpare(string devName)
{
    pthread_rwlock_rdlock(&stateLock);

    int ret = state->CanAddSpare();
    if (ret != 0)
    {
        pthread_rwlock_unlock(&stateLock);
        POS_TRACE_ERROR(ret, "Failed to add spare device to array {}", name_);
        return ret;
    }
    ret = devMgr_->AddSpare(devName);
    if (0 != ret)
    {
        pthread_rwlock_unlock(&stateLock);
        POS_TRACE_ERROR(ret, "Failed to add spare device to array {}", name_);
        return ret;
    }
    ret = _Flush();
    if (0 != ret)
    {
        pthread_rwlock_unlock(&stateLock);
        POS_TRACE_ERROR(ret, "Failed to add spare device to array {}", name_);
        return ret;
    }

    EventSmartPtr event(new RebuildHandler(this, nullptr));
    eventScheduler->EnqueueEvent(event);
    pthread_rwlock_unlock(&stateLock);
    POS_TRACE_INFO((int)POS_EVENT_ID::ARRAY_DEVICE_ADDED, "Spare device was successfully added to array {}", name_);

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

    POS_TRACE_INFO((int)POS_EVENT_ID::ARRAY_DEVICE_REMOVED,
        "The spare device {} removed from array {}", devName, name_);
    return 0;

error:
    pthread_rwlock_unlock(&stateLock);
    POS_TRACE_ERROR(ret, "Failed to remove spare device {} from array {}", devName, name_);
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

string
Array::GetMetaRaidType(void)
{
    return meta_.metaRaidType;
}

string
Array::GetDataRaidType(void)
{
    return meta_.dataRaidType;
}

string
Array::GetCreateDatetime(void)
{
    return meta_.createDatetime;
}

string
Array::GetUpdateDatetime(void)
{
    return meta_.updateDatetime;
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

int
Array::_Flush(void)
{
    meta_.arrayName = GetName();
    meta_.metaRaidType = GetMetaRaidType();
    meta_.dataRaidType = GetDataRaidType();
    meta_.devs = devMgr_->ExportToMeta();

    return abrControl->SaveAbr(name_, meta_);
}

int
Array::_CreatePartitions(void)
{
    DeviceSet<ArrayDevice*> devs = devMgr_->Export();
    return ptnMgr->CreateAll(devs.nvm, devs.data, intf);
}

void
Array::_DeletePartitions(void)
{
    ptnMgr->DeleteAll(intf);
}

void
Array::_FormatMetaPartition(void)
{
    DeviceSet<ArrayDevice*> devs = devMgr_->Export();
    ptnMgr->FormatMetaPartition(devs.data, intf);
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
            if (pthread_rwlock_trywrlock(&stateLock) == 0)
            {
                POS_TRACE_INFO((int)POS_EVENT_ID::ARRAY_DEVICE_DETACHED,
                    "Spare-device {} is detached from array {}", devName, name_);
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
            eventId = (int)POS_EVENT_ID::ARRAY_DEVICE_DETACHED;
            POS_TRACE_ERROR(eventId,
                "Not allowed device {} is detached from array {}", devName, name_);
            break;
        }
    }

    return eventId;
}

void
Array::MountDone(void)
{
    _CheckRebuildNecessity();
}

int
Array::CheckUnmountable(void)
{
    return state->IsUnmountable();
}

int
Array::CheckDeletable(void)
{
    return state->IsDeletable();
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
            POS_TRACE_DEBUG((int)POS_EVENT_ID::ARRAY_DEBUG_MSG, "Resume Rebuild with rebuildDevice {}", devName);
            EventSmartPtr event(new RebuildHandler(this, rebuildDevice));
            eventScheduler->EnqueueEvent(event);
        }
        else
        {
            POS_TRACE_ERROR((int)POS_EVENT_ID::ARRAY_DEBUG_MSG, "Resume Rebuild without rebuildDevice");
        }
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
    POS_TRACE_INFO((int)POS_EVENT_ID::ARRAY_DEVICE_DETACHED,
        "Data device {} is detached from array {}", target->GetUblock()->GetName(), name_);
    bool isRebuildingDevice = false;
    ArrayDeviceState devState = target->GetState();
    if (devState == ArrayDeviceState::FAULT)
    {
        return;
    }
    else if (target->GetState() == ArrayDeviceState::REBUILD)
    {
        isRebuildingDevice = true;
    }

    state->DataRemoved(isRebuildingDevice);
    target->SetState(ArrayDeviceState::FAULT);
    sysDevMgr->RemoveDevice(target->GetUblock());
    target->SetUblock(nullptr);

    bool isRebuildable = state->IsRebuildable();

    if (state->IsMounted())
    {
        int ret = _Flush();
        if (0 != ret)
        {
            return;
        }
    }

    if (isRebuildingDevice || state->IsBroken())
    {
        target->SetRebuild(false);
        rebuilder->StopRebuild(name_);
    }
    else if (isRebuildable)
    {
        EventSmartPtr event(new RebuildHandler(this, target));
        eventScheduler->EnqueueEvent(event);
    }
}

void
Array::_RebuildDone(RebuildResult result)
{
    POS_TRACE_DEBUG((int)POS_EVENT_ID::REBUILD_DEBUG_MSG,
        "Array {} rebuild done. result:{}", name_, result.result);
    pthread_rwlock_wrlock(&stateLock);
    if (result.result != RebuildState::PASS)
    {
        state->SetRebuildDone(false);
        pthread_rwlock_unlock(&stateLock);
        return;
    }

    result.target->SetState(ArrayDeviceState::NORMAL);
    state->SetRebuildDone(true);
    int ret = _Flush();
    if (0 != ret)
    {
        POS_TRACE_ERROR(ret, "Array {} failed to save the device state from rebuild to normal", name_);
    }
    pthread_rwlock_unlock(&stateLock);
    rebuilder->RebuildDone(name_);
}

bool
Array::TriggerRebuild(ArrayDevice* target)
{
    POS_TRACE_DEBUG((int)POS_EVENT_ID::ARRAY_DEBUG_MSG, "Trigger Rebuild Start");
    bool retry = false;

    pthread_rwlock_wrlock(&stateLock);
    if (target == nullptr)
    {
        POS_TRACE_DEBUG(POS_EVENT_ID::REBUILD_DEBUG_MSG,
            "TryRebuild::rebuild target device is not existed");
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
    target->SetRebuild(true);
    int ret = devMgr_->ReplaceWithSpare(target);
    if (ret != 0)
    {
        state->SetRebuildDone(false);
        state->SetDegraded();
        target->SetRebuild(false);
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

    POS_TRACE_DEBUG(POS_EVENT_ID::ARRAY_DEBUG_MSG, "Preparing Rebuild");
    IArrayRebuilder* arrRebuilder = rebuilder;
    string arrName = name_;
    RebuildComplete cb = std::bind(&Array::_RebuildDone, this, placeholders::_1);
    list<RebuildTarget*> tasks = intf->GetRebuildTargets();
    thread t([arrRebuilder, arrName, target, cb, tasks]() {
        arrRebuilder->Rebuild(arrName, target, cb, tasks);
    });

    t.detach();
    POS_TRACE_DEBUG((int)POS_EVENT_ID::ARRAY_DEBUG_MSG, "Trigger Rebuild End");
    return retry;
}

bool
Array::ResumeRebuild(ArrayDevice* target)
{
    POS_TRACE_DEBUG((int)POS_EVENT_ID::ARRAY_DEBUG_MSG, "Resume Rebuild Start");

    pthread_rwlock_wrlock(&stateLock);
    assert(target != nullptr);

    if (state->SetRebuild() == false)
    {
        POS_TRACE_WARN(POS_EVENT_ID::REBUILD_TRIGGER_FAIL,
            "Failed to resume rebuild. Array {}'s state is not rebuildable", name_);
        pthread_rwlock_unlock(&stateLock);
        return false;
    }

    pthread_rwlock_unlock(&stateLock);

    POS_TRACE_DEBUG(POS_EVENT_ID::ARRAY_DEBUG_MSG, "Preparing Rebuild");
    IArrayRebuilder* arrRebuilder = rebuilder;
    string arrName = name_;
    RebuildComplete cb = std::bind(&Array::_RebuildDone, this, placeholders::_1);
    list<RebuildTarget*> tasks = intf->GetRebuildTargets();
    thread t([arrRebuilder, arrName, target, cb, tasks]()
    {
        arrRebuilder->Rebuild(arrName, target, cb, tasks);
    });

    t.detach();
    POS_TRACE_DEBUG((int)POS_EVENT_ID::ARRAY_DEBUG_MSG, "Resume Rebuild End");
    return true;
}

void
Array::_RegisterService(void)
{
    ArrayService::Instance()->Setter()->Register(name_, intf->GetTranslator(),
        intf->GetRecover(), this);
}

void
Array::_UnregisterService(void)
{
    ArrayService::Instance()->Setter()->Unregister(name_);
}

void
Array::_ResetMeta(void)
{
    meta_.arrayName = "";
    meta_.createDatetime = "";
    meta_.updateDatetime = "";
    meta_.devs.nvm.clear();
    meta_.devs.data.clear();
    meta_.devs.spares.clear();
}

} // namespace pos

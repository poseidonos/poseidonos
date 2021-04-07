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

#include "src/allocator/allocator.h"
#include "src/array/array_name_policy.h"
#include "src/array/device/array_device_remove_handler.h"
#include "src/array/ft/rebuild_handler.h"
#include "src/device/device_manager.h"
#include "src/gc/garbage_collector.h"
#include "src/logger/logger.h"
#include "src/master_context/mbr_manager.h"
#include "src/scheduler/event_argument.h"

namespace ibofos
{
Array::Array()
: metaMgr_(MbrManagerSingleton::Instance()),
  devMgr_(DeviceManagerSingleton::Instance()),
  sysDevMgr(DeviceManagerSingleton::Instance())
{
    pthread_rwlock_init(&stateLock, nullptr);
}

Array::~Array()
{
}

bool
Array::ArrayExist(string arrayName)
{
    return arrayName == name_;
}

string
Array::GetCurrentStateStr(void)
{
    return state_.GetCurrentStateStr();
}

int
Array::Load(string arrayName)
{
    pthread_rwlock_wrlock(&stateLock);
    int ret = _LoadImpl(arrayName);
    pthread_rwlock_unlock(&stateLock);
    if (ret != 0)
    {
        IBOF_TRACE_ERROR(ret, "Failed to load Array");
    }
    else
    {
        IBOF_TRACE_INFO(ret, "Array loaded successfully");
    }
    return ret;
}

int
Array::_LoadImpl(string arrayName)
{
    int ret = state_.IsLoadable();
    if (ret != 0)
    {
        return ret;
    }

    devMgr_.Clear();
    ArrayMeta meta;
    ret = metaMgr_.Load(meta, arrayName);
    if (ret != 0)
    {
        return ret;
    }

    uint32_t missingCnt = 0;
    uint32_t brokenCnt = 0;
    ret = devMgr_.Import(meta.devs, missingCnt, brokenCnt);
    if (ret != 0)
    {
        state_.SetDelete();
        return ret;
    }
    state_.SetLoad(missingCnt, brokenCnt);
    SetArrayName(arrayName);
    return ret;
}

int
Array::Create(DeviceSet<string> nameSet, string arrayName, string metaRaidType, string dataRaidType)
{
    int ret = 0;
    pthread_rwlock_wrlock(&stateLock);
    ArrayNamePolicy namePolicy;
    ret = state_.IsCreatable();
    if (ret != 0)
    {
        goto error;
    }

    ret = devMgr_.Import(nameSet);
    if (ret != 0)
    {
        goto error;
    }

    ret = namePolicy.CheckArrayName(arrayName);
    if (ret != (int)IBOF_EVENT_ID::SUCCESS)
    {
        goto error;
    }

    if (dataRaidType != "RAID5")
    {
        ret = (int)IBOF_EVENT_ID::ARRAY_WRONG_FT_METHOD;
        goto error;
    }

    SetArrayName(arrayName);
    SetMetaRaidType(metaRaidType);
    SetDataRaidType(dataRaidType);

    metaMgr_.Reset();
    ret = _Flush();
    if (ret != 0)
    {
        goto error;
    }
    state_.SetCreate();

    pthread_rwlock_unlock(&stateLock);
    IBOF_TRACE_INFO(ret, "Array was successfully created");
    return 0;

error:
    pthread_rwlock_unlock(&stateLock);
    IBOF_TRACE_ERROR(ret, "failed to create Array");
    return ret;
}

int
Array::Mount()
{
    pthread_rwlock_wrlock(&stateLock);
    int ret = 0;
    if (state_.Exists() == false)
    {
        ret = (int)IBOF_EVENT_ID::ARRAY_STATE_NOT_EXIST;
        IBOF_TRACE_WARN(ret, "Array is not exist");
        goto error;
    }

    ret = _LoadImpl(name_);
    if (ret != 0)
    {
        goto error;
    }

    ret = state_.IsMountable();
    if (ret != 0)
    {
        goto error;
    }

    ret = _CreatePartitions();
    if (ret != 0)
    {
        goto error;
    }

    ret = state_.SetMount();
    if (ret != 0)
    {
        goto error;
    }

    pthread_rwlock_unlock(&stateLock);
    IBOF_TRACE_INFO(ret, "Array was successfully mounted");
    return ret;

error:
    pthread_rwlock_unlock(&stateLock);
    IBOF_TRACE_ERROR(ret, "Failed to mount array");
    return ret;
}

int
Array::Unmount()
{
    pthread_rwlock_wrlock(&stateLock);
    int ret = state_.IsUnmountable();
    if (ret != 0)
    {
        goto error;
    }

    ptnMgr_.DeleteAll();

    ret = state_.SetUnmount();
    if (ret != 0)
    {
        goto error;
    }

    pthread_rwlock_unlock(&stateLock);
    IBOF_TRACE_INFO(ret, "Array was successfully unmounted");
    return 0;

error:
    pthread_rwlock_unlock(&stateLock);
    IBOF_TRACE_ERROR(ret, "Failed to unmount array");
    return ret;
}

int
Array::Delete(string arrayName)
{
    if (arrayName != name_)
    {
        IBOF_TRACE_WARN((int)IBOF_EVENT_ID::ARRAY_WRONG_NAME,
            "array requested does not exist");
        return (int)IBOF_EVENT_ID::ARRAY_WRONG_NAME;
    }

    pthread_rwlock_wrlock(&stateLock);
    int ret = state_.IsDeletable();
    if (ret != 0)
    {
        goto error;
    }

    devMgr_.Clear();
    ret = metaMgr_.Clear();
    if (ret != 0)
    {
        goto error;
    }

    state_.SetDelete();

    pthread_rwlock_unlock(&stateLock);
    IBOF_TRACE_INFO(ret, "Array was successfully deleted");
    return 0;

error:
    pthread_rwlock_unlock(&stateLock);
    IBOF_TRACE_ERROR(ret, "Failed to delete array");
    return ret;
}

int
Array::AddSpare(string devName, string arrayName)
{
    if (arrayName != name_)
    {
        IBOF_TRACE_WARN((int)IBOF_EVENT_ID::ARRAY_WRONG_NAME,
            "array requested does not exist");
        return (int)IBOF_EVENT_ID::ARRAY_WRONG_NAME;
    }

    pthread_rwlock_rdlock(&stateLock);

    int ret = state_.CanAddSpare();
    if (ret != 0)
    {
        pthread_rwlock_unlock(&stateLock);
        IBOF_TRACE_ERROR(ret, "Failed to add spare device");
        return ret;
    }
    ret = devMgr_.AddSpare(devName);
    if (0 != ret)
    {
        pthread_rwlock_unlock(&stateLock);
        IBOF_TRACE_ERROR(ret, "Failed to add spare device");
        return ret;
    }
    ret = _Flush();
    if (0 != ret)
    {
        pthread_rwlock_unlock(&stateLock);
        IBOF_TRACE_ERROR(ret, "Failed to add spare device");
        return ret;
    }

    bool isStop = false;
    EventSmartPtr event(new RebuildHandler(isStop));
    EventArgument::GetEventScheduler()->EnqueueEvent(event);

    pthread_rwlock_unlock(&stateLock);
    IBOF_TRACE_INFO(ret, "Spare device was successfully added");

    return 0;
}

int
Array::RemoveSpare(string devName, string arrayName)
{
    if (arrayName != name_)
    {
        IBOF_TRACE_WARN((int)IBOF_EVENT_ID::ARRAY_WRONG_NAME,
            "array requested does not exist");
        return (int)IBOF_EVENT_ID::ARRAY_WRONG_NAME;
    }

    pthread_rwlock_rdlock(&stateLock);

    int ret = state_.CanRemoveSpare();
    if (ret != 0)
    {
        goto error;
    }
    ret = devMgr_.RemoveSpare(devName);
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

    IBOF_TRACE_INFO((int)IBOF_EVENT_ID::ARRAY_DEVICE_REMOVED,
        "the SPARE device {} removed from the Array", devName);
    return 0;

error:
    pthread_rwlock_unlock(&stateLock);
    IBOF_TRACE_ERROR(ret, "Failed to remove spare device");
    return ret;
}

const PartitionLogicalSize*
Array::GetSizeInfo(PartitionType type)
{
    const PartitionLogicalSize* sizeInfo = nullptr;
    sizeInfo = ptnMgr_.GetSizeInfo(type);
    return sizeInfo;
}
int
Array::Translate(const PartitionType type,
    PhysicalBlkAddr& dst,
    const LogicalBlkAddr& src)
{
    return ptnMgr_.Translate(type, dst, src);
}

int
Array::Convert(const PartitionType type,
    list<PhysicalWriteEntry>& dst,
    const LogicalWriteEntry& src)
{
    return ptnMgr_.Convert(type, dst, src);
}

int
Array::_Flush(void)
{
    ArrayMeta meta;
    meta.arrayName = GetArrayName();
    meta.metaRaidType = GetMetaRaidType();
    meta.dataRaidType = GetDataRaidType();
    meta.devs = devMgr_.ExportToMeta();

    return metaMgr_.Store(meta);
}

int
Array::_CreatePartitions()
{
    DeviceSet<ArrayDevice*> devs = devMgr_.Export();
    if (devs.nvm.size() == 0)
    {
        return (int)IBOF_EVENT_ID::ARRAY_PARTITION_CREATION_ERROR;
    }
    ArrayDevice* nvm = devs.nvm.at(0);
    return ptnMgr_.CreateAll(nvm, devs.data);
}

bool
Array::IsRecoverableDevice(ArrayDevice* target)
{
    pthread_rwlock_wrlock(&stateLock);

    ArrayDeviceState devState = target->GetState();
    if (devState != ArrayDeviceState::FAULT)
    {
        IBOF_TRACE_INFO((int)IBOF_EVENT_ID::ARRAY_DEVICE_DETACHED,
            "Data-device {} is detached", target->uBlock->GetName());
        _DetachData(target);
    }

    bool recoverable = state_.IsRecoverable();

    pthread_rwlock_unlock(&stateLock);
    return recoverable;
}

bool
Array::IsRecoverableDevice(ArrayDevice* target, ArrayDeviceState oldState)
{
    pthread_rwlock_wrlock(&stateLock);
    bool recoverable = false;
    ArrayDeviceState currState = target->GetState();
    if (oldState == ArrayDeviceState::FAULT && currState != ArrayDeviceState::FAULT)
    {
        if (state_.IsMounted() && !state_.IsBroken())
        {
            recoverable = true;
        }
    }
    else
    {
        if (currState != ArrayDeviceState::FAULT)
        {
            IBOF_TRACE_INFO((int)IBOF_EVENT_ID::ARRAY_DEVICE_DETACHED,
                "Data-device {} is detached", target->uBlock->GetName());
            _DetachData(target);
        }
        recoverable = state_.IsRecoverable();
    }

    pthread_rwlock_unlock(&stateLock);
    return recoverable;
}

int
Array::DetachDevice(UBlockDevice* uBlock)
{
    ArrayDeviceType devType;
    ArrayDevice* dev = nullptr;
    string devName = uBlock->GetName();
    tie(dev, devType) = devMgr_.GetDev(uBlock);

    int eventId = 0;

    switch (devType)
    {
        case ArrayDeviceType::SPARE:
        {
            if (pthread_rwlock_trywrlock(&stateLock) == 0)
            {
                IBOF_TRACE_INFO((int)IBOF_EVENT_ID::ARRAY_DEVICE_DETACHED,
                    "Spare-device {} is detached", devName);
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
                    IBOF_TRACE_INFO((int)IBOF_EVENT_ID::ARRAY_DEVICE_DETACHED,
                        "Data-device {} is detached", devName);
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
            eventId = (int)IBOF_EVENT_ID::ARRAY_DEVICE_DETACHED;
            IBOF_TRACE_ERROR(eventId,
                "Not allowed device {} is detached", devName);
            break;
        }
    }

    return eventId;
}

void
Array::_DetachSpare(ArrayDevice* target)
{
    int ret = devMgr_.RemoveSpare(target);
    if (0 != ret)
    {
        return;
    }
    UBlockDevice* uBlock = target->uBlock;
    bool removed = target->TryRemoveUblock();
    if (removed == false)
    {
        assert(0);
    }
    sysDevMgr->RemoveDevice(uBlock);
    delete (target);
    if (state_.IsMounted())
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

    state_.DataRemoved(isRebuildingDevice);
    target->SetState(ArrayDeviceState::FAULT);
    bool isRebuildable = state_.IsRebuildable();

    if (state_.IsMounted())
    {
        int ret = _Flush();
        if (0 != ret)
        {
            return;
        }
    }

    EventScheduler* scheduler = EventArgument::GetEventScheduler();
    EventSmartPtr removeHandler(new ArrayDeviceRemoveHandler(target));
    scheduler->EnqueueEvent(removeHandler);

    if (isRebuildingDevice
        || StateManagerSingleton::Instance()->GetState() == State::STOP)
    {
        bool isStop = true;
        EventSmartPtr event(new RebuildHandler(isStop));
        scheduler->EnqueueEvent(event);
    }
    else if (isRebuildable)
    {
        bool isStop = false;
        EventSmartPtr event(new RebuildHandler(isStop, target));
        scheduler->EnqueueEvent(event);
    }
}

DeviceSet<string>
Array::GetDevNames()
{
    return devMgr_.ExportToName();
}

int
Array::RebuildRead(UbioSmartPtr ubio)
{
    return ptnMgr_.RebuildRead(ubio);
}

void
Array::Rebuild(ArrayDevice* target)
{
    ptnMgr_.Rebuild(target,
        bind(&Array::_RebuildDone, this, target, placeholders::_1));
}

void
Array::StopRebuilding()
{
    ptnMgr_.StopRebuilding();
}

void
Array::_RebuildDone(ArrayDevice* target, RebuildState result)
{
    pthread_rwlock_wrlock(&stateLock);
    AllocatorSingleton::Instance()->StopRebuilding();

    if (result != RebuildState::PASS)
    {
        state_.SetRebuildDone(false);
        pthread_rwlock_unlock(&stateLock);
        return;
    }

    target->SetState(ArrayDeviceState::NORMAL);
    state_.SetRebuildDone(true);
    int ret = _Flush();
    if (0 != ret)
    {
        IBOF_TRACE_ERROR(ret, "failed to complete rebuild");
    }
    pthread_rwlock_unlock(&stateLock);
}

bool
Array::TriggerRebuild(ArrayDevice* target)
{
    bool retry = false;

    pthread_rwlock_wrlock(&stateLock);
    if (target == nullptr)
    {
        IBOF_TRACE_DEBUG(IBOF_EVENT_ID::REBUILD_DEBUG_MSG,
            "TryRebuild::rebuild target device is not existed");
        target = devMgr_.GetFaulty();
        if (target == nullptr)
        {
            pthread_rwlock_unlock(&stateLock);
            return retry;
        }
    }
    if (target->GetState() != ArrayDeviceState::FAULT || target->uBlock != nullptr)
    {
        IBOF_TRACE_DEBUG(IBOF_EVENT_ID::REBUILD_DEBUG_MSG,
            "Rebuild target device is not removed yet");
        pthread_rwlock_unlock(&stateLock);
        retry = true;
        return retry;
    }

    if (state_.SetRebuild() == false)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::REBUILD_TRIGGER_FAIL,
            "Failed to trigger rebuild. Current array state is not rebuildable");
        pthread_rwlock_unlock(&stateLock);
        return retry;
    }
    // Degraded
    // System State Invoke Rebuilding
    int ret = devMgr_.ReplaceWithSpare(target);
    if (ret != 0)
    {
        state_.SetRebuildDone(false);
        state_.SetDegraded();
        IBOF_TRACE_WARN(IBOF_EVENT_ID::REBUILD_TRIGGER_FAIL,
            "Failed to trigger rebuild. spare device is not available");
        pthread_rwlock_unlock(&stateLock);
        return retry;
    }

    target->SetState(ArrayDeviceState::REBUILD);
    ret = _Flush();
    if (0 != ret)
    {
        IBOF_TRACE_WARN(IBOF_EVENT_ID::REBUILD_TRIGGER_FAIL,
            "Failed to trigger rebuild. Flush failed.");
        pthread_rwlock_unlock(&stateLock);
        return retry;
    }
    pthread_rwlock_unlock(&stateLock);

    thread t([target]() {
        Array* sysArray = ArraySingleton::Instance();
        int ret = sysArray->PrepareRebuild(target);
        if (ret == 0)
        {
            sysArray->Rebuild(target);
        }
    });

    t.detach();
    return retry;
}

int
Array::PrepareRebuild(ArrayDevice* target)
{
    IBOF_TRACE_DEBUG(IBOF_EVENT_ID::ARRAY_DEBUG_MSG, "Preparing Rebuild");

    GarbageCollectorSingleton::Instance()->End();
    int ret = AllocatorSingleton::Instance()->PrepareRebuild();
    GarbageCollectorSingleton::Instance()->Start();

    return ret;
}

void
Array::NotifyIbofosMounted()
{
    bool isStop = false;
    EventSmartPtr event(new RebuildHandler(isStop));
    EventArgument::GetEventScheduler()->EnqueueEvent(event);
}

bool
Array::TryLock(PartitionType type, StripeId stripeId)
{
    return ptnMgr_.TryLock(type, stripeId);
}

void
Array::Unlock(PartitionType type, StripeId stripeId)
{
    ptnMgr_.Unlock(type, stripeId);
}

uint32_t
Array::GetRebuildingProgress()
{
    return ptnMgr_.GetRebuildingProgress();
}

} // namespace ibofos

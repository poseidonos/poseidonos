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
#include <sstream>

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
#include "src/master_context/config_manager.h"

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
    debugArray.RegisterDebugInfoInstance("Array_Info_Array" + std::to_string(GetIndex()));
    debugArrayQueue.RegisterDebugInfoQueue("History_Array_Info_Array" + std::to_string(GetIndex()), 1000, true);
    RegisterDebugInfoMaker(&debugArray, &debugArrayQueue, false);
}

Array::~Array(void)
{
    delete publisher;
    delete svc;
    delete ptnMgr;
    delete state;
    delete devMgr_;
}

void
Array::MakeDebugInfo(DebugArray& obj)
{
    obj.arrayInfo = Serialize();
    obj.state = GetState().ToString();
    obj.rebuildProgress = rebuilder->GetRebuildProgress(name_);
    obj.isWTEnabled = isWTEnabled;
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
        POS_TRACE_TRACE(EID(POS_TRACE_ARRAY_LOADED), "{}", Serialize());
    }
    AddDebugInfo();
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

    publisher = new ArrayMetricsPublisher(this, state);
    RaidState rs = ptnMgr->GetRaidState();
    state->SetLoad(rs);
    return ret;
}

int
Array::Create(DeviceSet<string> nameSet, string metaFt, string dataFt)
{
    RaidType dataRaidType = RaidType(dataFt);
    RaidType metaRaidType = RaidType(metaFt);
    POS_TRACE_INFO(EID(CREATE_ARRAY_DEBUG_MSG), "Trying to create array({}), metaFt:{}, dataFt:{}",
        name_, metaRaidType.ToString(), dataRaidType.ToString());

    if (dataRaidType == RaidTypeEnum::NOT_SUPPORTED ||
        metaRaidType == RaidTypeEnum::NOT_SUPPORTED)
    {
        int ret = EID(CREATE_ARRAY_NOT_SUPPORTED_RAIDTYPE);
        POS_TRACE_WARN(ret, "metaFt: {}, dataFt: {}", metaFt, dataFt);
        return ret;
    }
    bool canAddSpare = dataRaidType != RaidTypeEnum::NONE &&
                dataRaidType != RaidTypeEnum::RAID0;
    if (canAddSpare == false && nameSet.spares.size() > 0)
    {
        int ret = EID(CREATE_ARRAY_RAID_DOES_NOT_SUPPORT_SPARE_DEV);
        POS_TRACE_WARN(ret, "RaidType: {}", dataRaidType.ToString());
        return ret;
    }

    int ret = 0;
    ArrayMeta meta;
    ArrayNamePolicy namePolicy;
    UniqueIdGenerator uIdGen;

    pthread_rwlock_wrlock(&stateLock);
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
        abrControl->DeleteAbr(name_);
        goto error;
    }

    ret = _CreatePartitions(RaidType(meta.metaRaidType), RaidType(meta.dataRaidType));
    if (ret != 0)
    {
        abrControl->DeleteAbr(name_);
        goto error;
    }
    ptnMgr->FormatPartition(PartitionType::META_SSD, index_, IODispatcherSingleton::Instance());

    publisher = new ArrayMetricsPublisher(this, state);
    state->SetCreate();
    pthread_rwlock_unlock(&stateLock);
    POS_TRACE_TRACE(EID(POS_TRACE_ARRAY_CREATED), "{}", Serialize());
    AddDebugInfo();
    return 0;

error:
    devMgr_->Clear();
    pthread_rwlock_unlock(&stateLock);
    POS_TRACE_TRACE(ret, "Unable to create array({})", name_);
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
    POS_TRACE_TRACE(EID(POS_TRACE_ARRAY_UNMOUNTED), "array_name:{}", name_);
    // pthread_rwlock_unlock(&stateLock);
}

void
Array::Shutdown(void)
{
    POS_TRACE_INFO(EID(UNMOUNT_BROKEN_ARRAY_DEBUG_MSG), "Trying to shut down broken array({})", name_);
    _UnregisterService();
    state->SetShutdown();
    isWTEnabled = false;
    POS_TRACE_TRACE(EID(POS_TRACE_ARRAY_SHUTDOWN), "array_name:{}", name_);
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
    POS_TRACE_TRACE(EID(POS_TRACE_ARRAY_DELETED), "array_name:{}", name_);
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
    if (_CanAddSpare() == false)
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
        ret = EID(ADD_SPARE_SSD_NAME_NOT_FOUND);
        POS_TRACE_WARN(ret, "devName: {}", devName);
        return ret;
    }

    string spareSN = dev->GetSN();
    string involvedArray = abrControl->FindArrayWithDeviceSN(spareSN);
    if (involvedArray != "")
    {
        pthread_rwlock_unlock(&stateLock);
        ret = EID(ADD_SPARE_DEVICE_ALREADY_OCCUPIED);
        POS_TRACE_WARN(ret, "dev_name:{}, occupier:{}", devName, involvedArray);
        return ret;
    }

    ret = devMgr_->AddSpare(devName);
    if (0 != ret)
    {
        pthread_rwlock_unlock(&stateLock);
        POS_TRACE_WARN(ret, "Unable to add spare device to array({})", name_);
        return ret;
    }
    ret = _Flush();
    if (0 != ret)
    {
        pthread_rwlock_unlock(&stateLock);
        POS_TRACE_ERROR(ret, "Unable to add spare device to array({})", name_);
        return ret;
    }

    vector<IArrayDevice*> targets = devMgr_->GetFaulty();
    if (targets.size() > 0 && state->IsRebuildable())
    {
        POS_TRACE_INFO(EID(INVOKE_REBUILD_DUE_TO_SPARE_ADDED), "array_name:{}, targetCnt:{}",
            name_, targets.size());
        bool isResume = false;
        InvokeRebuild(targets, isResume);
    }
    pthread_rwlock_unlock(&stateLock);
    POS_TRACE_INFO(EID(ADD_SPARE_DEBUG_MSG), "Spare device {} was successfully added to array({})", devName, name_);

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

    POS_TRACE_INFO(EID(REMOVE_DEV_DEBUG_MSG),
        "The spare device {} removed from array({})", devName, name_);
    return 0;

error:
    pthread_rwlock_unlock(&stateLock);
    POS_TRACE_ERROR(ret, "Unable to remove spare device {} from array({})", devName, name_);
    return ret;
}

int
Array::ReplaceDevice(string devName)
{
    pthread_rwlock_wrlock(&stateLock);
    ArrayDeviceType devType = ArrayDeviceType::NONE;
    ArrayDevice* target = nullptr;
    tie(target, devType) = devMgr_->GetDevByName(devName);
    POS_TRACE_INFO(EID(REPLACE_DEV_DEBUG_MSG), "trying to replace device from array, dev_name:{}, dev_type:{} array_name:{}", devName, devType, name_);
    int ret = 0;
    if (devType == ArrayDeviceType::DATA && target != nullptr)
    {
        ret = state->CanReplaceData();
        if (ret == 0)
        {
            if (target->GetState() == ArrayDeviceState::NORMAL)
            {
                if (_CanAddSpare() == false)
                {
                    ret = EID(REPLACE_DEV_UNSUPPORTED_RAID_TYPE);
                    POS_TRACE_WARN(ret, "meta_raid:{}, data_raid:{}", GetMetaRaidType(), GetDataRaidType());
                }
                else
                {
                    vector<ArrayDevice*> spares = devMgr_->GetAvailableSpareDevices();
                    if (spares.size() == 0)
                    {
                        ret = EID(REPLACE_DEV_NO_AVAILABLE_SPARE);
                        POS_TRACE_WARN(ret, "");
                    }
                    else
                    {
                        target->SetState(ArrayDeviceState::REBUILD);
                        RaidState rs = ptnMgr->GetRaidState();
                        state->RaidStateUpdated(rs);
                        if (state->SetRebuild() == false)
                        {
                            // a rebuild state must be obtained.
                            assert(false);
                        }
                        ArrayDevice* swapOut = nullptr;
                        ret = devMgr_->ReplaceWithSpare(target, swapOut);
                        if (ret != 0)
                        {
                            // rollback tasks performed.
                            state->SetRebuildDone(false);
                            target->SetState(ArrayDeviceState::NORMAL);
                            RaidState rs = ptnMgr->GetRaidState();
                            state->RaidStateUpdated(rs);
                        }
                        else
                        {
                            _Flush();
                            POS_TRACE_TRACE(EID(REPLACE_DEV_DEBUG_MSG),
                                "device {} is replaced to {} successfully, array:{}", devName, target->GetName(), name_);
                            DoRebuildAsync(vector<IArrayDevice*>{target}, vector<IArrayDevice*>{swapOut}, RebuildTypeEnum::QUICK);
                        }
                    }
                }
            }
            else
            {
                ret = EID(REPLACE_DEV_ONLY_NORMAL_DEV_CAN_BE_REMOVED);
                POS_TRACE_WARN(ret, "dev_status:{} (0-NORMAL, 1-FAULT, 2-REBUILD)", target->GetState());
            }
        }
    }
    else
    {
        ret = EID(REPLACE_DEV_SSD_NAME_NOT_FOUND);
        POS_TRACE_WARN(ret, "devName:{}", devName);
    }
    pthread_rwlock_unlock(&stateLock);
    return ret;
}

int
Array::Rebuild(void)
{
    POS_TRACE_INFO(EID(INVOKE_REBUILD_DUE_TO_USER_REQUEST), "array_name:{}", name_);
    pthread_rwlock_rdlock(&stateLock);
    int eid = 0;

    if (devMgr_->GetSpareDevices().size() == 0)
    {
        eid = EID(REBUILD_ARRAY_SPARE_DOES_NOT_EXIST);
        pthread_rwlock_unlock(&stateLock);
        return eid;
    }
    if (_CanAddSpare() == false)
    {
        eid = EID(REBUILD_ARRAY_RAID_NOT_SUPPORTED);
        pthread_rwlock_unlock(&stateLock);
        return eid;
    }
    if (rebuilder->IsRebuilding(name_) == true)
    {
        eid = EID(REBUILD_ARRAY_ALREADY_IN_PROGRESS);
        pthread_rwlock_unlock(&stateLock);
        return eid;
    }
    if (state->IsMountable() == true)
    {
        eid = EID(REBUILD_ARRAY_OFFLINED);
        pthread_rwlock_unlock(&stateLock);
        return eid;
    }
    if (state->IsRebuildable() == false) 
    {
        eid = EID(REBUILD_ARRAY_IS_NORMAL);
        pthread_rwlock_unlock(&stateLock);
        return eid;
    }

    // Devices that have been suspended during rebuilding have priority for rebuild
    vector<IArrayDevice*> targets = devMgr_->GetRebuilding();
    bool isResume = true;
    if (targets.size() == 0)
    {
        targets = devMgr_->GetFaulty();
        isResume = false;
    }

    // Unexpected case handling: spare exists and the state is degraded, but if there is no target
    assert (targets.size() != 0);
    bool forceRebuild = true;
    InvokeRebuild(targets, isResume, forceRebuild);
    pthread_rwlock_unlock(&stateLock);
    AddDebugInfo();
    return 0;
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
    AddDebugInfo();
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

int
Array::IsRecoverable(IArrayDevice* target, UBlockDevice* uBlock)
{
    if (pthread_rwlock_trywrlock(&stateLock) != 0)
    {
        return static_cast<int>(IoRecoveryRetType::RETRY);
    }
    if (state->IsBroken() || !state->IsMounted())
    {
        pthread_rwlock_unlock(&stateLock);
        return static_cast<int>(IoRecoveryRetType::FAIL);
    }

    if (uBlock == nullptr                       // Translate / Covnert fail
        || target->GetUblock().get() != uBlock) // Detached device after address translation
    {
        pthread_rwlock_unlock(&stateLock);
        return static_cast<int>(IoRecoveryRetType::SUCCESS);
    }

    _DetachData(static_cast<ArrayDevice*>(target));
    bool ret = state->IsRecoverable();
    pthread_rwlock_unlock(&stateLock);
    if (ret == true)
    {
        return static_cast<int>(IoRecoveryRetType::SUCCESS);
    }
    return static_cast<int>(IoRecoveryRetType::FAIL);
}

IArrayDevice*
Array::FindDevice(string devSn)
{
    ArrayDeviceType devType = ArrayDeviceType::NONE;
    ArrayDevice* dev = nullptr;
    tie(dev, devType) = devMgr_->GetDevBySn(devSn);
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
    if (isWTEnabled == true)
    {
        POS_TRACE_INFO(EID(MOUNT_ARRAY_DEBUG_MSG), "WT of Array {} is enabled",
            name_);
    }
    else
    {
        POS_TRACE_INFO(EID(MOUNT_ARRAY_DEBUG_MSG), "WT of Array {} is disabled",
            name_);
    }
}

void
Array::MountDone(void)
{
    POS_TRACE_TRACE(EID(POS_TRACE_ARRAY_MOUNTED), "{}", Serialize());
    vector<IArrayDevice*> suspendedTargets = devMgr_->GetRebuilding();
    AddDebugInfo();
    if (suspendedTargets.size() > 0 && state->IsRebuildable())
    {
        POS_TRACE_INFO(EID(INVOKE_RESUME_REBUILD), "array_name:{}, targetCnt:{}",
            name_, suspendedTargets.size());
        bool isResume = true;
        InvokeRebuild(suspendedTargets, isResume);
    }
    else
    {
        vector<IArrayDevice*> targets = devMgr_->GetFaulty();
        if (targets.size() > 0 && state->IsRebuildable())
        {
            POS_TRACE_INFO(EID(INVOKE_REBUILD_DUE_TO_ARRAY_MOUNTED), "array_name:{}, targetCnt:{}",
                name_, targets.size());
            bool isResume = false;
            InvokeRebuild(targets, isResume);
        }
    }
    int ret = _Flush();
    assert(ret == 0);
}

int
Array::CheckUnmountable(void)
{
    return state->IsUnmountable();
}

string
Array::Serialize(void)
{
    vector<string> arrayinfo;
    arrayinfo.push_back("name:" + name_);
    arrayinfo.push_back("index:" + to_string(index_));
    arrayinfo.push_back("uuid:" + to_string(uniqueId));
    arrayinfo.push_back("raidtype_meta:" + GetMetaRaidType());
    arrayinfo.push_back("raidtype_data:" + GetDataRaidType());
    arrayinfo.push_back("date_created:" + GetCreateDatetime());
    StateContext* stateCtx = GetStateCtx();
    if (stateCtx != nullptr)
    {
        arrayinfo.push_back("state:" + stateCtx->ToStateType().ToString());
        arrayinfo.push_back("situation:" + stateCtx->GetSituation().ToString());
    }
    string wt = IsWriteThroughEnabled() == true ? "true" : "false";
    arrayinfo.push_back("wt_enabled:" + wt);
    DeviceSet<string> nameSet = GetDevNames();
    {
        string devs = "buffer_devs:";
        int cnt = 0;
        for (string name : nameSet.nvm)
        {
            if (cnt != 0)
            {
                devs += ",";
            }
            devs += name;
            cnt++;
        }
        arrayinfo.push_back(devs);
    }
    {
        string devs = "data_devs:";
        int cnt = 0;
        for (string name : nameSet.data)
        {
            if (cnt != 0)
            {
                devs += ",";
            }
            devs += name;
            cnt++;
        }
        arrayinfo.push_back(devs);
    }
    {
        string devs = "spare_devs:";
        int cnt = 0;
        for (string name : nameSet.spares)
        {
            if (cnt != 0)
            {
                devs += ",";
            }
            devs += name;
            cnt++;
        }
        arrayinfo.push_back(devs);
    }
    {
        const PartitionLogicalSize* lSize = GetSizeInfo(PartitionType::JOURNAL_SSD);
        if (lSize != nullptr)
        {
            arrayinfo.push_back("part_journal_ssd_segments:" + to_string(lSize->totalSegments));
        }
        const PartitionPhysicalSize* pSize = ptnMgr->GetPhysicalSize(PartitionType::JOURNAL_SSD);
        if (pSize != nullptr)
        {
            arrayinfo.push_back("part_journal_ssd_start_lba:" + to_string(pSize->startLba));
            arrayinfo.push_back("part_journal_ssd_last_lba:" + to_string(pSize->lastLba));
        }
    }
    {
        const PartitionLogicalSize* lSize = GetSizeInfo(PartitionType::META_SSD);
        if (lSize != nullptr)
        {
            arrayinfo.push_back("part_meta_ssd_segments:" + to_string(lSize->totalSegments));
        }
        const PartitionPhysicalSize* pSize = ptnMgr->GetPhysicalSize(PartitionType::META_SSD);
        if (pSize != nullptr)
        {
            arrayinfo.push_back("part_meta_ssd_start_lba:" + to_string(pSize->startLba));
            arrayinfo.push_back("part_meta_ssd_last_lba:" + to_string(pSize->lastLba));
        }
    }
    {
        const PartitionLogicalSize* lSize = GetSizeInfo(PartitionType::USER_DATA);
        if (lSize != nullptr)
        {
            arrayinfo.push_back("part_user_data_segments:" + to_string(lSize->totalSegments));
        }
        const PartitionPhysicalSize* pSize = ptnMgr->GetPhysicalSize(PartitionType::USER_DATA);
        if (pSize != nullptr)
        {
            arrayinfo.push_back("part_user_data_start_lba:" + to_string(pSize->startLba));
            arrayinfo.push_back("part_user_data_last_lba:" + to_string(pSize->lastLba));
        }
    }

    stringstream ss;
    for (string str : arrayinfo)
    {
        ss << str << ", ";
    }
    return ss.str();
}

bool
Array::_CanAddSpare(void)
{
    RaidType raidType = RaidType(GetDataRaidType());
    return raidType != RaidTypeEnum::NONE && raidType != RaidTypeEnum::RAID0;
}

void
Array::_DetachSpare(ArrayDevice* target)
{
    UblockSharedPtr uBlock = target->GetUblock();
    AddDebugInfo();
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

    bool needStopRebuild = target->GetState() == ArrayDeviceState::REBUILD && state->IsRebuilding() == true;
    target->SetState(ArrayDeviceState::FAULT);
    RaidState rs = ptnMgr->GetRaidState();
    if (rs == RaidState::FAILURE || state->IsRebuilding() == false)
    {
        state->RaidStateUpdated(rs);
    }
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

    if (needStopRebuild)
    {
        rebuilder->StopRebuild(name_, EID(REBUILD_STOPPED_OTHER_DEVS_DETACHMENT));
    }
    else if (state->IsRebuildable())
    {
        vector<IArrayDevice*> targets {target};
        POS_TRACE_INFO(EID(INVOKE_REBUILD_DUE_TO_DEVICE_DETACHED), "array_name:{}, targetCnt:{}",
            name_, targets.size());
        bool isResume = false;
        InvokeRebuild(targets, isResume);
    }
    AddDebugInfo();
}

void
Array::_RebuildDone(vector<IArrayDevice*> dsts, vector<IArrayDevice*> srcs, RebuildResult result)
{
    POS_TRACE_INFO(EID(REBUILD_DONE),
        "array_name:{}, result:{}", name_, REBUILD_STATE_STR[(int)result.result]);
    rebuilder->RebuildDone(result);
    pthread_rwlock_wrlock(&stateLock);
    for (IArrayDevice* src : srcs)
    {
        // in case of the quick rebuild completed
        UblockSharedPtr ublock = src->GetUblock();
        if (ublock != nullptr)
        {
            ublock->SetClass(DeviceClass::SYSTEM);
            src->SetUblock(nullptr);
        }
        delete src;
    }
    if (result.result == RebuildState::PASS)
    {
        for (IArrayDevice* dst : dsts)
        {
            dst->SetState(ArrayDeviceState::NORMAL);
        }
        int ret = _Flush();
        if (0 != ret)
        {
            POS_TRACE_ERROR(ret, "Unable to update the device state of array({})", name_);
        }
    }
    RaidState rs = ptnMgr->GetRaidState();
    state->RaidStateUpdated(rs);

    vector<IArrayDevice*> targets = devMgr_->GetFaulty();
    if (targets.size() > 0 && state->IsRebuildable())
    {
        POS_TRACE_INFO(EID(INVOKE_REBUILD_AFTER_REBUILD_DONE), "array_name:{}, targetCnt:{}",
            name_, targets.size());
        bool isResume = false;
        InvokeRebuild(targets, isResume);
    }
    pthread_rwlock_unlock(&stateLock);
}

void
Array::InvokeRebuild(vector<IArrayDevice*> targets, bool isResume, bool force)
{
    if (force == false)
    {
        bool isAutoRebuild = true;
        ConfigManagerSingleton::Instance()->GetValue("rebuild", "auto_start",
            &isAutoRebuild, ConfigType::CONFIG_TYPE_BOOL);
        if (isAutoRebuild == false)
        {
            POS_TRACE_WARN(EID(RECOMMEND_PERFORM_REBUILD), "array_name:{}", name_);
            return;
        }
    }
    POS_TRACE_INFO(EID(TRIGGER_REBUILD_DEBUG), "array_name:{}, targetCnt:{}, isForce:{}, isResume:{}",
        name_, targets.size(), force, isResume);
    EventSmartPtr event(new RebuildHandler(this, targets, isResume));
    eventScheduler->EnqueueEvent(event);
}

bool
Array::TriggerRebuild(vector<IArrayDevice*> targets)
{
    bool retry = false;
    if (targets.size() == 0)
    {
        POS_TRACE_WARN(EID(REBUILD_ARRAY_DEBUG_MSG), "Rebuild triggered but no targets exist, ignored");
        return retry;
    }
    POS_TRACE_DEBUG(EID(REBUILD_ARRAY_DEBUG_MSG), "Rebuild triggered, targetSize:{}", targets.size());
    pthread_rwlock_wrlock(&stateLock);
    for (auto target : targets)
    {
        if (target->GetState() != ArrayDeviceState::FAULT || target->GetUblock() != nullptr)
        {
            POS_TRACE_DEBUG(EID(TRIGGER_REBUILD_DEBUG),
                "Rebuild target device is not removed yet");
            pthread_rwlock_unlock(&stateLock);
            retry = true;
            return retry;
        }
    }

    if (state->SetRebuild() == false)
    {
        POS_TRACE_WARN(EID(TRIGGER_REBUILD_DEBUG),
            "Failed to trigger rebuild. Current array state is not rebuildable");
        pthread_rwlock_unlock(&stateLock);
        return retry;
    }

    for (auto it = targets.begin(); it != targets.end(); )
    {
        ArrayDevice* src = nullptr;
        int ret = devMgr_->ReplaceWithSpare(dynamic_cast<ArrayDevice*>(*it), src);
        if (ret == 0)
        {
            delete src;
            (*it)->SetState(ArrayDeviceState::REBUILD);
            ++it;
        }
        else
        {
            targets.erase(it);
        }
    }

    if (targets.size() == 0)
    {
        state->SetRebuildDone(false);
        state->SetDegraded();
        POS_TRACE_WARN(EID(TRIGGER_REBUILD_DEBUG),
            "Failed to trigger rebuild. spare device is not available");
        pthread_rwlock_unlock(&stateLock);
        return retry;
    }

    _Flush();
    pthread_rwlock_unlock(&stateLock);

    DoRebuildAsync(targets, vector<IArrayDevice*>(), RebuildTypeEnum::BASIC);
    return retry;
}

bool
Array::ResumeRebuild(vector<IArrayDevice*> targets)
{
    POS_TRACE_DEBUG(EID(TRIGGER_REBUILD_DEBUG), "Resume rebuild invoked, targetCnt:{}", targets.size());
    pthread_rwlock_wrlock(&stateLock);
    bool retry = false;
    if (state->SetRebuild() == false)
    {
        POS_TRACE_WARN(EID(TRIGGER_REBUILD_DEBUG),
            "Failed to resume rebuild. Array({})'s state is not rebuildable", name_);
        pthread_rwlock_unlock(&stateLock);
        return retry;
    }
    pthread_rwlock_unlock(&stateLock);

    DoRebuildAsync(targets, vector<IArrayDevice*>(), RebuildTypeEnum::BASIC);
    return retry;
}

void
Array::DoRebuildAsync(vector<IArrayDevice*> dst, vector<IArrayDevice*> src, RebuildTypeEnum rt)
{
    IArrayRebuilder* arrRebuilder = rebuilder;
    string arrName = name_;
    uint32_t arrId = index_;
    RebuildComplete cb = bind(&Array::_RebuildDone, this, dst, src, placeholders::_1);
    list<RebuildTarget*> tasks = svc->GetRebuildTargets();

    if (rt == RebuildTypeEnum::BASIC)
    {
        POS_TRACE_INFO(EID(REBUILD_REQUEST), "type:BASIC, dstCnt:{}, srcCnt:{}", dst.size(), src.size());
        thread t([arrRebuilder, arrName, arrId, dst, cb, tasks]()
        {
            list<RebuildTarget*> targets = tasks;
            arrRebuilder->Rebuild(arrName, arrId, dst, cb, targets);
        });
        t.detach();
    }
    else
    {
        POS_TRACE_INFO(EID(REBUILD_REQUEST), "type:QUICK, dstCnt:{}, srcCnt:{}", dst.size(), src.size());
        vector<pair<IArrayDevice*, IArrayDevice*>> quickPair;
        assert(dst.size() == src.size());
        uint32_t idx = 0;
        for (IArrayDevice* dev : dst)
        {
            quickPair.emplace_back(make_pair(src.at(idx), dev));
            idx++;
        }

        thread t([arrRebuilder, arrName, arrId, quickPair, cb, tasks]()
        {
            list<RebuildTarget*> targets = tasks;
            arrRebuilder->QuickRebuild(arrName, arrId, quickPair, cb, targets);
        });
        t.detach();
    }
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
                ArrayService::Instance()->Setter()->IncludeDevicesToLocker(devMgr_->GetDataDevices());
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
        ArrayService::Instance()->Setter()->ExcludeDevicesFromLocker(devMgr_->GetDataDevices());
    }
}

void
Array::SetTargetAddress(string targetAddress)
{
    this->targetAddress = targetAddress;
}

string
Array::GetTargetAddress()
{
    return targetAddress;
}

} // namespace pos

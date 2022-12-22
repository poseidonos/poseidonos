/*
 *   BSD LICENSE
 *   Copyright (c) 2022 Samsung Electronics Corporation
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
#include "posreplicator_manager.h"

#include "spdk/pos.h"
#include "src/event_scheduler/callback.h"
#include "src/include/pos_event_id.h"
#include "src/io/frontend_io/aio.h"
#include "src/io/frontend_io/aio_submission_adapter.h"
#include "src/io/frontend_io/unvmf_io_handler.h"
#include "src/logger/logger.h"
#include "src/master_context/config_manager.h"
#include "src/pos_replicator/grpc_publisher.h"
#include "src/pos_replicator/grpc_subscriber.h"
#include "src/pos_replicator/pos_replicator_io_completion.h"
#include "src/pos_replicator/posreplicator_status.h"
#include "src/pos_replicator/replicator_volume_subscriber.h"
#include "src/qos/qos_manager.h"
#include "src/sys_event/volume_event.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/telemetry/telemetry_client/telemetry_client.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"
#include "src/volume/i_volume_info_manager.h"
#include "src/volume/volume_base.h"

namespace pos
{
PosReplicatorManager::PosReplicatorManager(void)
: PosReplicatorManager(new AIO(), new TelemetryPublisher("pos_replicator"))
{
}

PosReplicatorManager::PosReplicatorManager(AIO* aio, TelemetryPublisher* telemetryPublisher)
: aio(aio),
  telemetryPublisher(telemetryPublisher),
  volumeSubscriberCnt(0),
  isEnabled(false)
{
    TelemetryClientSingleton::Instance()->RegisterPublisher(telemetryPublisher);
    for (int i = 0; i < ArrayMgmtPolicy::MAX_ARRAY_CNT; i++)
    {
        items[i] = nullptr;
    }

    arrayConvertTable.clear();
    POS_TRACE_INFO(EID(HA_DEBUG_MSG), "PosReplicatorManager has been constructed");
}

PosReplicatorManager::~PosReplicatorManager(void)
{
    if (telemetryPublisher != nullptr)
    {
        TelemetryClientSingleton::Instance()->DeregisterPublisher(telemetryPublisher->GetName());
        delete telemetryPublisher;
        telemetryPublisher = nullptr;
    }

    POS_TRACE_INFO(EID(HA_DEBUG_MSG), "PosReplicatorManager has been destructed");
}

void
PosReplicatorManager::Init(GrpcPublisher* publisher, GrpcSubscriber* subscriber, ConfigManager* configManager)
{
    int ret = configManager->GetValue("replicator", "enable", &isEnabled, CONFIG_TYPE_BOOL);
    if (ret == EID(SUCCESS))
    {
        replicatorStatus.Set(ReplicatorStatus::VOLUMECOPY_None);
        grpcPublisher = publisher;
        grpcSubscriber = subscriber;
        isEnabled = true;
        POS_TRACE_INFO(EID(HA_DEBUG_MSG), "PosReplicatorManager has been initialized");
    }
    else
    {
        POS_TRACE_WARN(EID(HA_DEBUG_MSG), "POS Replicator is disabled. Skip initializing ReplicatorManager");
    }
}

void
PosReplicatorManager::Dispose(void)
{
    // stop replicator grpc server and client
    if (grpcPublisher != nullptr)
    {
        delete grpcPublisher;
        grpcPublisher = nullptr;
    }
    if (grpcSubscriber != nullptr)
    {
        delete grpcSubscriber;
        grpcSubscriber = nullptr;
    }

    POS_TRACE_INFO(EID(HA_DEBUG_MSG), "PosReplicatorManager has been disposed");
}

void
PosReplicatorManager::Clear(void)
{
    std::unique_lock<std::mutex> lock(listMutex);
    for (int i = 0; i < ArrayMgmtPolicy::MAX_ARRAY_CNT; i++)
    {
        if (items[i] != nullptr)
        {
            items[i] = nullptr;
        }
    }
    volumeSubscriberCnt = 0;
}

int
PosReplicatorManager::Register(int arrayId, ReplicatorVolumeSubscriber* volumeSubscriber)
{
    if (arrayId < 0)
    {
        POS_TRACE_ERROR(EID(HA_VOLUME_SUBSCRIBER_REGISTER_FAIL), "Fail to Register Replicator Volume Subscriber for Array {}",
            volumeSubscriber->GetArrayName());
        return EID(HA_INVALID_INPUT_ARGUMENT);
    }

    std::unique_lock<std::mutex> lock(listMutex);

    if (items[arrayId] != nullptr)
    {
        POS_TRACE_ERROR(EID(HA_VOLUME_SUBSCRIBER_REGISTER_FAIL), "Replicator Volume Subscriber for array {} already exists",
            volumeSubscriber->GetArrayName());
        return EID(HA_INVALID_INPUT_ARGUMENT);
    }

    items[arrayId] = volumeSubscriber;
    volumeSubscriberCnt++;
    POS_TRACE_DEBUG(EID(HA_VOLUME_SUBSCRIBER_REGISTER_FAIL), "Replicator Volume Subscriber for array {} is registered",
        volumeSubscriber->GetArrayName());

    arrayConvertTable.push_back(std::pair<int, string>(arrayId, volumeSubscriber->GetArrayName()));

    return EID(SUCCESS);
}

void
PosReplicatorManager::Unregister(int arrayId)
{
    if (arrayId < 0 || arrayId >= ArrayMgmtPolicy::MAX_ARRAY_CNT)
    {
        POS_TRACE_ERROR(EID(HA_VOLUME_SUBSCRIBER_UNREGISTER), "Replicator Volume Subscriber for array {} does not exist", arrayId);
        return;
    }

    std::unique_lock<std::mutex> lock(listMutex);
    ReplicatorVolumeSubscriber* target = items[arrayId];
    if (target == nullptr)
    {
        POS_TRACE_ERROR(EID(HA_VOLUME_SUBSCRIBER_UNREGISTER), "Replicator Volume Subscriber for array {} does not exist", arrayId);
        return;
    }

    items[arrayId] = nullptr;
    volumeSubscriberCnt--;

    for (auto it = arrayConvertTable.begin(); it != arrayConvertTable.end(); ++it)
    {
        if (it->first == arrayId)
        {
            arrayConvertTable.erase(it);
            break;
        }
    }

    POS_TRACE_DEBUG(EID(HA_VOLUME_SUBSCRIBER_UNREGISTER), "Replicator Volume Subscriber for array {} is unregistered", arrayId);
}

int
PosReplicatorManager::HAIOSubmission(IO_TYPE ioType, int arrayId, int volumeId, uint64_t rba, uint64_t numChunks, std::shared_ptr<char*> dataList, uint64_t lsn)
{
    VolumeIoSmartPtr volumeIo = _MakeVolumeIo(ioType, arrayId, volumeId, rba, numChunks);
    // TODO (cheolho.kang): Should add the error handling. if nullptr return

    if (ioType == IO_TYPE::WRITE)
    {
        _InsertChunkToBlock(volumeIo, dataList, numChunks);
    }
    _RequestVolumeIo(volumeIo, lsn);
    _PublishIopsMetrics(ioType, volumeIo);
    return EID(SUCCESS);
}

void
PosReplicatorManager::HAIOCompletion(uint64_t lsn, VolumeIoSmartPtr volumeIo, uint64_t originRba, uint64_t originNumChunks)
{
    switch (volumeIo->dir)
    {
        case UbioDir::Read:
            HAReadCompletion(lsn, volumeIo, originRba, originNumChunks);
            break;
        case UbioDir::Write:
            HAWriteCompletion(lsn, volumeIo, originRba, originNumChunks);
            break;
        default:
            std::string errorMsg = "Wrong IO direction (only read/write types are supported). input dir: " + std::to_string((uint32_t)volumeIo->dir);
            throw errorMsg.c_str();
            break;
    }
}

void
PosReplicatorManager::HAReadCompletion(uint64_t lsn, VolumeIoSmartPtr volumeIo, uint64_t originRba, uint64_t originNumChunks)
{
    std::string volumeName;
    std::string arrayName;
    int ret = _ConvertVolumeIdToName(volumeIo->GetVolumeId(), volumeIo->GetArrayId(), volumeName);
    if (ret == EID(SUCCESS))
    {
        ret = _ConvertArrayIdToName(volumeIo->GetArrayId(), arrayName);
        if (ret != EID(SUCCESS))
        {
            POS_TRACE_WARN(ret, "Not Found Request lsn : {}, array Idx : {}, volume Idx : {}",
                lsn, volumeIo->GetArrayId(), volumeIo->GetVolumeId());
            return;
        }
    }

    grpcPublisher->CompleteRead(arrayName, volumeName, originRba, originNumChunks, lsn, volumeIo->GetBuffer());
}

void
PosReplicatorManager::HAWriteCompletion(uint64_t lsn, VolumeIoSmartPtr volumeIo, uint64_t originRba, uint64_t originNumChunks)
{
    // TODO (cheolho.kang): Need to combine HAWriteCompletion() and HAReadCompletion()
    std::string volumeName;
    std::string arrayName;
    int ret = _ConvertVolumeIdToName(volumeIo->GetVolumeId(), volumeIo->GetArrayId(), volumeName);
    if (ret == EID(SUCCESS))
    {
        ret = _ConvertArrayIdToName(volumeIo->GetArrayId(), arrayName);
        if (ret != EID(SUCCESS))
        {
            // TODO (cheolho.kang): Need to handle the error case
            POS_TRACE_WARN(ret, "Not Found Request lsn : {}, array Idx : {}, volume Idx : {}",
                lsn, volumeIo->GetArrayId(), volumeIo->GetVolumeId());
            return;
        }
    }
    grpcPublisher->CompleteWrite(arrayName, volumeName, originRba, originNumChunks, lsn);
}

int
PosReplicatorManager::ConvertIdToName(int arrayId, int volumeId, std::string& arrayName, std::string& volumeName)
{
    int ret = EID(SUCCESS);

    ret = _ConvertArrayIdToName(arrayId, arrayName);
    if (ret != EID(SUCCESS))
    {
        POS_TRACE_WARN(EID(HA_DEBUG_MSG),
            "Cannot convert array id. ID: {}, error_code: {}", arrayId, ret);
    }

    ret = _ConvertVolumeIdToName(volumeId, arrayId, volumeName);
    if (ret != EID(SUCCESS))
    {
        POS_TRACE_WARN(EID(HA_DEBUG_MSG),
            "Cannot convert volume id. ID: {}, error_code: {}", volumeId, ret);
    }

    return ret;
}

int
PosReplicatorManager::ConvertNameToIdx(std::pair<std::string, int>& arraySet, std::pair<std::string, int>& volumeSet)
{
    arraySet.second = _ConvertArrayNameToId(arraySet.first);
    volumeSet.second = _ConvertVolumeNameToId(volumeSet.first, arraySet.second);

    int ret = EID(SUCCESS);

    if (arraySet.second == HA_INVALID_ARRAY_IDX)
    {
        POS_TRACE_WARN(EID(HA_DEBUG_MSG),
            "Cannot find array. Name: {}, ID: {}", arraySet.first, arraySet.second);
        ret = EID(HA_INVALID_INPUT_ARGUMENT);
    }
    else if (volumeSet.second == HA_INVALID_VOLUME_IDX)
    {
        POS_TRACE_WARN(EID(HA_DEBUG_MSG),
            "Cannot find volume. Name: {}, ID: {}", volumeSet.first, volumeSet.second);
        ret = EID(HA_INVALID_INPUT_ARGUMENT);
    }
    return ret;
}

int
PosReplicatorManager::HandleHostWrite(VolumeIoSmartPtr volumeIo)
{
    int result = EID(SUCCESS);
    if (isEnabled && volumeIo->dir == UbioDir::Write)
    {
        std::string arrayName;
        std::string volumeName;
        result = ConvertIdToName(volumeIo->GetArrayId(), volumeIo->GetVolumeId(), arrayName, volumeName);
        if (result != EID(SUCCESS))
        {
            return result;
        }

        ReplicatorStatus currentStatus = replicatorStatus.Get();
        if (currentStatus == ReplicatorStatus::VOLUMECOPY_PrimaryVolumeCopy)
        {
            result = grpcPublisher->PushDirtyLog(arrayName, volumeName, volumeIo->GetSectorRba(), ChangeByteToSector(volumeIo->GetSize()));
        }
        else if (currentStatus == ReplicatorStatus::VOLUMECOPY_PrimaryVolumeCopyWriteSuspend)
        {
            // Hold IO until state changes to VolumeCopy
            while (replicatorStatus.Get() != ReplicatorStatus::VOLUMECOPY_PrimaryVolumeCopy)
            {
                usleep(1);
            }
            result = grpcPublisher->PushDirtyLog(arrayName, volumeName, volumeIo->GetSectorRba(), ChangeByteToSector(volumeIo->GetSize()));
        }
        else if (currentStatus == ReplicatorStatus::VOLUMECOPY_PrimaryLiveReplication)
        {
            uint64_t lsn = 0;
            result = grpcPublisher->PushHostWrite(arrayName, volumeName, volumeIo->GetSectorRba(), ChangeByteToSector(volumeIo->GetSize()), volumeIo->GetBuffer(), lsn);
            _AddWaitPOSIoRequest(lsn, volumeIo);
            return result;
        }
    }

    if (true == QosManagerSingleton::Instance()->IsFeQosEnabled())
    {
        AioSubmissionAdapter aioSubmission;
        QosManagerSingleton::Instance()->HandlePosIoSubmission(&aioSubmission, volumeIo);
    }
    else
    {
        AIO aio;
        aio.SubmitAsyncIO(volumeIo);
    }
    return result;
}

int
PosReplicatorManager::CompleteUserIO(uint64_t lsn, int arrayId, int volumeId)
{
    VolumeIoSmartPtr volumeIo;
    auto itr = waitPosIoRequest[arrayId][volumeId].find(lsn);
    if (itr != waitPosIoRequest[arrayId][volumeId].end())
    {
        volumeIo = itr->second;
    }
    else
    {
        POS_TRACE_WARN(EID(HA_REQUESTED_NOT_FOUND), "Not Found Request lsn : {}, array Idx : {}, volume Idx : {}", lsn, arrayId, volumeId);
        return EID(HA_REQUESTED_NOT_FOUND);
    }
    waitPosIoRequest[arrayId][volumeId].erase(lsn);

    if (true == QosManagerSingleton::Instance()->IsFeQosEnabled())
    {
        AioSubmissionAdapter aioSubmission;
        QosManagerSingleton::Instance()->HandlePosIoSubmission(&aioSubmission, volumeIo);
    }
    else
    {
        AIO aio;
        aio.SubmitAsyncIO(volumeIo);
    }

    return EID(SUCCESS);
}

bool
PosReplicatorManager::IsEnabled(void)
{
    return isEnabled;
}

void
PosReplicatorManager::SetVolumeCopyStatus(ReplicatorStatus status)
{
    std::lock_guard<std::mutex> lock(statusLock);
    // TODO(cheolho.kang): add argument to speicific volume index
    // TODO(cheolho.kang): add status list each volume
    replicatorStatus.Set(status);

    POSMetricValue v;
    v.gauge = static_cast<uint64_t>(status);
    telemetryPublisher->PublishData(TEL150000_REPLICATION_STATUS, v, POSMetricTypes::MT_GAUGE);
}

ReplicatorStatus
PosReplicatorManager::GetVolumeCopyStatus(void)
{
    std::lock_guard<std::mutex> lock(statusLock);
    // TODO(cheolho.kang): add argument to speicific volume index
    // TODO(cheolho.kang): add status list each volume
    ReplicatorStatus result = replicatorStatus.Get();
    return result;
}

VolumeIoSmartPtr
PosReplicatorManager::_MakeVolumeIo(IO_TYPE ioType, int arrayId, int volumeId, uint64_t rba, uint64_t numChunks, std::shared_ptr<char*> dataList)
{
    pos_io posIo;
    posIo.ioType = ioType;
    posIo.array_id = arrayId;
    posIo.volume_id = volumeId;
    posIo.offset = ChangeSectorToByte(rba);
    posIo.length = ChangeSectorToByte(numChunks);
    posIo.iov = nullptr;

    return aio->CreatePosReplicatorVolumeIo(posIo, 0);
}

void
PosReplicatorManager::_RequestVolumeIo(VolumeIoSmartPtr volumeIo, uint64_t lsn)
{
    CallbackSmartPtr posReplicatorIOCompletion(new PosReplicatorIOCompletion(volumeIo, volumeIo->GetSectorRba(), ChangeByteToSector(volumeIo->GetSize()), lsn, volumeIo->GetCallback()));
    volumeIo->SetCallback(posReplicatorIOCompletion);

    aio->SubmitAsyncIO(volumeIo);
}

void
PosReplicatorManager::_InsertChunkToBlock(VolumeIoSmartPtr volumeIo, std::shared_ptr<char*> dataList, uint64_t numChunks)
{
    for (uint64_t index = 0; index < numChunks; index++)
    {
        char* bufferPtr = (char*)volumeIo.get()->GetBuffer() + index * ArrayConfig::SECTOR_SIZE_BYTE;
        char* targetPtr = dataList.get()[index];
        memcpy((void*)bufferPtr, (void*)(targetPtr), ArrayConfig::SECTOR_SIZE_BYTE);
    }
}

int
PosReplicatorManager::_ConvertArrayIdToName(int arrayId, std::string& arrayName)
{
    int ret = EID(HA_INVALID_INPUT_ARGUMENT);

    for (auto it = arrayConvertTable.begin(); it != arrayConvertTable.end(); ++it)
    {
        if (it->first == arrayId)
        {
            ret = EID(SUCCESS);
            arrayName = it->second;
            break;
        }
    }

    return ret;
}

int
PosReplicatorManager::_ConvertVolumeIdToName(int volumeId, int arrayId, std::string& volumeName)
{
    IVolumeInfoManager* volMgr =
        VolumeServiceSingleton::Instance()->GetVolumeManager(arrayId);

    int ret = EID(HA_INVALID_INPUT_ARGUMENT);

    if (volMgr == nullptr)
    {
        return ret;
    }

    ret = volMgr->GetVolumeName(volumeId, volumeName);

    return ret;
}

int
PosReplicatorManager::_ConvertArrayNameToId(std::string arrayName)
{
    int ret = HA_INVALID_ARRAY_IDX;

    for (auto it = arrayConvertTable.begin(); it != arrayConvertTable.end(); ++it)
    {
        if (it->second == arrayName)
        {
            ret = it->first;
            break;
        }
    }

    return ret;
}

int
PosReplicatorManager::_ConvertVolumeNameToId(std::string volumeName, int arrayId)
{
    IVolumeInfoManager* volMgr =
        VolumeServiceSingleton::Instance()->GetVolumeManager(arrayId);

    if (volMgr == nullptr)
    {
        return HA_INVALID_VOLUME_IDX;
    }

    int ret = volMgr->GetVolumeID(volumeName);
    return ret;
}

void
PosReplicatorManager::_AddWaitPOSIoRequest(uint64_t lsn, VolumeIoSmartPtr volumeIo)
{
    waitPosIoRequest[volumeIo->GetArrayId()][volumeIo->GetVolumeId()].insert({lsn, volumeIo});
}

void
PosReplicatorManager::_PublishIopsMetrics(IO_TYPE ioType, VolumeIoSmartPtr volumeIo)
{
    std::string volumeName;
    _ConvertVolumeIdToName(volumeIo->GetVolumeId(), volumeIo->GetArrayId(), volumeName);
    POSMetric metric(TEL50010_WRITE_IOPS_VOLUME, POSMetricTypes::MT_GAUGE);
    metric.AddLabel("array_id", to_string(volumeIo->GetArrayId()));
    metric.AddLabel("volume_name", volumeName);
    metric.AddLabel("type", ioType == IO_TYPE::READ ? "read" : "write");
    metric.SetGaugeValue(ChangeByteToSector(volumeIo->GetSize()));
    telemetryPublisher->PublishMetric(metric);
}
} // namespace pos

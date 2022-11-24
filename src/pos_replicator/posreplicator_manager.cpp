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

#include "pos_replicator_io_completion.h"
#include "src/include/pos_event_id.h"
#include "src/io/frontend_io/aio.h"
#include "src/logger/logger.h"

namespace pos
{
PosReplicatorManager::PosReplicatorManager(void)
: volumeSubscriberCnt(0)
{
    for (int i = 0; i < ArrayMgmtPolicy::MAX_ARRAY_CNT; i++)
    {
        items[i] = nullptr;
    }

    arrayConvertTable.clear();
    POS_TRACE_INFO(EID(HA_DEBUG_MSG), "PosReplicatorManager has been constructed");
}

PosReplicatorManager::~PosReplicatorManager(void)
{
    POS_TRACE_INFO(EID(HA_DEBUG_MSG), "PosReplicatorManager has been destructed");
}

void
PosReplicatorManager::Init(GrpcPublisher* publisher, GrpcSubscriber* subscriber)
{
    // grpc server or client start
    grpcPublisher = publisher;
    grpcSubscriber = subscriber;

    POS_TRACE_INFO(EID(HA_DEBUG_MSG), "PosReplicatorManager has been initialized");
}

void
PosReplicatorManager::Dispose(void)
{
    // stop grpc server or client
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
PosReplicatorManager::NotifyNewUserIORequest(pos_io io)
{
    uint64_t lsn;
    char buf[4096]; // TODO(jeddy.choi): need to implement copy bytes between io and buf

    std::string arrayName;
    std::string volumeName;
    int ret;

    ret = ConvertVolumeIdtoVolumeName(io.volume_id, io.array_id, volumeName);
    if (ret != EID(SUCCESS))
    {
        POS_TRACE_WARN(ret, "Invalid Input. Volume_ID: {}", io.volume_id);
        return ret;
    }
    ret = ConvertArrayIdtoArrayName(io.array_id, arrayName);
    if (ret != EID(SUCCESS))
    {
        POS_TRACE_WARN(ret, "Invalid Input. Array_ID: {}", io.array_id);
        return ret;
    }

    ret = grpcPublisher->PushHostWrite(io.offset, io.length,
        arrayName, volumeName, buf, lsn);

    if (ret != EID(SUCCESS))
    {
        POS_TRACE_WARN(ret, "Fail PushHostWrite");
        return ret;
    }
    _AddWaitPOSIoRequest(lsn, io);

    return EID(SUCCESS);
}

int
PosReplicatorManager::CompleteUserIO(uint64_t lsn, int arrayId, int volumeId)
{
    VolumeIoSmartPtr volumeIo;
    auto itr = donePosIoRequest[arrayId][volumeId].find(lsn);
    if (itr != donePosIoRequest[arrayId][volumeId].end())
    {
        volumeIo = itr->second;
    }
    else
    {
        POS_TRACE_WARN(EID(HA_REQUESTED_NOT_FOUND), "Not Found Request lsn : {}, array Idx : {}, volume Idx : {}",
            lsn, arrayId, volumeId);
        return EID(HA_REQUESTED_NOT_FOUND);
    }

    volumeIo->GetCallback()->Execute();

    donePosIoRequest[arrayId][volumeId].erase(lsn);

    return EID(SUCCESS);
}

int
PosReplicatorManager::UserVolumeWriteSubmission(uint64_t lsn, int arrayId, int volumeId)
{
    pos_io userRequest;

    auto itr = waitPosIoRequest[arrayId][volumeId].find(lsn);
    if (itr != waitPosIoRequest[arrayId][volumeId].end())
    {
        userRequest = itr->second;
    }
    else
    {
        return EID(HA_REQUESTED_NOT_FOUND);
    }

    // TODO (cheolho.kang): Check funtionality later
    // _MakeIoRequest(GrpcCallbackType::WaitGrpc, &userRequest, lsn);

    waitPosIoRequest[arrayId][volumeId].erase(lsn);

    return EID(SUCCESS);
}

int
PosReplicatorManager::HAIOSubmission(IO_TYPE ioType, int arrayId, int volumeId, uint64_t rba, uint64_t numBlocks, std::shared_ptr<char*> dataList)
{
    VolumeIoSmartPtr volumeIo = _MakeVolumeIo(ioType, arrayId, volumeId, rba, numBlocks);
    if (ioType == IO_TYPE::WRITE)
    {
        _InsertChunkToBlock(volumeIo, dataList, numBlocks);
    }
    _RequestVolumeIo(GrpcCallbackType::GrpcReply, volumeIo, REPLICATOR_INVALID_LSN);
    return EID(SUCCESS);
}

VolumeIoSmartPtr
PosReplicatorManager::_MakeVolumeIo(IO_TYPE ioType, int arrayId, int volumeId, uint64_t rba, uint64_t numBlocks, std::shared_ptr<char*> dataList)
{
    AIO aio;
    pos_io posIo;

    posIo.ioType = ioType;
    posIo.array_id = arrayId;
    posIo.volume_id = volumeId;
    posIo.offset = ChangeSectorToByte(rba);
    posIo.length = ChangeSectorToByte(numBlocks);
    posIo.iov = nullptr;

    return aio.CreatePosReplicatorVolumeIo(posIo, REPLICATOR_INVALID_LSN);
}

void
PosReplicatorManager::HAIOCompletion(uint64_t lsn, VolumeIoSmartPtr volumeIo)
{
    switch (volumeIo->dir)
    {
        case UbioDir::Read:
            HAReadCompletion(lsn, volumeIo);
            break;
        case UbioDir::Write:
            HAWriteCompletion(lsn, volumeIo);
            break;
        default:
            std::string errorMsg = "Wrong IO direction (only read/write types are supported). input dir: " + std::to_string((uint32_t)volumeIo->dir);
            throw errorMsg.c_str();
            break;
    }
}

void
PosReplicatorManager::HAWriteCompletion(uint64_t lsn, VolumeIoSmartPtr volumeIo)
{
    // TODO (cheolho.kang): Need to combine HAWriteCompletion() and HAReadCompletion()
    std::string volumeName;
    std::string arrayName;
    int ret = ConvertVolumeIdtoVolumeName(volumeIo->GetVolumeId(), volumeIo->GetArrayId(), volumeName);
    if (ret == EID(SUCCESS))
    {
        ret = ConvertArrayIdtoArrayName(volumeIo->GetArrayId(), arrayName);
        if (ret != EID(SUCCESS))
        {
            // TODO (cheolho.kang): Need to handle the error case
            POS_TRACE_WARN(ret, "Not Found Request lsn : {}, array Idx : {}, volume Idx : {}",
                lsn, volumeIo->GetArrayId(), volumeIo->GetVolumeId());
            return;
        }
    }

    grpcPublisher->CompleteWrite(lsn, volumeName, arrayName);
}

void
PosReplicatorManager::HAReadCompletion(uint64_t lsn, VolumeIoSmartPtr volumeIo)
{
    std::string volumeName;
    std::string arrayName;
    int ret = ConvertVolumeIdtoVolumeName(volumeIo->GetVolumeId(), volumeIo->GetArrayId(), volumeName);
    if (ret == EID(SUCCESS))
    {
        ret = ConvertArrayIdtoArrayName(volumeIo->GetArrayId(), arrayName);
        if (ret != EID(SUCCESS))
        {
            POS_TRACE_WARN(ret, "Not Found Request lsn : {}, array Idx : {}, volume Idx : {}",
                lsn, volumeIo->GetArrayId(), volumeIo->GetVolumeId());
            return;
        }
    }
    uint64_t numBlocks = ChangeByteToSector(volumeIo->GetSize());

    grpcPublisher->CompleteRead(arrayName, volumeName, volumeIo->GetSectorRba(), numBlocks, lsn, volumeIo->GetBuffer());
}

int
PosReplicatorManager::ConvertArrayIdtoArrayName(int arrayId, std::string& arrayName)
{
    int ret = EID(HA_INVALID_INPUT_ARGUMENT);

    // Array Idx <-> Array Name
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
PosReplicatorManager::ConvertVolumeIdtoVolumeName(int volumeId, int arrayId, std::string& volumeName)
{
    // Volume Idx <-> Volume Name
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
PosReplicatorManager::ConvertArrayNametoArrayId(std::string arrayName)
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
PosReplicatorManager::ConvertVolumeNametoVolumeId(std::string volumeName, int arrayId)
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
PosReplicatorManager::_AddWaitPOSIoRequest(uint64_t lsn, pos_io io)
{
    waitPosIoRequest[io.array_id][io.volume_id].insert({lsn, io});
}

void
PosReplicatorManager::AddDonePOSIoRequest(uint64_t lsn, VolumeIoSmartPtr volumeIo)
{
    donePosIoRequest[volumeIo->GetArrayId()][volumeIo->GetVolumeId()].insert({lsn, volumeIo});
}

void
PosReplicatorManager::_RequestVolumeIo(GrpcCallbackType callbackType, VolumeIoSmartPtr volumeIo, uint64_t lsn)
{
    AIO aio;
    POS_TRACE_DEBUG(EID(HA_DEBUG_MSG),
        "Io Submmit from replicator, rba: {}", volumeIo->GetSectorRba());

    CallbackSmartPtr posReplicatorIOCompletion(new PosReplicatorIOCompletion(callbackType, volumeIo, lsn, volumeIo->GetCallback()));

    volumeIo->SetCallback(posReplicatorIOCompletion);

    aio.SubmitAsyncIO(volumeIo);
}

int
PosReplicatorManager::ConvertNametoIdx(std::pair<std::string, int>& arraySet, std::pair<std::string, int>& volumeSet)
{
    arraySet.second = ConvertArrayNametoArrayId(arraySet.first);
    volumeSet.second = ConvertVolumeNametoVolumeId(volumeSet.first, arraySet.second);

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

void
PosReplicatorManager::_InsertChunkToBlock(VolumeIoSmartPtr volumeIo, std::shared_ptr<char*> dataList, uint64_t numBlocks)
{
    for (uint64_t index = 0; index < numBlocks; index++)
    {
        char* bufferPtr = (char*)volumeIo.get()->GetBuffer() + index * ArrayConfig::SECTOR_SIZE_BYTE;
        char* targetPtr = dataList.get()[index];
        memcpy((void*)bufferPtr, (void*)(targetPtr), ArrayConfig::SECTOR_SIZE_BYTE);
    }
}
} // namespace pos

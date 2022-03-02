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

#include "src/io/frontend_io/aio.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{

PosReplicatorManager::PosReplicatorManager(void)
: volumeSubscriberCnt(0)
{

    arrayConvertTable.clear();
}

PosReplicatorManager::~PosReplicatorManager(void)
{
}

void
PosReplicatorManager::Init(void)
{
    // grpc server or client start
    grpcPublisher = new GrpcPublisher(nullptr);
    grpcSubscriber = new GrpcSubscriber();
    
}

void
PosReplicatorManager::Dispose(void)
{
    // stop grpc server or client
    delete grpcPublisher;
    delete grpcSubscriber;
}

int
PosReplicatorManager::NotifyNewUserIORequest(pos_io io)
{/**/
    // user data가 새로 들어오면 본 함수를 통해 HA에 write request를 전달
    // DB가 생성한 lsn을 HA를 통해 전달받아 해당 값과 POS_IO를 통해 _AddWaitPOSIoRequest 를 이용하여 POS_IO를 저장

    uint64_t lsn;
    char buf[4096]; // user IO Data를 복사해서 전달해야 함 같은 buffer에 동시접근이 발생할 경우 문제가 발생할 수 있음

    std::string arrayName;
    std::string volumeName;
    int ret;

    ret = ConvertVolumeIdtoVolumeName(io.volume_id, io.array_id, arrayName);
    if (ret != EID(SUCCESS))
    {
        return ret;
    }
    ret = ConvertArrayIdtoArrayName(io.array_id, volumeName);
    if (ret != EID(SUCCESS))
    {
        return ret;
    }

    ret = grpcPublisher->PushHostWrite(io.offset, io.length,
        arrayName, volumeName, buf, lsn);

    if (ret != EID(SUCCESS))
    {
        return ret;
    }
    _AddWaitPOSIoRequest(lsn, io);

    return EID(SUCCESS);
}

int
PosReplicatorManager::CompelteUserIO(uint64_t lsn, int arrayId, int volumeId)
{
    VolumeIoSmartPtr volumeIo;
    auto itr = donePosIoRequest[arrayId][volumeId].find(lsn);
    if (itr != donePosIoRequest[arrayId][volumeId].end())
    {
        volumeIo = itr->second;
    }
    else
    {
        return EID(HA_REQUESTED_NOT_FOUND);
    }

    volumeIo->GetCallback()->Execute();

    donePosIoRequest[arrayId][volumeId].erase(lsn);

    return EID(SUCCESS);
}

int
PosReplicatorManager::UserVolumeWriteSubmission(uint64_t lsn, int arrayId, int volumeId)
{/**/
    // User Volume Write 동작을 요청 AIO를 생성하고, AIO를 통해 수행된 Write Completion 동작이 Write Splitter로 돌아올 수 있도록 한다.
    // _MakeWriteRequest 사용
    // CompleteUserVolumeWrite(void)로 AIO 동작을 마친 callback이 올라올 수 있도록 한다.
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

    _MakeIoRequest(GrpcCallbackType::WaitGrpc, userRequest, lsn);

    waitPosIoRequest[arrayId][volumeId].erase(lsn);

    return EID(SUCCESS);
}


int
PosReplicatorManager::HAIOSubmission(IO_TYPE ioType, int arrayId, int volumeId, uint64_t rba, uint64_t num_blocks, void* data)
{/**/
    //IO_TYPE::WRITE
    //IO_TYPE::READ
    pos_io posIo;

    posIo = _MakePosIo(ioType, arrayId, volumeId, rba, num_blocks);
    // Journal Volume에의 Write 동작 Request
    // Completion이 CompleteJournalVolumeWrite()으로 돌아올 수 있도록 한다.
    _MakeIoRequest(GrpcCallbackType::GrpcReply, posIo, REPLICATOR_INVALID_LSN);

    return EID(SUCCESS);
}

pos_io
PosReplicatorManager::_MakePosIo(IO_TYPE ioType, int arrayId, int volumeId, uint64_t rba, uint64_t num_blocks)
{
    pos_io posIo;
    string array_name;
    ConvertArrayIdtoArrayName(arrayId, array_name);

    posIo.ioType = ioType;
    posIo.array_id = arrayId;
    strncpy(posIo.arrayName, array_name.c_str(), array_name.size());
    posIo.volume_id = volumeId;
    posIo.offset = ChangeByteToSector(rba);
    posIo.length = ChangeSectorToByte(num_blocks);

    return posIo;
}

void
PosReplicatorManager::HAWriteCompletion(uint64_t lsn, pos_io io)
{
    // Journal Volume의 Write 동작 Complete 
    // HA에 Write가 완료되었음을 알린다.
    std::string volumeName;
    int ret = ConvertVolumeIdtoVolumeName(io.volume_id, io.array_id, volumeName);

    if (ret != EID(SUCCESS))
    {
        // to do POS log
        return;
    }

    grpcPublisher->CompleteWrite(lsn, volumeName, io.arrayName);
}

void
PosReplicatorManager::HAReadCompletion(uint64_t lsn, pos_io io, VolumeIoSmartPtr volumeIo)
{
    // read 완료 및 HA에 해당 read에 대한 결과값을 전달
    // 4KB 단위로 Data를 전달하며 Unmap Data였을 경우 해당 Data가 Unmap이었음을 알린다.
    // UECC의 경우에는 어떻게 할 것인가? -> 복구에 실패한 data에 대한 처리는 어떻게?
    std::string volumeName;
    int ret = ConvertVolumeIdtoVolumeName(io.volume_id, io.array_id, volumeName);

    if (ret != EID(SUCCESS))
    {
        // to do POS log
        return;
    }

    grpcPublisher->CompleteRead(lsn, io.length, volumeName, io.arrayName, io.iov->iov_base);
}

void
PosReplicatorManager::HAIOCompletion(uint64_t lsn, pos_io io, VolumeIoSmartPtr volumeIo)
{
    switch (io.ioType)
    {
    case IO_TYPE::READ:
    {
        HAReadCompletion(lsn, io, volumeIo);
    }
    break;
    case IO_TYPE::WRITE:
    {
        HAWriteCompletion(lsn, io);
    }
    break;
    }
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
    // waitPosIoRequest 에 lns - POS_IO 를 이용하여 Hash에 저장
    pos_io* inputIo = new pos_io;

    memcpy(inputIo, &io, sizeof(pos_io));

    waitPosIoRequest[io.array_id][io.volume_id].insert({lsn, *inputIo});
}

void
PosReplicatorManager::AddDonePOSIoRequest(uint64_t lsn, VolumeIoSmartPtr volumeIo)
{
    // donePosIoRequest 에 lns - POS_IO 를 이용하여 Hash에 저장
    donePosIoRequest[volumeIo->GetArrayId()][volumeIo->GetVolumeId()].insert({lsn, volumeIo});
}

void
PosReplicatorManager::_MakeIoRequest(GrpcCallbackType callbackType, pos_io io, uint64_t lsn)
{
    // volume copy / user data write를 위한 POS_IO를 생성한다.
    //User Data의 경우 Done이 본 Write Splitter로 돌아올 수 있도록 callback을 변조해 둔다.
    AIO aio;
    VolumeIoSmartPtr volumeIo = aio.CreatePosReplicatorVolumeIo(io, lsn);

    CallbackSmartPtr posReplicatorIOCompletion(new PosReplicatorIOCompletion(callbackType, volumeIo, lsn, io, volumeIo->GetCallback()));
    volumeIo->SetCallback(posReplicatorIOCompletion);

    aio.SubmitAsyncIO(volumeIo);
}

int
PosReplicatorManager::ConvertNametoIdx(std::pair<std::string, int> arraySet, std::pair<std::string, int> volumeSet)
{
    arraySet.second = ConvertArrayNametoArrayId(arraySet.first);
    volumeSet.second = ConvertVolumeNametoVolumeId(volumeSet.first, arraySet.second);

    int ret = EID(SUCCESS);

    if ((arraySet.second == HA_INVALID_ARRAY_IDX) || (volumeSet.second == HA_INVALID_VOLUME_IDX))
    {
        ret = EID(HA_INVALID_INPUT_ARGUMENT);
    }

    return ret;
}

PosReplicatorIOCompletion::PosReplicatorIOCompletion(GrpcCallbackType callbackType, VolumeIoSmartPtr inputVolumeIo, uint64_t lsn_, pos_io& posIo, CallbackSmartPtr originCallback_)
: Callback(true, CallbackType_PosReplicatorIOCompletion),
  grpcCallbackType(callbackType),
  volumeIo(inputVolumeIo),
  lsn(lsn_),
  posIo(posIo),
  originCallback(originCallback_)
{
}

PosReplicatorIOCompletion::~PosReplicatorIOCompletion(void)
{
}

bool
PosReplicatorIOCompletion::_DoSpecificJob(void)
{
    if (grpcCallbackType == GrpcCallbackType::WaitGrpc)
    {
        volumeIo->SetCallback(originCallback);
        PosReplicatorManagerSingleton::Instance()->AddDonePOSIoRequest(lsn, volumeIo);
    }
    else
    {
        PosReplicatorManagerSingleton::Instance()->HAIOCompletion(lsn, posIo, volumeIo);
    }

    return false;
}

}

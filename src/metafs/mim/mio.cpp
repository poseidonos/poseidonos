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

#include "mio.h"

#include "meta_volume_manager.h"
#include "metafs_common.h"
#include "metafs_config.h"
#include "metafs_control_request.h"
#include "mpio.h"

namespace pos
{
const MetaIoOpcode Mio::ioOpcodeMap[] = {MetaIoOpcode::Write, MetaIoOpcode::Read};
std::atomic<uint64_t> Mio::idAllocate_{0};

Mio::Mio(MpioAllocator* mpioAllocator)
: originReq(nullptr),
  opCode(MetaIoOpcode::Max),
  fileDataChunkSize(0),
  startLpn(0),
  error(0, false),
  ioCQ(nullptr),
  mpioAllocator(nullptr),
  mergedRequestList(nullptr),
  metaStorage(nullptr),
  UNIQUE_ID(idAllocate_++)
{
    _InitStateHandler();

    mpioAsyncDoneCallback = AsEntryPointParam1(&Mio::_HandleMpioDone, this);
    _BindMpioAllocator(mpioAllocator);
}

Mio::~Mio(void)
{
}

MetaLpnType
Mio::_GetCalculateStartLpn(MetaFsIoRequest* ioReq)
{
    MetaLpnType start = 0;
    MetaLpnType offsetInLpn = ioReq->byteOffsetInFile / fileDataChunkSize;

    for (int i = 0; i < ioReq->extentsCount; ++i)
    {
        int64_t result = offsetInLpn - ioReq->extents[i].GetCount();
        if (result < 0)
        {
            start = ioReq->extents[i].GetStartLpn() + offsetInLpn;
            break;
        }
        offsetInLpn -= ioReq->extents[i].GetCount();
    }

    return start;
}

void
Mio::Setup(MetaFsIoRequest* ioReq, MetaLpnType baseLpn, MetaStorageSubsystem* metaStorage)
{
    assert(mpioAllocator != nullptr);
    assert(ioReq->extents != nullptr);
    assert(ioReq->extentsCount != 0);

    originReq = ioReq;
    this->metaStorage = metaStorage;

    fileDataChunkSize = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;
    opCode = ioOpcodeMap[static_cast<uint32_t>(originReq->reqType)];
    startLpn = _GetCalculateStartLpn(ioReq);

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[Mio ][SetupMio   ] type={}, req.tagId={}, fileOffset={}, baseLpn={}, startLpn={}",
        originReq->reqType, originReq->tagId, originReq->byteOffsetInFile, baseLpn, startLpn);
}

void
Mio::Reset(void)
{
    MetaAsyncRunnable<MetaAsyncCbCxt, MioState, MioStateExecuteEntry>::Init();
    fileDataChunkSize = 0;
    error = std::make_pair(0, false);

    if (nullptr != mergedRequestList)
    {
        ClearMergedRequestList();
    }

    if (originReq)
    {
        delete originReq;
        originReq = nullptr;
    }

    ResetTimestamp();
}

void
Mio::_InitStateHandler(void)
{
    RegisterStateHandler(MioState::Init,
        new MioStateExecuteEntry(MioState::Init, AsEntryPointParam1(&Mio::Init, this), MioState::Issued));
    RegisterStateHandler(MioState::Issued,
        new MioStateExecuteEntry(MioState::Issued, AsEntryPointParam1(&Mio::Issue, this), MioState::Complete));
    RegisterStateHandler(MioState::Complete,
        new MioStateExecuteEntry(MioState::Complete, AsEntryPointParam1(&Mio::Complete, this), MioState::Complete));
}

void
Mio::_BindMpioAllocator(MpioAllocator* mpioAllocator)
{
    assert(this->mpioAllocator == nullptr && mpioAllocator != nullptr);
    this->mpioAllocator = mpioAllocator;
}

void
Mio::SetMpioDoneNotifier(PartialMpioDoneCb& partialMpioDoneNotifier)
{
    this->partialMpioDoneNotifier = partialMpioDoneNotifier;
}

void
Mio::SetMpioDonePoller(MpioDonePollerCb& mpioDonePoller)
{
    this->mpioDonePoller = mpioDonePoller;
}

void
Mio::SetIoCQ(MetaFsIoMultilevelQ<Mio*, RequestPriority>* ioCQ)
{
    this->ioCQ = ioCQ;
}

void
Mio::_HandleMpioDone(void* data)
{
    Mpio* mpio = reinterpret_cast<Mpio*>(data);

    _FinalizeMpio(*mpio);

    SetNextState(MioState::Complete);
    ExecuteAsyncState();
}

bool
Mio::IsRead(void)
{
    return originReq->reqType == MetaIoRequestType::Read;
}

MpioType
Mio::_LookupMpioType(MetaIoRequestType type)
{
    switch (type)
    {
        case MetaIoRequestType::Read:
            return MpioType::Read;
        case MetaIoRequestType::Write:
            return MpioType::Write;
        default:
            assert(false);
    }
}

void
Mio::SetLocalAioCbCxt(MioAsyncDoneCb& callback)
{
    aioCbCxt.Init(this, callback);
    SetAsyncCbCxt(&aioCbCxt, false);
}

int
Mio::GetArrayId(void)
{
    return originReq->arrayId;
}

Mpio*
Mio::_AllocMpio(MpioIoInfo& mpioIoInfo, bool partialIO)
{
    MpioType mpioType = _LookupMpioType(originReq->reqType);
    MetaStorageType storageType = originReq->targetMediaType;
    Mpio* mpio = mpioAllocator->TryAlloc(mpioType, storageType, mpioIoInfo.metaLpn, partialIO, mpioIoInfo.arrayId);

    if (mpio == nullptr)
        return nullptr;

    mpio->StoreTimestamp(MpioTimestampStage::Allocate);
    mpio->Setup(mpioIoInfo, partialIO, false /*forceSyncIO*/, metaStorage);
    mpio->SetLocalAioCbCxt(mpioAsyncDoneCallback);

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[Mpio][Alloc      ] type={}, req.tagId={}, mpio_id={}, fileOffsetinChunk={}, fileOffset={}",
        mpioIoInfo.opcode, mpioIoInfo.tagId, mpioIoInfo.mpioId,
        mpioIoInfo.startByteOffset, originReq->byteOffsetInFile);

    return mpio;
}

void
Mio::SetMergedRequestList(std::vector<MetaFsIoRequest*>* list)
{
    assert(mergedRequestList == nullptr);
    mergedRequestList = list;
}

void
Mio::ClearMergedRequestList(void)
{
    for (auto it : *mergedRequestList)
        if (it != originReq)
            delete it;

    delete mergedRequestList;
    mergedRequestList = nullptr;
}

std::vector<MetaFsIoRequest*>*
Mio::GetMergedRequestList(void)
{
    return mergedRequestList;
}

void
Mio::_PrepareMpioInfo(MpioIoInfo& mpioIoInfo, MetaLpnType lpn,
    FileSizeType byteOffset, FileSizeType byteSize, FileBufType buf,
    MetaLpnType lpnCnt, uint32_t mpio_id)
{
    mpioIoInfo.opcode = static_cast<MetaIoOpcode>(originReq->reqType);
    mpioIoInfo.targetMediaType = originReq->targetMediaType;
    mpioIoInfo.targetFD = originReq->fd;
    mpioIoInfo.metaLpn = lpn;
    mpioIoInfo.startByteOffset = byteOffset;
    mpioIoInfo.byteSize = byteSize;
    mpioIoInfo.pageCnt = lpnCnt;
    mpioIoInfo.userBuf = buf;
    mpioIoInfo.tagId = originReq->tagId;
    mpioIoInfo.mpioId = mpio_id;
    mpioIoInfo.arrayId = originReq->arrayId;
    mpioIoInfo.signature = originReq->fileCtx->signature;
}

// FIXME: for better parallel execution, let's issue io request for each mpio as soon as mio builds mpio contexta
/* 
The host I/O issued as Mio data structure, and the Mio is consisted with several Mpio moudled by fileDataChunkSize(4032Byte) that the other 64Byte is page control info such as signature, version, and so on. 
*/
void
Mio::_BuildMpioMap(void)
{
    uint32_t curLpn, remainingBytes, byteOffsetInChunk, byteSize;
    FileBufType curUserBuf;

    remainingBytes = originReq->byteSize;
    curLpn = startLpn;
    byteOffsetInChunk = originReq->byteOffsetInFile % fileDataChunkSize;
    curUserBuf = originReq->buf;

    if (byteOffsetInChunk + originReq->byteSize < fileDataChunkSize)
    {
        byteSize = originReq->byteSize;
    }
    else
    {
        byteSize = fileDataChunkSize - byteOffsetInChunk;
    }

    MpioType ioType = _LookupMpioType(originReq->reqType);

    uint32_t mpio_cnt = 0;
    // issue Mpios
    do
    {
        // no more Mpio operations
        while (mpioAllocator->IsEmpty(ioType))
        {
            // Complete Process, MpioHandler::BottomhalfMioProcessing()
            mpioDonePoller();
        }

        // Build Mpio data structure
        MpioIoInfo mpioIoInfo;
        _PrepareMpioInfo(mpioIoInfo, curLpn, byteOffsetInChunk, byteSize, curUserBuf, 1 /* LpnCnt */, mpio_cnt++);

        // _AllocMpio() always returns valid mpio, because of mpioDonePoller() above.
        Mpio* mpio = _AllocMpio(mpioIoInfo, byteSize != fileDataChunkSize /* partialIO */);
        mpio->SetPartialDoneNotifier(partialMpioDoneNotifier);
        mpio->SetPriority(originReq->priority);

        if (MpioCacheState::FirstRead == mpio->GetCacheState())
        {
            // pass, new mpio
        }
        else if (MpioCacheState::Mergeable == mpio->GetCacheState())
        {
            // copy data for merged requests
            if (nullptr != mergedRequestList)
            {
                for (std::vector<MetaFsIoRequest*>::iterator it = mergedRequestList->begin(); it != mergedRequestList->end(); ++it)
                {
                    FileSizeType size = (*it)->byteSize;
                    FileBufType targetBuf = (uint8_t*)(mpio->GetMDPageDataBuf()) + ((*it)->byteOffsetInFile % fileDataChunkSize);
                    FileBufType originBuf = (*it)->buf;
                    memcpy(targetBuf, originBuf, size);
                }
            }
            // copy data for single request
            else
            {
                mpio->SetCacheState(MpioCacheState::MergeSingle);
            }
        }
        else if (MpioCacheState::MergeSingle == mpio->GetCacheState())
        {
            assert(0);
        }

        mpio->ExecuteAsyncState();

        curLpn++;
        curUserBuf = (void*)((uint8_t*)curUserBuf + byteSize);
        remainingBytes -= byteSize;
        byteOffsetInChunk = 0;

        if (remainingBytes >= fileDataChunkSize)
        {
            byteSize = fileDataChunkSize;
        }
        else
        {
            byteSize = remainingBytes;
        }
    } while (remainingBytes);
}

MetaLpnType
Mio::GetStartLpn(void)
{
    return startLpn;
}

bool
Mio::IsTargetStorageSSD(void)
{
    return originReq->targetMediaType == MetaStorageType::SSD;
}

void*
Mio::GetClientAioCbCxt(void)
{
    return originReq->aiocb;
}

bool
Mio::IsSyncIO(void)
{
    return originReq->ioMode == MetaIoMode::Sync;
}

bool
Mio::Init(MioState expNextState)
{
    StoreTimestamp(MioTimestampStage::Initialize);
    SetNextState(expNextState);
    return true;
}

bool
Mio::Issue(MioState expNextState)
{
    StoreTimestamp(MioTimestampStage::Issue);
    _BuildMpioMap();
    SetNextState(expNextState);

    return false; // not continue to execute. bottom half procedure will dispatch pending mpio
}

MfsError
Mio::GetError(void)
{
    return error;
}

void
Mio::_FinalizeMpio(Mpio& mpio)
{
    // FIXME: obtain last error code (Need to deal with another approach?)

    MfsError rc;
    rc = mpio.GetErrorStatus();
    if (rc.first != 0 || rc.second == true)
    {
        this->error = mpio.GetErrorStatus();
    }
}

bool
Mio::Complete(MioState expNextState)
{
    StoreTimestamp(MioTimestampStage::Complete);
    SetNextState(expNextState);

    StoreTimestamp(MioTimestampStage::Enqueue);
    ioCQ->Enqueue(this, originReq->priority);

    metaStorage = nullptr;

    return true;
}

void
Mio::NotifyCompletionToClient(void)
{
    if (error.first != 0 || error.second == true)
    {
        // originReq => cloneReqMsg
        // originalMsg => the msg from a user thread
        originReq->originalMsg->SetError(true);
    }

    originReq->originalMsg->NotifyIoCompletionToClient();
}
} // namespace pos

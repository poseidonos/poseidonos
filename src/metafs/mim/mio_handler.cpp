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

#include "mio_handler.h"

#include <string>
#include <utility>

#include "mfs_aiocb_cxt.h"
#include "mfs_async_runnable_template.h"
#include "mfs_log.h"
#include "mfs_mutex.h"
#include "mfs_mvm_top.h"
#include "mss.h"
#include "src/scheduler/event.h"
#include "src/scheduler/event_argument.h"

#if defined(IBOFOS_BACKEND_IO)
#include "mfs_aio_completer.h"
#endif

#define RANGE_OVERLAP_CHECK_EN 1

MioHandler::MioHandler(int threadId, int coreId, int coreCount)
: cpuStallCnt(0)
{
    std::string cqName("IoCQ-" + std::to_string(coreId));
    ioCQ.Init(cqName.c_str(), MAX_CONCURRENT_MIO_PROC_THRESHOLD);

    mpioPool = new MpioPool(MAX_CONCURRENT_MIO_PROC_THRESHOLD);
    mioPool = new MioPool(mpioPool, MAX_CONCURRENT_MIO_PROC_THRESHOLD);

    mioCompletionCallback = AsEntryPointParam1(&MioHandler::_HandleMioCompletion, this);

    this->bottomhalfHandler = nullptr;
    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        "mio handler constructed. threadId={}, coreId={}",
        threadId, coreId);

    for (int idx = 0; idx < (int)MetaStorageType::Max; idx++)
    {
        ioRangeOverlapChker[idx].Init(mvmTopMgr.GetMaxMetaLpn(static_cast<MetaVolumeType>(idx)));
    }
}

MioHandler::~MioHandler(void)
{
    delete mpioPool;
    delete mioPool;
    for (int idx = 0; idx < (int)MetaStorageType::Max; idx++)
    {
        ioRangeOverlapChker[idx].Reset();
    }
}

void
MioHandler::BindPartialMpioHandler(MpioHandler* ptMpioHandler)
{
    this->bottomhalfHandler = ptMpioHandler;
    this->bottomhalfHandler->BindMpioPool(mpioPool);

    partialMpioDoneNotifier = AsEntryPointParam1(&MpioHandler::EnqueuePartialMpio, bottomhalfHandler);
    mpioDonePoller = AsEntryPointNoParam(&MpioHandler::BottomhalfMioProcessing, bottomhalfHandler);
}

void
MioHandler::_HandleIoSQ(void)
{
    // if mio is not available, no new request can be serviced.
    if (mioPool->IsEmpty())
    {
        return;
    }

    MetaFsIoReqMsg* reqMsg = ioSQ.Dequeue();
    if (nullptr == reqMsg)
    {
        if (cpuStallCnt++ > 1000)
        {
            usleep(1);
            cpuStallCnt = 0;
        }
        return;
    }
    cpuStallCnt = 0;

#if RANGE_OVERLAP_CHECK_EN // range overlap enable
    if (_IsRangeOverlapConflicted(reqMsg))
    {
        _PushToRetry(reqMsg);
        return;
    }
#endif
    Mio* mio = DispatchMio(*reqMsg);
    if (nullptr == mio)
    {
        _PushToRetry(reqMsg);
        return;
    }

    _RegisterRangeLockInfo(reqMsg);
    ExecuteMio(*mio);
}

void
MioHandler::_HandleIoCQ(void)
{
    Mio* mio = ioCQ.Dequeue();
    if (mio)
    {
        while (mio->IsCompleted() != true)
        {
        }

#if RANGE_OVERLAP_CHECK_EN // range overlap enable
        _FreeLockContext(mio);
        _DiscoverIORangeOverlap(); // find other pending I/O
#endif
        if (true == mio->IsSyncIO())
        {
            mio->NotifyCompletionToClient();
        }
        _FinalizeMio(mio);
    }
}

void
MioHandler::_DiscoverIORangeOverlap(void)
{
    for (auto it = pendingIoRetryQ.begin(); it != pendingIoRetryQ.end();)
    {
        MetaFsIoReqMsg* pendingIoReq = it->second;

        if (!_IsRangeOverlapConflicted(pendingIoReq))
        {
            Mio* mio = DispatchMio(*pendingIoReq);
            if (nullptr == mio)
                return;

            pendingIoRetryQ.erase(it);

            MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
                "[Msg ][EraseRetryQ] type={}, req.tagId={}, fileOffset={}, Lpn={}, numPending={}",
                pendingIoReq->reqType, pendingIoReq->tagId, pendingIoReq->byteOffsetInFile,
                pendingIoReq->byteOffsetInFile / MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE,
                pendingIoRetryQ.size());

            _RegisterRangeLockInfo(pendingIoReq);
            ExecuteMio(*mio);
        }

        it = pendingIoRetryQ.upper_bound(pendingIoReq->baseMetaLpn);
    }
}

void
MioHandler::TophalfMioProcessing(void)
{
    _HandleIoSQ();
    _HandleIoCQ();
}

bool
MioHandler::EnqueueNewReq(MetaFsIoReqMsg& reqMsg)
{
    if (false == ioSQ.Enqueue(&reqMsg))
    {
        return false;
    }
    return true;
}

void
MioHandler::_FinalizeMio(Mio* mio)
{
    mioPool->Release(mio);
}

Mio*
MioHandler::_AllocNewMio(MetaFsIoReqMsg& reqMsg)
{
    Mio* mio = mioPool->Alloc();

    if (nullptr == mio)
        return nullptr;

    MetaLpnType fileBaseLpn;
    IBOF_EVENT_ID sc;
    sc = mvmTopMgr.GetFileBaseLpn(reqMsg.fd, fileBaseLpn);
    assert(sc == IBOF_EVENT_ID::SUCCESS);

    mio->Setup(&reqMsg, fileBaseLpn);

    if (false == mio->IsSyncIO())
    {
        mio->SetLocalAioCbCxt(mioCompletionCallback);
    }

    mio->SetMpioDoneNotifier(partialMpioDoneNotifier);
    mio->SetMpioDonePoller(mpioDonePoller);
    mio->SetIoCQ(&ioCQ);

    sc = mvmTopMgr.GetTargetMediaType(reqMsg.fd, reqMsg.targetMediaType);
    assert(sc == IBOF_EVENT_ID::SUCCESS);

    return mio;
}

Mio*
MioHandler::DispatchMio(MetaFsIoReqMsg& reqMsg)
{
    Mio* mio = _AllocNewMio(reqMsg);

    return mio;
}

void
MioHandler::_PushToRetry(MetaFsIoReqMsg* reqMsg)
{
    reqMsg->SetRetryFlag();
    pendingIoRetryQ.insert(pair<MetaLpnType, MetaFsIoReqMsg*>(reqMsg->baseMetaLpn, reqMsg));

    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[Msg ][_PushToRetry] type={}, req.tagId={}, ByteOffset={}, Lpn={}, numPending={}",
        reqMsg->reqType, reqMsg->tagId, reqMsg->byteOffsetInFile,
        reqMsg->byteOffsetInFile / MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE,
        pendingIoRetryQ.size());
}

bool
MioHandler::_IsRangeOverlapConflicted(MetaFsIoReqMsg* reqMsg)
{
    if (reqMsg->reqType == MetaIoReqTypeEnum::Read)
    {
        return false;
    }

    return ioRangeOverlapChker[(int)reqMsg->targetMediaType].IsRangeOverlapConflicted(reqMsg);
}

void
MioHandler::_RegisterRangeLockInfo(MetaFsIoReqMsg* reqMsg)
{
    ioRangeOverlapChker[(int)reqMsg->targetMediaType].PushReqToRangeLockMap(reqMsg);
}

void
MioHandler::_FreeLockContext(Mio* mio)
{
    int storage = mio->IsTargetStorageSSD() ? (int)MetaStorageType::SSD : (int)MetaStorageType::NVRAM;
    ioRangeOverlapChker[storage].FreeLockContext(mio->GetStartLpn(), mio->IsRead());
}

void
MioHandler::ExecuteMio(Mio& mio)
{
    mio.ExecuteAsyncState();
}

void
MioHandler::_HandleMioCompletion(void* data)
{
    // FIXME: will replace to handle below logic by mio completion event handler
    Mio* mio = reinterpret_cast<Mio*>(data);
    assert(mio->IsSyncIO() == false);

    MetaFsAioCbCxt* aiocb = reinterpret_cast<MetaFsAioCbCxt*>(mio->GetClientAioCbCxt());

    if (aiocb)
    {
        aiocb->SetErrorStatus(mio->GetError());

        _SendAioDoneEvent(aiocb);
    }
}

void
MioHandler::_SendAioDoneEvent(void* aiocb)
{
    // create event for callback by event worker
#if defined(IBOFOS_BACKEND_IO)
    MetaFsAioCompleter* event =
        new MetaFsAioCompleter(static_cast<MetaFsAioCbCxt*>(aiocb));
    ibofos::EventArgument::GetEventScheduler()->EnqueueEvent(event);
#else
    (reinterpret_cast<MetaFsAioCbCxt*>(aiocb))->InvokeCallback();
#endif
}

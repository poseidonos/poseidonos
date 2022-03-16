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

#include "metafs_io_scheduler.h"

#include <string>
#include <thread>

#include "src/metafs/include/metafs_aiocb_cxt.h"
#include "src/metafs/mim/scalable_meta_io_worker.h"
#include "src/metafs/mvm/meta_volume_manager.h"

namespace pos
{
MetaFsIoScheduler::MetaFsIoScheduler(const int threadId, const int coreId,
    const int totalCoreCount, const std::string& threadName,
    const cpu_set_t mioCoreSet, MetaFsConfigManager* config,
    TelemetryPublisher* tp)
: MetaFsIoHandlerBase(threadId, coreId, threadName),
  TOTAL_CORE_COUNT(totalCoreCount),
  MIO_CORE_COUNT(CPU_COUNT(&mioCoreSet)),
  MIO_CORE_SET(mioCoreSet),
  config_(config),
  tp_(tp),
  cpuStallCnt_(0)
{
}

MetaFsIoScheduler::~MetaFsIoScheduler(void)
{
    threadExit_ = true;
}

void
MetaFsIoScheduler::ExitThread(void)
{
    for (auto metaIoWorker : metaIoWorkerList_)
    {
        POS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
            "Exit MioHandler, " + metaIoWorker->GetLogString());

        metaIoWorker->ExitThread();
        delete metaIoWorker;
    }
    metaIoWorkerList_.clear();

    MetaFsIoHandlerBase::ExitThread();

    POS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
        "Exit MetaIoScheduler, " + GetLogString());
}

void
MetaFsIoScheduler::IssueRequest(MetaFsIoRequest* reqMsg)
{
    FileSizeType chunkSize = reqMsg->fileCtx->chunkSize;
    uint64_t byteOffset = 0;
    MetaLpnType fileBaseLpn = reqMsg->fileCtx->fileBaseLpn;
    MetaLpnType startLpn = fileBaseLpn + (reqMsg->byteOffsetInFile / chunkSize);
    MetaLpnType endLpn = fileBaseLpn + ((reqMsg->byteOffsetInFile + reqMsg->byteSize - 1) / chunkSize);

    // set the request count to process the callback count
    if (MetaIoMode::Async == reqMsg->ioMode)
    {
        ((MetaFsAioCbCxt*)reqMsg->aiocb)->SetCallbackCount(endLpn - startLpn + 1);
    }
    else
    {
        reqMsg->originalMsg->requestCount = endLpn - startLpn + 1;
    }

    for (MetaLpnType idx = startLpn; idx <= endLpn; idx++)
    {
        // reqMsg     : only for meta scheduler, not meta handler thread
        // cloneReqMsg: new copy, sent to meta handler thread by scheduler
        // reqMsg->originalMsg: from a user thread
        MetaFsIoRequest* cloneReqMsg = new MetaFsIoRequest();
        cloneReqMsg->CopyUserReqMsg(*reqMsg);

        // 1st
        if (idx == startLpn)
        {
            cloneReqMsg->buf = reqMsg->buf;
            cloneReqMsg->byteOffsetInFile = reqMsg->byteOffsetInFile;
            if (chunkSize < (reqMsg->byteSize + (reqMsg->byteOffsetInFile % chunkSize)))
            {
                cloneReqMsg->byteSize = chunkSize - (reqMsg->byteOffsetInFile % chunkSize);
            }
            else
            {
                cloneReqMsg->byteSize = reqMsg->byteSize;
            }
        }
        // last
        else if (idx == endLpn)
        {
            cloneReqMsg->buf = (FileBufType)((uint64_t)reqMsg->buf + byteOffset);
            cloneReqMsg->byteOffsetInFile = reqMsg->byteOffsetInFile + byteOffset;
            cloneReqMsg->byteSize = reqMsg->byteSize - byteOffset;
        }
        else
        {
            cloneReqMsg->buf = (FileBufType)((uint64_t)reqMsg->buf + byteOffset);
            cloneReqMsg->byteOffsetInFile = reqMsg->byteOffsetInFile + byteOffset;
            cloneReqMsg->byteSize = chunkSize;
        }

        byteOffset += cloneReqMsg->byteSize;
        cloneReqMsg->baseMetaLpn = idx;

        metaIoWorkerList_[idx % MIO_CORE_COUNT]->EnqueueNewReq(cloneReqMsg);
    }

    // delete msg instance, this instance was only for meta scheduler
    delete reqMsg;
}

void
MetaFsIoScheduler::EnqueueNewReq(MetaFsIoRequest* reqMsg)
{
    ioMultiQ.Enqueue(reqMsg, reqMsg->priority);
}

bool
MetaFsIoScheduler::AddArrayInfo(const int arrayId)
{
    bool result = true;

    for (auto metaIoWorker : metaIoWorkerList_)
    {
        if (!metaIoWorker->AddArrayInfo(arrayId))
        {
            result = false;
            break;
        }
    }

    return result;
}

bool
MetaFsIoScheduler::RemoveArrayInfo(const int arrayId)
{
    bool result = true;

    for (auto metaIoWorker : metaIoWorkerList_)
    {
        if (!metaIoWorker->RemoveArrayInfo(arrayId))
        {
            result = false;
            break;
        }
    }

    return result;
}

void
MetaFsIoScheduler::StartThread(void)
{
    th_ = new std::thread(AsEntryPointNoParam(&MetaFsIoScheduler::Execute, this));

    POS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
        "Start MetaIoScheduler, " + GetLogString());

    _CreateMioThread();
}

void
MetaFsIoScheduler::_CreateMioThread(void)
{
    const std::string fileName = "MioHandler";
    uint32_t handlerId = 0;
    int availableMioCoreCnt = MIO_CORE_COUNT;
    for (uint32_t coreId = 0; coreId < TOTAL_CORE_COUNT; ++coreId)
    {
        if (CPU_ISSET(coreId, &MIO_CORE_SET))
        {
            ScalableMetaIoWorker* mioHandler =
                new ScalableMetaIoWorker(handlerId++, coreId, fileName, config_, tp_);
            mioHandler->StartThread();
            metaIoWorkerList_.emplace_back(mioHandler);
            availableMioCoreCnt--;

            POS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
                "Create MioHandler, " + mioHandler->GetLogString());

            if (availableMioCoreCnt == 0)
            {
                break;
            }
        }
    }

    if (availableMioCoreCnt)
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::MFS_ERROR_MESSAGE,
            "The Count of created MioHandler: {}, expected count: {}",
            MIO_CORE_COUNT - availableMioCoreCnt, MIO_CORE_COUNT);
    }
}

void
MetaFsIoScheduler::Execute(void)
{
    PrepareThread();

    while (!threadExit_)
    {
        MetaFsIoRequest* reqMsg = _FetchPendingNewReq();

        if (!reqMsg)
        {
            if (cpuStallCnt_++ > MAX_CPU_STALL_COUNT)
            {
                usleep(1);
                cpuStallCnt_ = 0;
            }
            continue;
        }
        cpuStallCnt_ = 0;

        IssueRequest(reqMsg);
    }
}

MetaFsIoRequest*
MetaFsIoScheduler::_FetchPendingNewReq(void)
{
    return ioMultiQ.Dequeue();
}
} // namespace pos

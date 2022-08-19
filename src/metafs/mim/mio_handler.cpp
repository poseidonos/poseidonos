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

#include "mio_handler.h"

#include <string>
#include <utility>
#include <vector>

#include "meta_volume_manager.h"
#include "metafs_aiocb_cxt.h"
#include "metafs_log.h"
#include "metafs_mutex.h"
#include "mfs_async_runnable_template.h"
#include "src/event_scheduler/event.h"
#include "src/metafs/config/metafs_config_manager.h"
#include "src/metafs/include/metafs_service.h"
#include "src/metafs/storage/mss.h"
#include "src/telemetry/telemetry_client/telemetry_client.h"

namespace pos
{
MioHandler::MioHandler(const int threadId, const int coreId,
    MetaFsConfigManager* configManager, TelemetryPublisher* tp)
: ioSQ(nullptr),
  ioCQ(nullptr),
  MIO_POOL_SIZE(configManager->GetMioPoolCapacity()),
  MPIO_POOL_SIZE(configManager->GetMpioPoolCapacity()),
  WRITE_CACHE_CAPACITY(configManager->GetWriteMpioCacheCapacity()),
  coreId(coreId),
  telemetryPublisher(tp),
  sampledTimeSpentProcessingAllStages(0),
  sampledTimeSpentFromIssueToComplete(0),
  totalProcessedMioCount(0),
  sampledProcessedMioCount(0),
  metaFsTimeInterval(configManager->GetTimeIntervalInMillisecondsForMetric()),
  skipCount(0),
  SAMPLING_SKIP_COUNT(configManager->GetSamplingSkipCount()),
  issueCountByStorage(),
  issueCountByFileType()
{
    ioCQ = new MetaFsIoQ<Mio*>();
    ioSQ = new MetaFsIoWrrQ<MetaFsIoRequest*, MetaFileType>(configManager->GetWrrWeight());

    mpioAllocator = new MpioAllocator(configManager);
    _CreateMioPool();

    mioCompletionCallback = AsEntryPointParam1(&MioHandler::_HandleMioCompletion, this);

    this->bottomhalfHandler = nullptr;
    POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
        "Mio handler constructed. threadId: {}, coreId: {}, mio pool size: {}",
        threadId, coreId, MIO_POOL_SIZE);
}

MioHandler::MioHandler(const int threadId, const int coreId,
    MetaFsConfigManager* configManager,
    MetaFsIoWrrQ<MetaFsIoRequest*, MetaFileType>* ioSQ,
    MetaFsIoQ<Mio*>* ioCQ, MpioAllocator* mpioAllocator,
    MetaFsPool<Mio*>* mioPool, TelemetryPublisher* tp)
: ioSQ(ioSQ),
  ioCQ(ioCQ),
  mioPool(mioPool),
  mpioAllocator(mpioAllocator),
  MIO_POOL_SIZE(configManager->GetMioPoolCapacity()),
  MPIO_POOL_SIZE(configManager->GetMpioPoolCapacity()),
  WRITE_CACHE_CAPACITY(configManager->GetWriteMpioCacheCapacity()),
  coreId(coreId),
  telemetryPublisher(tp),
  sampledTimeSpentProcessingAllStages(0),
  sampledTimeSpentFromIssueToComplete(0),
  totalProcessedMioCount(0),
  sampledProcessedMioCount(0),
  metaFsTimeInterval(configManager->GetTimeIntervalInMillisecondsForMetric()),
  skipCount(0),
  SAMPLING_SKIP_COUNT(configManager->GetSamplingSkipCount()),
  issueCountByStorage(),
  issueCountByFileType()
{
    mioCompletionCallback = AsEntryPointParam1(&MioHandler::_HandleMioCompletion, this);

    this->bottomhalfHandler = nullptr;
}

MioHandler::~MioHandler(void)
{
    if (mpioAllocator)
    {
        delete mpioAllocator;
        mpioAllocator = nullptr;
    }

    if (mioPool)
    {
        delete mioPool;
        mioPool = nullptr;
    }

    for (uint32_t index = 0; index < MetaFsConfig::MAX_ARRAY_CNT; index++)
    {
        for (uint32_t storage = 0; storage < NUM_STORAGE; storage++)
        {
            if (ioRangeOverlapChker[index][storage])
            {
                delete ioRangeOverlapChker[index][storage];
                ioRangeOverlapChker[index][storage] = nullptr;
            }
        }
    }

    if (ioCQ)
    {
        delete ioCQ;
        ioCQ = nullptr;
    }

    if (ioSQ)
    {
        delete ioSQ;
        ioSQ = nullptr;
    }
}

void
MioHandler::_CreateMioPool(void)
{
    mioPool = new MetaFsPool<Mio*>(MIO_POOL_SIZE);
    for (size_t i = 0; i < MIO_POOL_SIZE; ++i)
    {
        mioPool->AddToPool(new Mio(mpioAllocator));
    }
}

void
MioHandler::BindPartialMpioHandler(MpioHandler* ptMpioHandler)
{
    this->bottomhalfHandler = ptMpioHandler;
    this->bottomhalfHandler->BindMpioAllocator(mpioAllocator);

    partialMpioDoneNotifier = AsEntryPointParam1(&MpioHandler::EnqueuePartialMpio, bottomhalfHandler);
    mpioDonePoller = AsEntryPointNoParam(&MpioHandler::BottomhalfMioProcessing, bottomhalfHandler);
}

void
MioHandler::_HandleIoSQ(void)
{
    _PublishPeriodicMetrics();

    // if mio is not available, no new request can be serviced.
    if (!mioPool->GetFreeCount())
    {
        return;
    }

    MetaFsIoRequest* reqMsg = ioSQ->Dequeue();
    if (!reqMsg)
    {
        usleep(1);
        return;
    }
    reqMsg->StoreTimestamp(IoRequestStage::Dequeue);

    if (_IsRangeOverlapConflicted(reqMsg) || _IsPendedRange(reqMsg))
    {
        reqMsg->StoreTimestamp(IoRequestStage::EnqueueToRetryQ);
        _PushToRetry(reqMsg);
        return;
    }

    Mio* mio = DispatchMio(*reqMsg);
    if (!mio)
    {
        EnqueueNewReq(reqMsg);
        return;
    }

    _RegisterRangeLockInfo(reqMsg);

    ExecuteMio(*mio);
}

void
MioHandler::_UpdateSubmissionMetricsConditionally(const Mio& mio)
{
    issueCountByStorage[(int)mio.GetTargetStorage()]++;
    issueCountByFileType[(int)mio.GetFileType()]++;
}

void
MioHandler::_UpdateCompletionMetricsConditionally(Mio* mio)
{
    totalProcessedMioCount++;

    if (skipCount++ % SAMPLING_SKIP_COUNT == 0)
    {
        sampledTimeSpentProcessingAllStages += mio->GetElapsedInMilli(MioTimestampStage::Allocate, MioTimestampStage::Release).count();
        sampledTimeSpentFromIssueToComplete += mio->GetElapsedInMilli(MioTimestampStage::Issue, MioTimestampStage::Complete).count();
        sampledProcessedMioCount++;
        skipCount = 0;
    }
}

void
MioHandler::_PublishPeriodicMetrics(void)
{
    if (telemetryPublisher && metaFsTimeInterval.CheckInterval())
    {
        POSMetricVector* metricVector = new POSMetricVector();

        POSMetric mFreeMioCount(TEL40200_METAFS_FREE_MIO_CNT, POSMetricTypes::MT_GAUGE);
        mFreeMioCount.SetGaugeValue(mioPool->GetFreeCount());
        metricVector->emplace_back(mFreeMioCount);

        POSMetric metricMioHandlerWorking(TEL40205_METAFS_MIO_HANDLER_IS_WORKING, POSMetricTypes::MT_GAUGE);
        metricMioHandlerWorking.SetGaugeValue(GetCurrDateTimestamp());
        metricVector->emplace_back(metricMioHandlerWorking);

        if (totalProcessedMioCount)
        {
            POSMetric m(TEL40302_METAFS_PROCESSED_MIO_COUNT, POSMetricTypes::MT_GAUGE);
            m.SetGaugeValue(totalProcessedMioCount);
            metricVector->emplace_back(m);
            totalProcessedMioCount = 0;
        }

        for (uint32_t idx = 0; idx < NUM_STORAGE; idx++)
        {
            POSMetric m(TEL40103_METAFS_WORKER_ISSUE_COUNT_PARTITION, POSMetricTypes::MT_GAUGE);
            m.AddLabel("type", std::to_string(idx));
            m.SetGaugeValue(issueCountByStorage[idx]);
            metricVector->emplace_back(m);
            issueCountByStorage[idx] = 0;
        }

        for (uint32_t idx = 0; idx < NUM_FILE_TYPE; idx++)
        {
            POSMetric m(TEL40105_METAFS_WORKER_ISSUE_COUNT_FILE_TYPE, POSMetricTypes::MT_GAUGE);
            m.AddLabel("type", std::to_string(idx));
            m.SetGaugeValue(issueCountByFileType[idx]);
            metricVector->emplace_back(m);
            issueCountByFileType[idx] = 0;
        }

        if (sampledProcessedMioCount)
        {
            POSMetric mTimeSpentAllStage(TEL40301_METAFS_MIO_TIME_SPENT_PROCESSING_ALL_STAGES, POSMetricTypes::MT_GAUGE);
            mTimeSpentAllStage.SetGaugeValue(sampledTimeSpentProcessingAllStages);
            metricVector->emplace_back(mTimeSpentAllStage);

            POSMetric mTimeSpentIssueToComplete(TEL40203_METAFS_MIO_TIME_FROM_ISSUE_TO_COMPLETE, POSMetricTypes::MT_GAUGE);
            mTimeSpentIssueToComplete.SetGaugeValue(sampledTimeSpentFromIssueToComplete);
            metricVector->emplace_back(mTimeSpentIssueToComplete);

            POSMetric m(TEL40204_METAFS_MIO_SAMPLED_COUNT, POSMetricTypes::MT_GAUGE);
            m.SetGaugeValue(sampledProcessedMioCount);
            metricVector->emplace_back(m);

            sampledTimeSpentProcessingAllStages = 0;
            sampledTimeSpentFromIssueToComplete = 0;
            sampledProcessedMioCount = 0;
        }

        for (auto& item : *metricVector)
        {
            item.AddLabel("thread_name", std::to_string(coreId));
        }

        telemetryPublisher->PublishMetricList(metricVector);
    }
}

void
MioHandler::_HandleIoCQ(void)
{
    Mio* mio = ioCQ->Dequeue();
    if (mio)
    {
        mio->StoreTimestamp(MioTimestampStage::PopFromCQ);

        while (mio->IsCompleted() != true)
        {
        }

        _FreeLockContext(mio);
        _DiscoverIORangeOverlap(); // find other pending I/O

        if (mio->IsSyncIO())
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
        MetaFsIoRequest* pendingIoReq = it->second;

        if (!_IsRangeOverlapConflicted(pendingIoReq))
        {
            if (MetaStorageType::SSD != pendingIoReq->targetMediaType)
            {
                if (!_ExecutePendedIo(pendingIoReq))
                    break;
            }
            else
            {
                Mio* mio = DispatchMio(*pendingIoReq);
                if (!mio)
                    return;

                pendingIoRetryQ.erase(it);

                MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
                    "[Msg ][EraseRetryQ] type={}, req.tagId={}, fileOffset={}, Lpn={}, numPending={}",
                    pendingIoReq->reqType, pendingIoReq->tagId, pendingIoReq->byteOffsetInFile,
                    pendingIoReq->byteOffsetInFile / MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE,
                    pendingIoRetryQ.size());

                _RegisterRangeLockInfo(pendingIoReq);
                pendingIoReq->StoreTimestamp(IoRequestStage::DequeueToRetryQ);
                ExecuteMio(*mio);
            }
        }

        it = pendingIoRetryQ.upper_bound(pendingIoReq->baseMetaLpn);
    }
}

bool
MioHandler::_ExecutePendedIo(MetaFsIoRequest* reqMsg)
{
    Mio* mio = DispatchMio(*reqMsg);
    if (!mio)
        return false;

    _RegisterRangeLockInfo(reqMsg);

    std::vector<MetaFsIoRequest*>* reqList = new std::vector<MetaFsIoRequest*>();
    auto range = pendingIoRetryQ.equal_range(reqMsg->baseMetaLpn);

    for (auto it = range.first; it != range.second;)
    {
        MetaFsIoRequest* msg = it->second;
        if (reqMsg->arrayId == msg->arrayId)
        {
            reqList->push_back(msg);
            msg->StoreTimestamp(IoRequestStage::DequeueToRetryQ);
            pendingIoRetryQ.erase(it++);
        }
        else
        {
            ++it;
        }
    }

    mio->SetMergedRequestList(reqList);

    ExecuteMio(*mio);

    return true;
}

bool
MioHandler::_IsPendedRange(MetaFsIoRequest* reqMsg)
{
    auto it = pendingIoRetryQ.find(reqMsg->baseMetaLpn);

    if (it == pendingIoRetryQ.end())
    {
        return false;
    }
    else
    {
        for (; it != pendingIoRetryQ.end(); ++it)
        {
            if (reqMsg->targetMediaType == it->second->targetMediaType)
            {
                return true;
            }
        }
    }

    return false;
}

void
MioHandler::TophalfMioProcessing(void)
{
    _HandleIoSQ();
    _HandleIoCQ();
}

void
MioHandler::EnqueueNewReq(MetaFsIoRequest* reqMsg)
{
    reqMsg->StoreTimestamp(IoRequestStage::Enqueue);
    ioSQ->Enqueue(reqMsg, reqMsg->GetFileType());
}

void
MioHandler::_FinalizeMio(Mio* mio)
{
    mio->StoreTimestamp(MioTimestampStage::Release);
    _UpdateCompletionMetricsConditionally(mio);

    if (mio->GetMergedRequestList())
    {
        // metricCountOfStorageType[(int)mio->GetTargetStorage()] -= mio->GetMergedRequestList()->size();
        MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
            "merged request count: {}",
            mio->GetMergedRequestList()->size());
    }
    mio->Reset();
    mioPool->Release(mio);
}

Mio*
MioHandler::_AllocNewMio(MetaFsIoRequest& reqMsg)
{
    Mio* mio = mioPool->TryAlloc();

    if (!mio)
        return nullptr;

    MetaLpnType fileBaseLpn = reqMsg.fileCtx->fileBaseLpn;

    mio->StoreTimestamp(MioTimestampStage::Allocate);
    mio->Setup(&reqMsg, fileBaseLpn, reqMsg.fileCtx->storage);

    if (!mio->IsSyncIO())
    {
        mio->SetLocalAioCbCxt(mioCompletionCallback);
    }

    mio->SetMpioDoneNotifier(partialMpioDoneNotifier);
    mio->SetMpioDonePoller(mpioDonePoller);
    mio->SetIoCQ(ioCQ);

    reqMsg.targetMediaType = reqMsg.fileCtx->storageType;

    return mio;
}

Mio*
MioHandler::DispatchMio(MetaFsIoRequest& reqMsg)
{
    Mio* mio = _AllocNewMio(reqMsg);

    return mio;
}

void
MioHandler::_PushToRetry(MetaFsIoRequest* reqMsg)
{
    reqMsg->SetRetryFlag();
    pendingIoRetryQ.insert(pair<MetaLpnType, MetaFsIoRequest*>(reqMsg->baseMetaLpn, reqMsg));

    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "[Msg ][_PushToRetry] type={}, req.tagId={}, ByteOffset={}, Lpn={}, numPending={}",
        reqMsg->reqType, reqMsg->tagId, reqMsg->byteOffsetInFile,
        reqMsg->byteOffsetInFile / MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE,
        pendingIoRetryQ.size());
}

bool
MioHandler::_IsRangeOverlapConflicted(MetaFsIoRequest* reqMsg)
{
    if (reqMsg->reqType == MetaIoRequestType::Read)
    {
        return false;
    }

    int storage = (int)reqMsg->targetMediaType;
    int arrayId = reqMsg->arrayId;

    if (ioRangeOverlapChker[arrayId][storage])
        return ioRangeOverlapChker[arrayId][storage]->IsRangeOverlapConflicted(reqMsg);

    return false;
}

void
MioHandler::_RegisterRangeLockInfo(MetaFsIoRequest* reqMsg)
{
    int storage = (int)reqMsg->targetMediaType;
    int arrayId = reqMsg->arrayId;

    if (ioRangeOverlapChker[arrayId][storage])
        ioRangeOverlapChker[arrayId][storage]->PushReqToRangeLockMap(reqMsg);
}

void
MioHandler::_FreeLockContext(Mio* mio)
{
    int storage = (int)mio->GetTargetStorage();
    int arrayId = mio->GetArrayId();

    if (ioRangeOverlapChker[arrayId][storage])
        ioRangeOverlapChker[arrayId][storage]->FreeLockContext(mio->GetStartLpn(), mio->IsRead());
}

void
MioHandler::ExecuteMio(Mio& mio)
{
    _UpdateSubmissionMetricsConditionally(mio);
    mio.ExecuteAsyncState();
}

bool
MioHandler::AddArrayInfo(const int arrayId, const MaxMetaLpnMapPerMetaStorage& map)
{
    if (!map.size())
    {
        POS_TRACE_ERROR(EID(MFS_ERROR_MESSAGE),
            "There is no valid meta volume, coreId: {}",
            coreId);
        return false;
    }

    for (auto& entry : map)
    {
        uint32_t storage = (int)entry.first;
        ioRangeOverlapChker[arrayId][storage] = new MetaFsIoRangeOverlapChker();
        ioRangeOverlapChker[arrayId][storage]->Init(entry.second);
    }

    return true;
}

// for test
bool
MioHandler::AddArrayInfo(const int arrayId, const MetaStorageType type, MetaFsIoRangeOverlapChker* checker)
{
    ioRangeOverlapChker[arrayId][(int)type] = checker;

    return true;
}

bool
MioHandler::RemoveArrayInfo(const int arrayId)
{
    bool result = true;

    for (uint32_t storage = 0; storage < NUM_STORAGE; storage++)
    {
        if (!ioRangeOverlapChker[arrayId][storage])
        {
            result = false;
            continue;
        }

        delete ioRangeOverlapChker[arrayId][storage];
        ioRangeOverlapChker[arrayId][storage] = nullptr;
    }

    return result;
}

void
MioHandler::_HandleMioCompletion(void* data)
{
    // FIXME: will replace to handle below logic by mio completion event handler
    Mio* mio = reinterpret_cast<Mio*>(data);
    assert(mio->IsSyncIO() == false);

    std::vector<MetaFsIoRequest*>* reqList = mio->GetMergedRequestList();
    if (reqList)
    {
        for (auto it : *reqList)
        {
            MetaFsAioCbCxt* aiocb = reinterpret_cast<MetaFsAioCbCxt*>(it->aiocb);

            if (aiocb)
            {
                aiocb->SetErrorStatus(mio->GetError());
                _SendAioDoneEvent(aiocb);
            }
        }
    }
    else
    {
        MetaFsAioCbCxt* aiocb = reinterpret_cast<MetaFsAioCbCxt*>(mio->GetClientAioCbCxt());

        if (aiocb)
        {
            aiocb->SetErrorStatus(mio->GetError());
            _SendAioDoneEvent(aiocb);
        }
    }
}

void
MioHandler::_SendAioDoneEvent(void* aiocb)
{
    (reinterpret_cast<MetaFsAioCbCxt*>(aiocb))->InvokeCallback();
}
} // namespace pos

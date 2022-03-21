
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

#include "mio_handler.h"

#include <chrono>
#include <ctime>
#include <string>
#include <utility>

#include "Air.h"
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
  cpuStallCnt(0),
  MIO_POOL_SIZE(configManager->GetMioPoolCapacity()),
  MPIO_POOL_SIZE(configManager->GetMpioPoolCapacity()),
  WRITE_CACHE_CAPACITY(configManager->GetWriteMpioCacheCapacity()),
  TIME_INTERVAL_IN_MILLISECOND_FOR_METRIC(configManager->GetTimeIntervalInMillisecondsForMetric()),
  coreId(coreId),
  telemetryPublisher(tp),
  metricSumOfSpendTime(0),
  metricSumOfMioCount(0)
{
    ioCQ = new MetaFsIoMultilevelQ<Mio*, RequestPriority>();
    ioSQ = new MetaFsIoMultilevelQ<MetaFsIoRequest*, RequestPriority>();

    mpioAllocator = new MpioAllocator(configManager);
    _CreateMioPool();

    mioCompletionCallback = AsEntryPointParam1(&MioHandler::_HandleMioCompletion, this);

    lastTime = std::chrono::steady_clock::now();

    this->bottomhalfHandler = nullptr;
    POS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
        "Mio handler constructed. threadId: {}, coreId: {}, mio pool size: {}",
        threadId, coreId, MIO_POOL_SIZE);
}

MioHandler::MioHandler(const int threadId, const int coreId,
    MetaFsConfigManager* configManager,
    MetaFsIoMultilevelQ<MetaFsIoRequest*, RequestPriority>* ioSQ,
    MetaFsIoMultilevelQ<Mio*, RequestPriority>* ioCQ, MpioAllocator* mpioAllocator,
    MetaFsPool<Mio*>* mioPool, TelemetryPublisher* tp)
: ioSQ(ioSQ),
  ioCQ(ioCQ),
  mioPool(mioPool),
  mpioAllocator(mpioAllocator),
  cpuStallCnt(0),
  MIO_POOL_SIZE(configManager->GetMioPoolCapacity()),
  MPIO_POOL_SIZE(configManager->GetMpioPoolCapacity()),
  WRITE_CACHE_CAPACITY(configManager->GetWriteMpioCacheCapacity()),
  TIME_INTERVAL_IN_MILLISECOND_FOR_METRIC(configManager->GetTimeIntervalInMillisecondsForMetric()),
  coreId(coreId),
  telemetryPublisher(tp),
  metricSumOfSpendTime(0),
  metricSumOfMioCount(0)
{
    mioCompletionCallback = AsEntryPointParam1(&MioHandler::_HandleMioCompletion, this);

    lastTime = std::chrono::steady_clock::now();

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
    _SendPeriodicMetrics();

    // if mio is not available, no new request can be serviced.
    if (!mioPool->GetFreeCount())
    {
        return;
    }

    MetaFsIoRequest* reqMsg = ioSQ->Dequeue();
    if (!reqMsg)
    {
        if (cpuStallCnt++ > 1000)
        {
            usleep(1);
            cpuStallCnt = 0;
        }
        return;
    }
    reqMsg->StoreTimestamp(IoRequestStage::Dequeue);
    cpuStallCnt = 0;

    if (_IsRangeOverlapConflicted(reqMsg) || _IsPendedRange(reqMsg))
    {
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
MioHandler::_SendPeriodicMetrics(void)
{
    std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
    size_t elapsedTime = (size_t)(std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count());

    if (elapsedTime >= TIME_INTERVAL_IN_MILLISECOND_FOR_METRIC)
    {
        std::string thread_name = to_string(coreId);
        POSMetric metricFreeMioCnt(TEL40102_METAFS_FREE_MIO_CNT, POSMetricTypes::MT_GAUGE);
        metricFreeMioCnt.AddLabel("thread_name", thread_name);
        metricFreeMioCnt.SetGaugeValue(mioPool->GetFreeCount());
        telemetryPublisher->PublishMetric(metricFreeMioCnt);

        if (metricSumOfMioCount != 0)
        {
            POSMetric metricTime(TEL40106_METAFS_SUM_OF_ALL_THE_TIME_SPENT_BY_MIO, POSMetricTypes::MT_GAUGE);
            metricTime.AddLabel("thread_name", thread_name);
            metricTime.SetGaugeValue(metricSumOfSpendTime);
            telemetryPublisher->PublishMetric(metricTime);

            POSMetric metricCount(TEL40107_METAFS_SUM_OF_MIO_COUNT, POSMetricTypes::MT_GAUGE);
            metricCount.AddLabel("thread_name", thread_name);
            metricCount.SetGaugeValue(metricSumOfMioCount);
            telemetryPublisher->PublishMetric(metricCount);

            metricSumOfSpendTime = 0;
            metricSumOfMioCount = 0;
        }

        lastTime = currentTime;
    }
}

void
MioHandler::_HandleIoCQ(void)
{
    Mio* mio = ioCQ->Dequeue();
    if (mio)
    {
        mio->StoreTimestamp(MioTimestampStage::Dequeue);

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
            if (MetaStorageType::NVRAM == pendingIoReq->targetMediaType)
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

                MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
                    "[Msg ][EraseRetryQ] type={}, req.tagId={}, fileOffset={}, Lpn={}, numPending={}",
                    pendingIoReq->reqType, pendingIoReq->tagId, pendingIoReq->byteOffsetInFile,
                    pendingIoReq->byteOffsetInFile / MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE,
                    pendingIoRetryQ.size());

                _RegisterRangeLockInfo(pendingIoReq);
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
    ioSQ->Enqueue(reqMsg, reqMsg->priority);
}

void
MioHandler::_FinalizeMio(Mio* mio)
{
    mio->StoreTimestamp(MioTimestampStage::Release);
    metricSumOfSpendTime += mio->GetElapsedInMilli(MioTimestampStage::Allocate, MioTimestampStage::Release).count();
    metricSumOfMioCount++;
    if (mio->GetMergedRequestList())
    {
        MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
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
    mio->Setup(&reqMsg, fileBaseLpn, MetaFsServiceSingleton::Instance()->GetMetaFs(reqMsg.arrayId)->GetMss());

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

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
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
    int storage = mio->IsTargetStorageSSD() ? (int)MetaStorageType::SSD : (int)MetaStorageType::NVRAM;
    int arrayId = mio->GetArrayId();

    if (ioRangeOverlapChker[arrayId][storage])
        ioRangeOverlapChker[arrayId][storage]->FreeLockContext(mio->GetStartLpn(), mio->IsRead());
}

void
MioHandler::ExecuteMio(Mio& mio)
{
    mio.ExecuteAsyncState();
}

bool
MioHandler::AddArrayInfo(const int arrayId)
{
    MetaFs* metaFs = MetaFsServiceSingleton::Instance()->GetMetaFs(arrayId);
    bool result = true;

    for (uint32_t storage = 0; storage < NUM_STORAGE; storage++)
    {
        if (!metaFs->mgmt->IsValidVolume(static_cast<MetaVolumeType>(storage)))
        {
            result = false;
            continue;
        }

        size_t maxLpn = metaFs->ctrl->GetMaxMetaLpn(static_cast<MetaVolumeType>(storage));

        ioRangeOverlapChker[arrayId][storage] = new MetaFsIoRangeOverlapChker();
        ioRangeOverlapChker[arrayId][storage]->Init(maxLpn);
    }

    return result;
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

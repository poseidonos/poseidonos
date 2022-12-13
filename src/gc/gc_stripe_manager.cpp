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

#include "src/gc/gc_stripe_manager.h"

#include <air/Air.h>

#include <utility>

#include "src/allocator/i_block_allocator.h"
#include "src/allocator/i_wbstripe_allocator.h"
#include "src/allocator_service/allocator_service.h"
#include "src/gc/gc_flush_submission.h"
#include "src/include/branch_prediction.h"
#include "src/include/meta_const.h"
#include "src/include/array_config.h"
#include "src/io/general_io/translator.h"
#include "src/logger/logger.h"
#include "src/resource_manager/buffer_pool.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/master_context/config_manager.h"
#include "src/volume/volume_service.h"

namespace pos
{
GcStripeManager::GcStripeManager(IArrayInfo* iArrayInfo)
: GcStripeManager(iArrayInfo, VolumeEventPublisherSingleton::Instance())
{
}

void
GcStripeManager::_SetBufferPool(void)
{
    BufferInfo info = {
        .owner = typeid(this).name(),
        .size = CHUNK_SIZE,
        // Assign a buffer that is twice the size of the GC read buffer
        .count = udSize->chunksPerStripe * ArrayConfig::GC_BUFFER_COUNT * 2};

    gcWriteBufferPool = memoryManager->CreateBufferPool(info);
    assert(gcWriteBufferPool != nullptr);
}

GcStripeManager::GcStripeManager(IArrayInfo* iArrayInfo,
    VolumeEventPublisher* inputVolumeEventPublisher,
    MemoryManager* memoryManager)
: VolumeEvent("GcStripeManager", iArrayInfo->GetName(), iArrayInfo->GetIndex()),
  iArrayInfo(iArrayInfo),
  volumeEventPublisher(inputVolumeEventPublisher),
  memoryManager(memoryManager)
{
    _SetForceFlushInterval();
    udSize = iArrayInfo->GetSizeInfo(PartitionType::USER_DATA);

    for (uint32_t volId = 0; volId < GC_VOLUME_COUNT; volId++)
    {
        blkInfoList[volId] = nullptr;
        gcActiveWriteBuffers[volId] = nullptr;
        flushed[volId] = true;
    }

    gcStripeCntRequested = 0;
    gcStripeCntCompleted = 0;
    gcStripeCntFlushRequested = 0;
    gcStripeCntFlushCompleted = 0;
    gcStripeCntMapUpdateRequested = 0;
    gcStripeCntMapUpdateCompleted = 0;
    gcStripeCntForceFlushRequested = 0;

    volumeEventPublisher->RegisterSubscriber(this, arrayName, arrayId);
    _SetBufferPool();
    if (publisher == nullptr)
    {
        publisher = new TelemetryPublisher("GCStripeManager");
        publisher->AddDefaultLabel("array_id", to_string(arrayId));
        TelemetryClientSingleton::Instance()->RegisterPublisher(publisher);
    }
}

GcStripeManager::~GcStripeManager(void)
{
    TelemetryClientSingleton::Instance()->DeregisterPublisher(publisher->GetName());
    for (auto t : timer)
    {
        delete t.second;
    }
    timer.clear();

    for (uint32_t volumeId = 0; volumeId < GC_VOLUME_COUNT; volumeId++)
    {
        if (blkInfoList[volumeId] != nullptr)
        {
            blkInfoList[volumeId]->clear();
            delete blkInfoList[volumeId];
        }

        if (gcActiveWriteBuffers[volumeId] != nullptr)
        {
            _ReturnBuffer(gcActiveWriteBuffers[volumeId]);
            gcActiveWriteBuffers[volumeId]->clear();
            delete gcActiveWriteBuffers[volumeId];
        }
    }
    if (gcWriteBufferPool != nullptr)
    {
        memoryManager->DeleteBufferPool(gcWriteBufferPool);
    }

    volumeEventPublisher->RemoveSubscriber(this, arrayName, arrayId);
}

int
GcStripeManager::VolumeCreated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    return EID(VOL_EVENT_OK);
}

int
GcStripeManager::VolumeDeleted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo)
{
    if (false == _IsWriteBufferFull(volEventBase->volId))
    {
        timerMtx.lock();
        auto it = timer.find(volEventBase->volId);
        if (it != timer.end())
        {
            POS_TRACE_INFO(EID(GC_FORCE_FLUSH_TIMER_DISPOSE),
                "vol_id:{}", volEventBase->volId);
            timer.erase(it);
            delete (*it).second;
        }
        timerMtx.unlock();

        POS_TRACE_INFO(EID(GC_FORCE_FLUSH_TRYLOCK_WAITING),
                "vol_id:{}", volEventBase->volId);
        do
        {
            usleep(1000);
        } while (false == ffLocker.TryForceFlushLock((uint32_t)volEventBase->volId));

        GcWriteBuffer* writeBuffers = gcActiveWriteBuffers[volEventBase->volId];
        delete blkInfoList[volEventBase->volId];
        SetFlushed(volEventBase->volId);
        ReturnBuffer(writeBuffers);
        SetFinished();
    }
    _ResetFlushLock((uint32_t)volEventBase->volId);
    return EID(VOL_EVENT_OK);
}

int
GcStripeManager::VolumeMounted(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    return EID(VOL_EVENT_OK);
}

int
GcStripeManager::VolumeUnmounted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo)
{
    return EID(VOL_EVENT_OK);
}

int
GcStripeManager::VolumeLoaded(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    return EID(VOL_EVENT_OK);
}

int
GcStripeManager::VolumeUpdated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    return EID(VOL_EVENT_OK);
}

int
GcStripeManager::VolumeDetached(vector<int> volList, VolumeArrayInfo* volArrayInfo)
{
    return EID(VOL_EVENT_OK);
}

GcWriteBuffer*
GcStripeManager::GetWriteBuffer(uint32_t volumeId)
{
    return gcActiveWriteBuffers[volumeId];
}

GcAllocateBlks
GcStripeManager::AllocateWriteBufferBlks(uint32_t volumeId, uint32_t numBlks)
{
    std::unique_lock<std::mutex> bufferLock(gcWriteBufferLock[volumeId]);
    GcAllocateBlks gcAllocateBlks;

    if (_IsWriteBufferFull(volumeId))
    {
        bool ret = _CreateActiveWriteBuffer(volumeId);
        if (unlikely(false == ret))
        {
            gcAllocateBlks.startOffset = UINT32_MAX;
            gcAllocateBlks.numBlks = 0;
            airlog("CNT_GcSM_AllocWriteBuf", "buf_full_create_fail", 0, 1);
            return gcAllocateBlks;
        }
        flushed[volumeId] = false;

        _StartTimer(volumeId);
        _SetActiveStripeTail(volumeId, 0);
        _SetActiveStripeRemaining(volumeId, udSize->blksPerStripe);
        _CreateBlkInfoList(volumeId);
    }

    gcAllocateBlks = _AllocateBlks(volumeId, numBlks);
    if (0 == gcAllocateBlks.numBlks)
    {
        airlog("CNT_GcSM_AllocWriteBuf", "alloc_fail", 0, 1);
        return gcAllocateBlks;
    }
    airlog("CNT_GcSM_AllocWriteBuf", "pass", 0, 1);
    return gcAllocateBlks;
}

void
GcStripeManager::_ReturnBuffer(GcWriteBuffer* buffer)
{
    gcWriteBufferPool->ReturnBuffers(buffer);
}

void
GcStripeManager::SetFinished(void)
{
    ++gcStripeCntCompleted;

    POSMetricVector* v = new POSMetricVector();
    {
        POSMetric metric(TEL90000_GC_STRIPE_COUNT_REQUESTED, POSMetricTypes::MT_GAUGE);
        metric.SetGaugeValue(gcStripeCntRequested);
        metric.AddLabel("array_id", to_string(arrayId));
        v->push_back(metric);
    }
    {
        POSMetric metric(TEL90001_GC_STRIPE_COUNT_COMPLETED, POSMetricTypes::MT_GAUGE);
        metric.SetGaugeValue(gcStripeCntCompleted);
        metric.AddLabel("array_id", to_string(arrayId));
        v->push_back(metric);
    }
    {
        POSMetric metric(TEL90002_GC_STRIPE_COUNT_FLUSH_REQUESTED, POSMetricTypes::MT_GAUGE);
        metric.SetGaugeValue(gcStripeCntFlushRequested);
        metric.AddLabel("array_id", to_string(arrayId));
        v->push_back(metric);
    }
    {
        POSMetric metric(TEL90003_GC_STRIPE_COUNT_FLUSH_COMPLETED, POSMetricTypes::MT_GAUGE);
        metric.SetGaugeValue(gcStripeCntFlushCompleted);
        metric.AddLabel("array_id", to_string(arrayId));
        v->push_back(metric);
    }
    {
        POSMetric metric(TEL90004_GC_STRIPE_COUNT_MAP_UPDATE_REQUESTED, POSMetricTypes::MT_GAUGE);
        metric.SetGaugeValue(gcStripeCntMapUpdateRequested);
        metric.AddLabel("array_id", to_string(arrayId));
        v->push_back(metric);
    }
    {
        POSMetric metric(TEL90005_GC_STRIPE_COUNT_MAP_UPDATE_COMPLETED, POSMetricTypes::MT_GAUGE);
        metric.SetGaugeValue(gcStripeCntMapUpdateCompleted);
        metric.AddLabel("array_id", to_string(arrayId));
        v->push_back(metric);
    }
    {
        POSMetric metric(TEL90006_GC_STRIPE_COUNT_FORCE_FLUSH_REQUESTED, POSMetricTypes::MT_GAUGE);
        metric.SetGaugeValue(gcStripeCntForceFlushRequested);
        metric.AddLabel("array_id", to_string(arrayId));
        v->push_back(metric);
    }
    publisher->PublishMetricList(v);
}

void
GcStripeManager::ReturnBuffer(GcWriteBuffer* buffer)
{
    _ReturnBuffer(buffer);
    delete buffer;
}

bool
GcStripeManager::IsAllFinished(void)
{
    return (gcStripeCntRequested - gcStripeCntCompleted == 0);
}

std::vector<BlkInfo>*
GcStripeManager::GetBlkInfoList(uint32_t volumeId)
{
    return blkInfoList[volumeId];
}

bool
GcStripeManager::DecreaseRemainingAndCheckIsFull(uint32_t volumeId, uint32_t cnt)
{
    std::unique_lock<std::mutex> bufferLock(gcWriteBufferLock[volumeId]);

    uint32_t remaining = _DecreaseActiveStripeRemaining(volumeId, cnt);

    return (0 == remaining);
}

void
GcStripeManager::SetBlkInfo(uint32_t volumeId, uint32_t offset, BlkInfo blkInfo)
{
    blkInfoList[volumeId]->at(offset) = blkInfo;
}

void
GcStripeManager::SetFlushed(uint32_t volumeId, bool force)
{
    if (force == false)
    {
        unique_lock<mutex> lock(timerMtx);
        auto it = timer.find(volumeId);
        if (it != timer.end())
        {
            (*it).second->Reset();
        }
    }
    assert(flushed[volumeId] == false);
    blkInfoList[volumeId] = nullptr;
    gcActiveWriteBuffers[volumeId] = nullptr;
    flushed[volumeId] = true;
    ++gcStripeCntRequested;
}

void
GcStripeManager::CheckTimeout(void)
{
    uint32_t arrayId = iArrayInfo->GetIndex();
    unique_lock<mutex> lock(timerMtx);
    for (auto t : timer)
    {
        if (t.second->CheckTimeout() == true)
        {
            bool isForceFlush = true;
            uint32_t volId = t.first;
            POS_TRACE_WARN(EID(GC_FORCE_FLUSH_TIMER_TIMEOUT),
                "array_id:{}, vol_id:{}, interval:{}", arrayId, volId, timeoutInterval);

            if (ffLocker.TryForceFlushLock(volId) == false)
            {
                POS_TRACE_WARN(EID(GC_FORCE_FLUSH_TRYLOCK_FAILED),
                    "array_id:{}, vol_id:{}", arrayId, volId);
                continue;
            }

            std::vector<BlkInfo>* allocatedBlkInfoList = GetBlkInfoList(volId);
            GcWriteBuffer* dataBuffer = GetWriteBuffer(volId);
            IVolumeManager* volumeManager = VolumeServiceSingleton::Instance()->GetVolumeManager(iArrayInfo->GetIndex());
            if (unlikely(EID(SUCCESS) != volumeManager->IncreasePendingIOCountIfNotZero(volId, VolumeIoType::InternalIo)))
            {
                SetFlushed(volId, isForceFlush);
                allocatedBlkInfoList->clear();
                delete allocatedBlkInfoList;
                ReturnBuffer(dataBuffer);
                SetFinished();
                POS_TRACE_WARN(EID(GC_FORCE_FLUSH_REJECTED), "array_id:{}, vol_id:{}", arrayId, volId);
            }
            else
            {
                EventSmartPtr flushEvent = std::make_shared<GcFlushSubmission>(iArrayInfo->GetName(),
                            allocatedBlkInfoList, volId, dataBuffer, this, isForceFlush);
                EventSchedulerSingleton::Instance()->EnqueueEvent(flushEvent);
                SetFlushed(volId, isForceFlush);
                ++gcStripeCntForceFlushRequested;
                POS_TRACE_WARN(EID(GC_STRIPE_FORCIBLY_FLUSHED), "array_id:{}, vol_id:{}", arrayId, volId);
            }
            ffLocker.UnlockForceFlushLock(volId);
            t.second->Reset();
        }
    }
}

bool
GcStripeManager::TryFlushLock(uint32_t volId)
{
    return ffLocker.TryLock(volId);
}

void
GcStripeManager::ReleaseFlushLock(uint32_t volId)
{
    ffLocker.Unlock(volId);
}

void
GcStripeManager::FlushSubmitted(void)
{
    ++gcStripeCntFlushRequested;
}

void
GcStripeManager::FlushCompleted(void)
{
    ++gcStripeCntFlushCompleted;
}

void
GcStripeManager::UpdateMapRequested(void)
{
    ++gcStripeCntMapUpdateRequested;
}

void
GcStripeManager::UpdateMapCompleted(void)
{
    ++gcStripeCntMapUpdateCompleted;
}

uint32_t
GcStripeManager::_DecreaseActiveStripeRemaining(uint32_t volumeId, uint32_t cnt)
{
    assert(cnt <= gcActiveStripeRemaining[volumeId]);

    uint32_t remainCount = gcActiveStripeRemaining[volumeId] -= cnt;

    return remainCount;
}

void
GcStripeManager::_SetActiveStripeRemaining(uint32_t volumeId, uint32_t cnt)
{
    gcActiveStripeRemaining[volumeId] = cnt;
}

void
GcStripeManager::_CreateBlkInfoList(uint32_t volumeId)
{
    blkInfoList[volumeId] = new std::vector<BlkInfo>(udSize->blksPerStripe);
    BlkInfo invalidBlkInfo;
    invalidBlkInfo.volID = UINT32_MAX;
    for (auto it = blkInfoList[volumeId]->begin(); it != blkInfoList[volumeId]->end(); ++it)
    {
        (*it) = invalidBlkInfo;
    }
}

bool
GcStripeManager::_CreateActiveWriteBuffer(uint32_t volumeId)
{
    gcActiveWriteBuffers[volumeId] = new GcWriteBuffer();
    uint32_t bufCount = udSize->chunksPerStripe;
    uint32_t minAcqCount = bufCount;
    bool ret = gcWriteBufferPool->TryGetBuffers(bufCount, gcActiveWriteBuffers[volumeId], minAcqCount);
    if (ret == false)
    {
        bufAllocRetryCnt++;
        delete gcActiveWriteBuffers[volumeId];
        if (bufAllocRetryCnt % 100 == 0)
        {
            POS_TRACE_DEBUG(EID(GC_GET_WRITE_BUFFER_FAILED), "vol_id:{}, failure_count:{}",
                volumeId, bufAllocRetryCnt);
        }
    }
    else
    {
        bufAllocRetryCnt = 0;
    }
    return ret;
}

bool
GcStripeManager::_IsWriteBufferFull(uint32_t volumeId)
{
    return (true == flushed[volumeId]);
}

uint32_t
GcStripeManager::_GetActiveStripeTail(uint32_t volumeId)
{
    return gcActiveStripeTail[volumeId];
}

void
GcStripeManager::_SetActiveStripeTail(uint32_t volumeId, uint32_t offset)
{
    gcActiveStripeTail[volumeId] = offset;
}

GcAllocateBlks
GcStripeManager::_AllocateBlks(uint32_t volumeId, uint32_t numBlks)
{
    assert(0 != numBlks);

    GcAllocateBlks gcAllocateBlks;
    gcAllocateBlks.startOffset = _GetActiveStripeTail(volumeId);
    if (udSize->blksPerStripe == gcAllocateBlks.startOffset)
    {
        gcAllocateBlks.numBlks = 0;
        return gcAllocateBlks;
    }

    if (udSize->blksPerStripe < gcAllocateBlks.startOffset + numBlks)
    {
        gcAllocateBlks.numBlks = udSize->blksPerStripe - gcAllocateBlks.startOffset;
    }
    else
    {
        gcAllocateBlks.numBlks = numBlks;
    }

    _SetActiveStripeTail(volumeId, gcAllocateBlks.startOffset + gcAllocateBlks.numBlks);

    return gcAllocateBlks;
}

void
GcStripeManager::_SetForceFlushInterval(void)
{
    uint64_t intervalInSec = timeoutInterval;
    int ret = ConfigManagerSingleton::Instance()->GetValue("flow_control", "force_flush_timeout_in_sec",
            &intervalInSec, ConfigType::CONFIG_TYPE_UINT64);
    if (ret == 0)
    {
        timeoutInterval = intervalInSec;
    }
    POS_TRACE_DEBUG(EID(GC_FORCE_FLUSH_TIMER_INIT), "interval: {} sec", timeoutInterval);
    // convert second to nano second
    timeoutInterval = timeoutInterval * 1000000000ULL;
}

void
GcStripeManager::_StartTimer(uint32_t volumeId)
{
    timerMtx.lock();
    auto it = timer.find(volumeId);
    SystemTimeoutChecker* t = nullptr;
    if (it == timer.end())
    {
        t = new SystemTimeoutChecker();
        timer.emplace(volumeId, t);
    }
    else
    {
        t = (*it).second;
    }
    if (t->IsActive() == false)
    {
        t->SetTimeout(timeoutInterval);
    }
    timerMtx.unlock();
}

void
GcStripeManager::_ResetFlushLock(uint32_t volId)
{
    ffLocker.Reset(volId);
}
} // namespace pos

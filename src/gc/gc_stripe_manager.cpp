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
#include "src/io/general_io/translator.h"
#include "src/logger/logger.h"
#include "src/resource_manager/buffer_pool.h"
#include "src/sys_event/volume_event_publisher.h"

namespace pos
{
GcStripeManager::GcStripeManager(IArrayInfo* iArrayInfo)
: GcStripeManager(iArrayInfo, VolumeEventPublisherSingleton::Instance())
{
}

bool
GcStripeManager::_SetBufferPool(void)
{
    BufferInfo info = {
        .owner = typeid(this).name(),
        .size = CHUNK_SIZE,
        .count = udSize->chunksPerStripe * GC_WRITE_BUFFER_COUNT};

    gcWriteBufferPool = memoryManager->CreateBufferPool(info);
    if (gcWriteBufferPool == nullptr)
    {
        return false;
    }
    return true;
}

GcStripeManager::GcStripeManager(IArrayInfo* iArrayInfo,
    VolumeEventPublisher* inputVolumeEventPublisher,
    MemoryManager* memoryManager)
: VolumeEvent("GcStripeManager", iArrayInfo->GetName(), iArrayInfo->GetIndex()),
  iArrayInfo(iArrayInfo),
  volumeEventPublisher(inputVolumeEventPublisher),
  memoryManager(memoryManager)
{
    udSize = iArrayInfo->GetSizeInfo(PartitionType::USER_DATA);

    for (uint32_t volId = 0; volId < GC_VOLUME_COUNT; volId++)
    {
        blkInfoList[volId] = nullptr;
        gcActiveWriteBuffers[volId] = nullptr;
        flushed[volId] = true;
    }
    flushedStripeCnt = 0;
    volumeEventPublisher->RegisterSubscriber(this, arrayName, arrayId);
    if (_SetBufferPool() == false)
    {
        POS_TRACE_ERROR(EID(GC_ERROR_MSG),
            "Failed to allocated memory in GC_STRIPE_MANAGER");
    }
}

GcStripeManager::~GcStripeManager(void)
{
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
        GcWriteBuffer* writeBuffers = gcActiveWriteBuffers[volEventBase->volId];
        delete blkInfoList[volEventBase->volId];
        SetFlushed(volEventBase->volId);
        ReturnBuffer(writeBuffers);
        SetFinished();
    }
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
    for (auto it = buffer->begin(); it != buffer->end(); it++)
    {
        gcWriteBufferPool->ReturnBuffer(*it);
    }
}

void
GcStripeManager::SetFinished(void)
{
    flushedStripeCnt--;
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
    return (flushedStripeCnt == 0);
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
GcStripeManager::SetFlushed(uint32_t volumeId)
{
    assert(flushed[volumeId] == false);
    blkInfoList[volumeId] = nullptr;
    gcActiveWriteBuffers[volumeId] = nullptr;
    flushedStripeCnt++;
    flushed[volumeId] = true;
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
}

bool
GcStripeManager::_CreateActiveWriteBuffer(uint32_t volumeId)
{
    gcActiveWriteBuffers[volumeId] = new GcWriteBuffer();

    for (uint32_t chunkCnt = 0; chunkCnt < udSize->chunksPerStripe; ++chunkCnt)
    {
        void* buffer = gcWriteBufferPool->TryGetBuffer();
        if (nullptr == buffer)
        {
            for (auto it = gcActiveWriteBuffers[volumeId]->begin(); it != gcActiveWriteBuffers[volumeId]->end(); it++)
            {
                gcWriteBufferPool->ReturnBuffer(*it);
            }
            delete gcActiveWriteBuffers[volumeId];
            return false;
        }
        gcActiveWriteBuffers[volumeId]->push_back(buffer);
    }
    return true;
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
} // namespace pos

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

#include "src/gc/gc_stripe_manager.h"

#include "src/allocator_service/allocator_service.h"
#include "src/io/general_io/translator.h"
#include "src/include/meta_const.h"
#include "src/gc/gc_flush_submission.h"

#include "src/allocator/i_wbstripe_allocator.h"
#include "src/allocator/i_block_allocator.h"
#include "src/spdk_wrapper/free_buffer_pool.h"
#include "src/sys_event/volume_event_publisher.h"

#include "src/include/branch_prediction.h"

#include "src/logger/logger.h"

#include "Air.h"

#include <utility>

namespace pos
{
GcStripeManager::GcStripeManager(IArrayInfo* iArrayInfo)
: GcStripeManager(iArrayInfo,
                new FreeBufferPool(iArrayInfo->GetSizeInfo(PartitionType::USER_DATA)->chunksPerStripe * GC_WRITE_BUFFER_CONUNT, CHUNK_SIZE),
                nullptr,
                nullptr,
                VolumeEventPublisherSingleton::Instance())
{
}

GcStripeManager::GcStripeManager(IArrayInfo* iArrayInfo,
                                FreeBufferPool* inputGcWriteBufferPool,
                                std::vector<BlkInfo>* inputBlkInfoList,
                                GcWriteBuffer* inputGcActiveWriteBuffer,
                                VolumeEventPublisher* inputVolumeEventPublisher)
: VolumeEvent("GcStripeManager", iArrayInfo->GetName(), iArrayInfo->GetIndex()),
  iArrayInfo(iArrayInfo),
  gcWriteBufferPool(inputGcWriteBufferPool),
  inputBlkInfoList(inputBlkInfoList),
  inputGcActiveWriteBuffer(inputGcActiveWriteBuffer),
  volumeEventPublisher(inputVolumeEventPublisher)
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
        delete gcWriteBufferPool;
    }
    volumeEventPublisher->RemoveSubscriber(this, arrayName, arrayId);
}

bool
GcStripeManager::VolumeCreated(std::string volName, int volID, uint64_t volSizeBytem, uint64_t maxiops, uint64_t maxbw, std::string arrayName, int arrayID)
{
    return true;
}

bool
GcStripeManager::VolumeDeleted(std::string volName, int volID, uint64_t volSizeByte, std::string arrayName, int arrayID)
{
    if (false == _IsWriteBufferFull(volID))
    {
        GcWriteBuffer* writeBuffers = gcActiveWriteBuffers[volID];
        delete blkInfoList[volID];
        SetFlushed(volID);
        ReturnBuffer(writeBuffers);
        SetFinished();
    }
    return true;
}

bool
GcStripeManager::VolumeMounted(std::string volName, std::string subnqn, int volID, uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw, std::string arrayName, int arrayID)
{
    return true;
}

bool
GcStripeManager::VolumeUnmounted(std::string volName, int volID, std::string arrayName, int arrayID)
{
    return true;
}

bool
GcStripeManager::VolumeLoaded(std::string name, int id, uint64_t totalSize, uint64_t maxiops, uint64_t maxbw, std::string arrayName, int arrayID)
{
    return true;
}

bool
GcStripeManager::VolumeUpdated(std::string volName, int volID, uint64_t maxiops, uint64_t maxbw, std::string arrayName, int arrayID)
{
    return true;
}

void
GcStripeManager::VolumeDetached(vector<int> volList, std::string arrayName, int arrayID)
{
}

GcWriteBuffer*
GcStripeManager::GetWriteBuffer(uint32_t volumeId)
{
    return gcActiveWriteBuffers[volumeId];
}

bool
GcStripeManager::AllocateWriteBufferBlks(uint32_t volumeId, uint32_t numBlks, uint32_t& offset, uint32_t& allocatedBlks)
{
    std::unique_lock<std::mutex> bufferLock(GetWriteBufferLock(volumeId));
    bool ret;

    if (_IsWriteBufferFull(volumeId))
    {
        ret = _CreateActiveWriteBuffer(volumeId);
        if (unlikely(false == ret))
        {
            airlog("CNT_GcSM_AllocWriteBuf", "AIR_FALSE", 0, 1);
            return false;
        }
        flushed[volumeId] = false;
        _SetActiveStripeTail(volumeId, 0);
        _SetActiveStripeRemaining(volumeId, udSize->blksPerStripe);
        _CreateBlkInfoList(volumeId);
    }

    ret =  _AllocateBlks(volumeId, numBlks, offset, allocatedBlks);
    if (false == ret)
    {
        airlog("CNT_GcSM_AllocWriteBuf", "AIR_FALSE", 0, 1);
        return false;
    }
    airlog("CNT_GcSM_AllocWriteBuf", "AIR_TRUE", 0, 1);
    return true;
}

void
GcStripeManager::MoveActiveWriteBuffer(uint32_t volumeId, GcWriteBuffer* buffer)
{
    std::unique_lock<std::mutex> bufferLock(GetWriteBufferLock(volumeId));
    buffer = std::move(gcActiveWriteBuffers[volumeId]);
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

std::mutex&
GcStripeManager::GetWriteBufferLock(uint32_t volumeId)
{
    return gcWriteBufferLock[volumeId];
}

bool
GcStripeManager::DecreaseRemainingAndCheckIsFull(uint32_t volumeId, uint32_t cnt)
{
    std::unique_lock<std::mutex> bufferLock(GetWriteBufferLock(volumeId));

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
    if (nullptr == inputBlkInfoList)
    {
        blkInfoList[volumeId] = new std::vector<BlkInfo>(udSize->blksPerStripe);
    }
    else
    {
        blkInfoList[volumeId] = inputBlkInfoList;
    }
}

bool
GcStripeManager::_CreateActiveWriteBuffer(uint32_t volumeId)
{
    if (nullptr == inputGcActiveWriteBuffer)
    {
        gcActiveWriteBuffers[volumeId] = new GcWriteBuffer();
    }
    else
    {
        gcActiveWriteBuffers[volumeId] = inputGcActiveWriteBuffer;
    }

    for (uint32_t chunkCnt = 0; chunkCnt < udSize->chunksPerStripe; ++chunkCnt)
    {
        void* buffer = gcWriteBufferPool->GetBuffer();
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

bool
GcStripeManager::_AllocateBlks(uint32_t volumeId, uint32_t numBlks, uint32_t& offset, uint32_t& allocatedBlks)
{
    assert(0 != numBlks);

    offset = _GetActiveStripeTail(volumeId);
    if (udSize->blksPerStripe == offset)
    {
        allocatedBlks = 0;
        return false;
    }

    if (udSize->blksPerStripe < offset + numBlks)
    {
        allocatedBlks = udSize->blksPerStripe - offset;
    }
    else
    {
        allocatedBlks = numBlks;
    }

    _SetActiveStripeTail(volumeId, offset + allocatedBlks);

    return true;
}
} // namespace pos

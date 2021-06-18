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

#pragma once
#include <vector>

#include "src/allocator/stripe/stripe.h"
#include "src/array_models/interface/i_array_info.h"
#include "src/gc/victim_stripe.h"
#include "src/sys_event/volume_event.h"

#include <string>
#include <utility>

namespace pos
{
class IBlockAllocator;
class IWBStripeAllocator;

using GcWriteBuffer = std::vector<void*>;

class GcStripeManager : public VolumeEvent
{
public:
    explicit GcStripeManager(IArrayInfo* array, FreeBufferPool* gcWriteBufferPool_ = nullptr);
    ~GcStripeManager(void);

    virtual bool VolumeCreated(std::string volName, int volID, uint64_t volSizeBytem, uint64_t maxiops, uint64_t maxbw, std::string arrayName, int arrayID) override;
    virtual bool VolumeDeleted(std::string volName, int volID, uint64_t volSizeByte, std::string arrayName, int arrayID) override;
    virtual bool VolumeMounted(std::string volName, std::string subnqn, int volID, uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw, std::string arrayName, int arrayID) override;
    virtual bool VolumeUnmounted(std::string volName, int volID, std::string arrayName, int arrayID) override;
    virtual bool VolumeLoaded(std::string name, int id, uint64_t totalSize, uint64_t maxiops, uint64_t maxbw, std::string arrayName, int arrayID) override;
    virtual bool VolumeUpdated(std::string volName, int volID, uint64_t maxiops, uint64_t maxbw, std::string arrayName, int arrayID) override;
    virtual void VolumeDetached(vector<int> volList, std::string arrayName, int arrayID) override;

    virtual bool AllocateWriteBufferBlks(uint32_t volumeId, uint32_t numBlks, uint32_t& offset, uint32_t& allocatedBlks);
    virtual void MoveActiveWriteBuffer(uint32_t volumeId, GcWriteBuffer* buffer);
    virtual std::mutex& GetWriteBufferLock(uint32_t volumeId);
    virtual void SetFinished(GcWriteBuffer* buffer);
    virtual GcWriteBuffer* GetWriteBuffer(uint32_t volumeId);
    virtual bool DecreaseRemainingAndCheckIsFull(uint32_t volumeId, uint32_t cnt);
    virtual void SetBlkInfo(uint32_t volumeId, uint32_t offset, BlkInfo blkInfo);
    virtual std::vector<BlkInfo>* GetBlkInfoList(uint32_t volumeId);
    virtual void SetFlushed(uint32_t volumeId);
    virtual bool IsAllFinished(void);

    static const uint32_t GC_WRITE_BUFFER_CONUNT = 512;
    static const uint32_t GC_VOLUME_COUNT = MAX_VOLUME_COUNT;

private:
    void _ReturnBuffer(GcWriteBuffer* buffer);
    bool _CreateActiveWriteBuffer(uint32_t volumeId);
    uint32_t _GetActiveStripeTail(uint32_t volumeId);
    void _SetActiveStripeTail(uint32_t volumeId, uint32_t offset);
    uint32_t _DecreaseActiveStripeRemaining(uint32_t volumeId, uint32_t cnt);
    void _SetActiveStripeRemaining(uint32_t volumeId, uint32_t cnt);
    bool _AllocateBlks(uint32_t volumeId, uint32_t numBlks, uint32_t& offset, uint32_t& allocatedBlks);
    bool _IsWriteBufferFull(uint32_t volumeId);
    void _CreateBlkInfoList(uint32_t volumeId);

    std::vector<Stripe*> gcStripeArray;
    IArrayInfo* array;
    std::string arrayName;

    FreeBufferPool* gcWriteBufferPool;
    GcWriteBuffer* gcActiveWriteBuffers[GC_VOLUME_COUNT];
    uint32_t gcActiveStripeTail[GC_VOLUME_COUNT];
    uint32_t gcActiveStripeRemaining[GC_VOLUME_COUNT];
    bool flushed[GC_VOLUME_COUNT];

    std::mutex gcWriteBufferLock[GC_VOLUME_COUNT];
    std::vector<BlkInfo>* blkInfoList[GC_VOLUME_COUNT];
    const PartitionLogicalSize* udSize;
    std::atomic<uint32_t> flushedStripeCnt;
};

} // namespace pos

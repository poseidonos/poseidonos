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

#pragma once
#include <vector>

#include "src/allocator/stripe/stripe.h"
#include "src/array_models/interface/i_array_info.h"
#include "src/gc/victim_stripe.h"
#include "src/gc/force_flush_locker/force_flush_locker.h"
#include "src/sys_event/volume_event.h"
#include "src/resource_manager/memory_manager.h"
#include "src/lib/system_timeout_checker.h"
#include "src/telemetry/telemetry_client/telemetry_client.h"

#include <string>
#include <utility>
#include <unordered_map>
#include <mutex>

namespace pos
{
class IBlockAllocator;
class IWBStripeAllocator;
class VolumeEventPublisher;
class BufferPool;

using GcWriteBuffer = std::vector<void*>;
struct GcAllocateBlks
{
    uint32_t startOffset;
    uint32_t numBlks;

    inline bool
    operator==(GcAllocateBlks input) const
    {
        return (input.startOffset == startOffset && input.numBlks == numBlks);
    }
};

class GcStripeManager : public VolumeEvent
{
public:
    explicit GcStripeManager(IArrayInfo* iArrayInfo);
    GcStripeManager(IArrayInfo* iArrayInfo,
                    VolumeEventPublisher* inputVolumeEventPublisher,
                    MemoryManager* memoryManager =
                        MemoryManagerSingleton::Instance());
    ~GcStripeManager(void);

    virtual int VolumeCreated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo) override;
    virtual int VolumeDeleted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo) override;
    virtual int VolumeMounted(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo) override;
    virtual int VolumeUnmounted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo) override;
    virtual int VolumeLoaded(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo) override;
    virtual int VolumeUpdated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo) override;
    virtual int VolumeDetached(vector<int> volList, VolumeArrayInfo* volArrayInfo) override;

    virtual GcAllocateBlks AllocateWriteBufferBlks(uint32_t volumeId, uint32_t numBlks);
    virtual void SetFinished(void);
    virtual void ReturnBuffer(GcWriteBuffer* buffer);
    virtual GcWriteBuffer* GetWriteBuffer(uint32_t volumeId);
    virtual bool DecreaseRemainingAndCheckIsFull(uint32_t volumeId, uint32_t cnt);
    virtual void SetBlkInfo(uint32_t volumeId, uint32_t offset, BlkInfo blkInfo);
    virtual std::vector<BlkInfo>* GetBlkInfoList(uint32_t volumeId);
    virtual void SetFlushed(uint32_t volumeId, bool force = false);
    virtual bool IsAllFinished(void);
    void CheckTimeout(void);
    virtual bool TryFlushLock(uint32_t volId);
    virtual void ReleaseFlushLock(uint32_t volId);
    virtual void FlushSubmitted(void);
    virtual void FlushCompleted(void);
    virtual void UpdateMapRequested(void);
    virtual void UpdateMapCompleted(void);

    static const uint32_t GC_VOLUME_COUNT = MAX_VOLUME_COUNT;

private:
    void _ReturnBuffer(GcWriteBuffer* buffer);
    bool _CreateActiveWriteBuffer(uint32_t volumeId);
    uint32_t _GetActiveStripeTail(uint32_t volumeId);
    void _SetActiveStripeTail(uint32_t volumeId, uint32_t offset);
    uint32_t _DecreaseActiveStripeRemaining(uint32_t volumeId, uint32_t cnt);
    void _SetActiveStripeRemaining(uint32_t volumeId, uint32_t cnt);
    GcAllocateBlks _AllocateBlks(uint32_t volumeId, uint32_t numBlks);
    bool _IsWriteBufferFull(uint32_t volumeId);
    void _CreateBlkInfoList(uint32_t volumeId);
    void _SetBufferPool(void);
    void _SetForceFlushInterval(void);
    void _StartTimer(uint32_t volumeId);
    void _ResetFlushLock(uint32_t volId);

    std::vector<Stripe*> gcStripeArray;
    IArrayInfo* iArrayInfo;

    BufferPool* gcWriteBufferPool = nullptr;
    GcWriteBuffer* gcActiveWriteBuffers[GC_VOLUME_COUNT];
    uint32_t gcActiveStripeTail[GC_VOLUME_COUNT];
    uint32_t gcActiveStripeRemaining[GC_VOLUME_COUNT];
    bool flushed[GC_VOLUME_COUNT];

    std::mutex gcWriteBufferLock[GC_VOLUME_COUNT];
    std::vector<BlkInfo>* blkInfoList[GC_VOLUME_COUNT];
    const PartitionLogicalSize* udSize;

    std::atomic<uint64_t> gcStripeCntRequested;
    std::atomic<uint64_t> gcStripeCntCompleted;
    std::atomic<uint64_t> gcStripeCntFlushRequested;
    std::atomic<uint64_t> gcStripeCntFlushCompleted;
    std::atomic<uint64_t> gcStripeCntMapUpdateRequested;
    std::atomic<uint64_t> gcStripeCntMapUpdateCompleted;
    std::atomic<uint64_t> gcStripeCntForceFlushRequested;

    VolumeEventPublisher* volumeEventPublisher;
    MemoryManager* memoryManager;
    unordered_map<uint32_t, SystemTimeoutChecker*> timer;
    uint64_t timeoutInterval = 10;
    mutex timerMtx;
    ForceFlushLocker ffLocker;
    uint32_t bufAllocRetryCnt = 0;
    void _RegisterTelemetry(uint32_t arrayId);
    TelemetryPublisher* publisher = nullptr;
};

} // namespace pos

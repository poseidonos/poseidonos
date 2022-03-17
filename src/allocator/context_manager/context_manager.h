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

#include <set>
#include <string>
#include <vector>

#include "src/allocator/address/allocator_address_info.h"
#include "src/allocator/context_manager/block_allocation_status.h"
#include "src/allocator/context_manager/gc_ctx/gc_ctx.h"
#include "src/allocator/context_manager/rebuild_ctx/rebuild_ctx.h"
#include "src/allocator/i_context_manager.h"
#include "src/allocator/i_context_replayer.h"
#include "src/allocator/i_segment_ctx.h"
#include "src/allocator/include/allocator_const.h"
#include "src/state/interface/i_state_control.h"

namespace pos
{
class IAllocatorFileIoClient;
class AllocatorFileIoManager;
class AllocatorCtx;
class SegmentCtx;
class ContextReplayer;
class TelemetryPublisher;
class EventScheduler;

const int NO_REBUILD_TARGET_USER_SEGMENT = 0;

class ContextIoManager;

class ContextManager : public IContextManager, public ISegmentCtx
{
public:
    ContextManager(void) = default;
    ContextManager(TelemetryPublisher* tp,
        AllocatorCtx* allocCtx_, SegmentCtx* segCtx_, RebuildCtx* rebuildCtx_,
        GcCtx* gcCtx_, BlockAllocationStatus* blockAllocStatus_,
        ContextIoManager* ioManager,
        ContextReplayer* ctxReplayer_, AllocatorAddressInfo* info_, uint32_t arrayId_);
    ContextManager(TelemetryPublisher* tp, AllocatorAddressInfo* info, uint32_t arrayId_);
    virtual ~ContextManager(void);
    virtual void Init(void);
    virtual void Dispose(void);

    virtual int FlushContexts(EventSmartPtr callback, bool sync);
    virtual SegmentId AllocateFreeSegment(void);
    virtual SegmentId AllocateGCVictimSegment(void);
    virtual SegmentId AllocateRebuildTargetSegment(void);
    virtual int ReleaseRebuildSegment(SegmentId segmentId);
    virtual bool NeedRebuildAgain(void);
    virtual int StopRebuilding(void);
    virtual uint32_t GetRebuildTargetSegmentCount(void);
    virtual int MakeRebuildTargetSegmentList(void);
    virtual std::set<SegmentId> GetNvramSegmentList(void);
    virtual int GetGcThreshold(GcMode mode);
    virtual uint64_t GetStoredContextVersion(int owner);

    virtual void ValidateBlks(VirtualBlks blks) override;
    virtual void InvalidateBlks(VirtualBlks blks) override;
    virtual void UpdateOccupiedStripeCount(StripeId lsid) override;

    virtual int SetNextSsdLsid(void);
    virtual char* GetContextSectionAddr(int owner, int section);
    virtual int GetContextSectionSize(int owner, int section);

    virtual RebuildCtx* GetRebuildCtx(void) { return rebuildCtx; }

    // TODO (huijeong.kim) remove mixed use of SegmentCtx and ISegmentCtx
    virtual SegmentCtx* GetSegmentCtx(void) { return segmentCtx; }
    virtual ISegmentCtx* GetISegmentCtx(void) { return this; }
    virtual AllocatorCtx* GetAllocatorCtx(void) { return allocatorCtx; }
    virtual ContextReplayer* GetContextReplayer(void) { return contextReplayer; }
    virtual GcCtx* GetGcCtx(void) { return gcCtx; }
    virtual std::mutex& GetCtxLock(void) { return ctxLock; }

    virtual BlockAllocationStatus* GetAllocationStatus(void) { return blockAllocStatus; }

    virtual uint32_t GetArrayId(void);
private:
    void _ResetSegmentStates(void);

    ContextIoManager* ioManager;

    AllocatorAddressInfo* addrInfo;
    AllocatorCtx* allocatorCtx;
    SegmentCtx* segmentCtx;
    RebuildCtx* rebuildCtx;

    ContextReplayer* contextReplayer;
    GcCtx* gcCtx;
    BlockAllocationStatus* blockAllocStatus;

    uint32_t arrayId;
    std::mutex ctxLock;

    TelemetryPublisher* telPublisher;
};

} // namespace pos

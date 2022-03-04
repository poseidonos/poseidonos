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

#include "src/allocator/context_manager/context_manager.h"

#include <string>
#include <vector>

#include "src/allocator/context_manager/allocator_ctx/allocator_ctx.h"
#include "src/allocator/context_manager/context_io_manager.h"
#include "src/allocator/context_manager/context_replayer.h"
#include "src/allocator/context_manager/gc_ctx/gc_ctx.h"
#include "src/allocator/context_manager/io_ctx/allocator_io_ctx.h"
#include "src/allocator/context_manager/rebuild_ctx/rebuild_ctx.h"
#include "src/allocator/context_manager/segment_ctx/segment_ctx.h"
#include "src/allocator/include/allocator_const.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/logger/logger.h"
#include "src/qos/qos_manager.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"

namespace pos
{
ContextManager::ContextManager(TelemetryPublisher* tp,
    AllocatorCtx* allocCtx_, SegmentCtx* segCtx_, RebuildCtx* rebuildCtx_,
    GcCtx* gcCtx_, BlockAllocationStatus* blockAllocStatus_,
    ContextIoManager* ioManager_,
    ContextReplayer* ctxReplayer_, AllocatorAddressInfo* info_, uint32_t arrayId_)
: addrInfo(info_),
  arrayId(arrayId_)
{
    // for UT
    ioManager = ioManager_;
    allocatorCtx = allocCtx_;
    segmentCtx = segCtx_;
    rebuildCtx = rebuildCtx_;
    gcCtx = gcCtx_;
    blockAllocStatus = blockAllocStatus_;
    contextReplayer = ctxReplayer_;
    telPublisher = tp;
}

ContextManager::ContextManager(TelemetryPublisher* tp, AllocatorAddressInfo* info, uint32_t arrayId_)
: ContextManager(tp, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, info, arrayId_)
{
    allocatorCtx = new AllocatorCtx(tp, info);
    rebuildCtx = new RebuildCtx(tp, info);
    gcCtx = new GcCtx();
    blockAllocStatus = new BlockAllocationStatus();
    segmentCtx = new SegmentCtx(tp, rebuildCtx, info, gcCtx, blockAllocStatus);


    contextReplayer = new ContextReplayer(allocatorCtx, segmentCtx, info);

    AllocatorFileIo* rebuildFileIo = new AllocatorFileIo(REBUILD_CTX, rebuildCtx, addrInfo, arrayId);
    AllocatorFileIo* segmentFileIo = new AllocatorFileIo(SEGMENT_CTX, segmentCtx, addrInfo, arrayId);
    AllocatorFileIo* allocatorFileIo = new AllocatorFileIo(ALLOCATOR_CTX, allocatorCtx, addrInfo, arrayId);
    ioManager = new ContextIoManager(info, tp, segmentFileIo, allocatorFileIo, rebuildFileIo);

    rebuildCtx->SetAllocatorFileIo(rebuildFileIo);
}

ContextManager::~ContextManager(void)
{
    delete segmentCtx;
    delete rebuildCtx;
    delete allocatorCtx;
    delete gcCtx;
    delete blockAllocStatus;
    delete ioManager;
    delete contextReplayer;
}

void
ContextManager::Init(void)
{
    allocatorCtx->Init();
    segmentCtx->Init();
    rebuildCtx->Init();
    ioManager->Init();
}

void
ContextManager::UpdateOccupiedStripeCount(StripeId lsid)
{
    SegmentId segId = lsid / addrInfo->GetstripesPerSegment();
    bool segmentFreed = segmentCtx->IncreaseOccupiedStripeCount(segId);

    if (segmentFreed == true)
    {
        POS_TRACE_DEBUG(EID(ALLOCATOR_SEGMENT_FREED),
            "[FreeSegment] segmentId:{} freed by occupied stripe count", segId);

        segmentCtx->UpdateGcFreeSegment(arrayId);
    }
}

void
ContextManager::ValidateBlks(VirtualBlks blks)
{
    SegmentId segId = blks.startVsa.stripeId / addrInfo->GetstripesPerSegment();
    segmentCtx->IncreaseValidBlockCount(segId, blks.numBlks);
}

void
ContextManager::InvalidateBlks(VirtualBlks blks)
{
    SegmentId segId = blks.startVsa.stripeId / addrInfo->GetstripesPerSegment();
    bool segmentFreed = segmentCtx->DecreaseValidBlockCount(segId, blks.numBlks);
    if (segmentFreed == true)
    {
        POS_TRACE_DEBUG(EID(ALLOCATOR_SEGMENT_FREED),
            "[FreeSegment] segmentId:{} freed by valid block count", segId);

        segmentCtx->UpdateGcFreeSegment(arrayId);
    }
}

void
ContextManager::Dispose(void)
{
    ioManager->WaitPendingIo(ContextIoManager::IOTYPE_ALL);
    segmentCtx->Dispose();
    rebuildCtx->Dispose();
    allocatorCtx->Dispose();
    ioManager->Dispose();
}

int
ContextManager::FlushContexts(EventSmartPtr callback, bool sync)
{
    return ioManager->FlushContexts(callback, sync);
}

SegmentId
ContextManager::AllocateFreeSegment(void)
{
    return segmentCtx->AllocateFreeSegment();
}

SegmentId
ContextManager::AllocateGCVictimSegment(void)
{
    return segmentCtx->AllocateGCVictimSegment();
}

int
ContextManager::GetGcThreshold(GcMode mode)
{
    return mode == MODE_NORMAL_GC ? gcCtx->GetNormalGcThreshold() : gcCtx->GetUrgentThreshold();
}

int
ContextManager::SetNextSsdLsid(void)
{
    POS_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "@SetNextSsdLsid");
    SegmentId segId = AllocateFreeSegment();
    if (segId == UNMAP_SEGMENT)
    {
        return -EID(ALLOCATOR_NO_FREE_SEGMENT);
    }

    std::unique_lock<std::mutex> lock(allocatorCtx->GetCtxLock());
    allocatorCtx->SetNextSsdLsid(segId);

    return 0;
}

uint64_t
ContextManager::GetStoredContextVersion(int owner)
{
    return ioManager->GetStoredContextVersion(owner);
}

SegmentId
ContextManager::AllocateRebuildTargetSegment(void)
{
    return segmentCtx->GetRebuildTargetSegment();
}

bool
ContextManager::NeedRebuildAgain(void)
{
    bool needToContinue = segmentCtx->LoadRebuildList();
    return needToContinue;
}

int
ContextManager::ReleaseRebuildSegment(SegmentId segId)
{
    return segmentCtx->SetRebuildCompleted(segId);
}

int
ContextManager::MakeRebuildTargetSegmentList(std::set<SegmentId>& segmentList)
{
    return segmentCtx->MakeRebuildTarget(segmentList);
}

int
ContextManager::StopRebuilding(void)
{
    return segmentCtx->StopRebuilding();
}

char*
ContextManager::GetContextSectionAddr(int owner, int section)
{
    return ioManager->GetContextSectionAddr(owner, section);
}

int
ContextManager::GetContextSectionSize(int owner, int section)
{
    return ioManager->GetContextSectionSize(owner, section);
}

uint32_t
ContextManager::GetRebuildTargetSegmentCount(void)
{
    return segmentCtx->GetRebuildTargetSegmentCount();
}

uint32_t
ContextManager::GetArrayId(void)
{
    return arrayId;
}

} // namespace pos

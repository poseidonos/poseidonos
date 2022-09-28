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
#include "src/journal_manager/log_buffer/versioned_segment_ctx.h"
#include "src/allocator/include/allocator_const.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/logger/logger.h"
#include "src/qos/qos_manager.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"

namespace pos
{
ContextManager::ContextManager(TelemetryPublisher* tp,
    AllocatorCtx* allocCtx_, SegmentCtx* segCtx_, RebuildCtx* rebuildCtx_, IVersionedSegmentContext* versionedSegCtx_,
    GcCtx* gcCtx_, BlockAllocationStatus* blockAllocStatus_, ContextIoManager* ioManager_,
    ContextReplayer* ctxReplayer_, AllocatorAddressInfo* info_, uint32_t arrayId_)
: addrInfo(info_),
  arrayId(arrayId_),
  logGroupIdInProgress(INVALID_LOG_GROUP_ID),
  allowDuplicatedFlush(true)
{
    // for UT
    ioManager = ioManager_;
    allocatorCtx = allocCtx_;
    segmentCtx = segCtx_;
    rebuildCtx = rebuildCtx_;
    versionedSegCtx = versionedSegCtx_;
    gcCtx = gcCtx_;
    blockAllocStatus = blockAllocStatus_;
    contextReplayer = ctxReplayer_;
    telPublisher = tp;
}

ContextManager::ContextManager(TelemetryPublisher* tp, AllocatorAddressInfo* info, uint32_t arrayId_)
: ContextManager(tp, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, info, arrayId_)
{
    allocatorCtx = new AllocatorCtx(tp, info);
    rebuildCtx = new RebuildCtx(tp, info);
    blockAllocStatus = new BlockAllocationStatus();
    gcCtx = new GcCtx(blockAllocStatus);
    segmentCtx = new SegmentCtx(tp, rebuildCtx, info, gcCtx, arrayId);

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
ContextManager::Dispose(void)
{
    ioManager->WaitPendingIo(ContextIoManager::IOTYPE_ALL);
    segmentCtx->Dispose();
    rebuildCtx->Dispose();
    allocatorCtx->Dispose();
    ioManager->Dispose();
}

void
ContextManager::SetAllocateDuplicatedFlush(bool flag)
{
    allowDuplicatedFlush = flag;
}

int
ContextManager::FlushContexts(EventSmartPtr callback, bool sync, int logGroupId)
{
    if ((false == allowDuplicatedFlush) && (logGroupIdInProgress == logGroupId))
    {
        POS_TRACE_ERROR(EID(ALLOCATOR_REQUESTED_FLUSH_WITH_ALREADY_IN_USED_LOG_GROUP_ID),
            "Failed to flush contexts, log group {} is already in use",
            logGroupId);
        return EID(ALLOCATOR_REQUESTED_FLUSH_WITH_ALREADY_IN_USED_LOG_GROUP_ID);
    }

    logGroupIdInProgress = logGroupId;

    SegmentInfo* vscSegInfo = (true == sync) ? nullptr : versionedSegCtx->GetUpdatedInfoToFlush(logGroupId);
    return ioManager->FlushContexts(callback, sync, reinterpret_cast<char*>(vscSegInfo));
}

SegmentId
ContextManager::AllocateFreeSegment(void)
{
    // TODO(huijeong.kim) to reduce critical section
    std::lock_guard<std::mutex> lock(ctxLock);
    return segmentCtx->AllocateFreeSegment();
}

SegmentId
ContextManager::AllocateGCVictimSegment(void)
{
    std::lock_guard<std::mutex> lock(ctxLock);
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
        return ERRID(ALLOCATOR_NO_FREE_SEGMENT);
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
    std::lock_guard<std::mutex> lock(ctxLock);
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
ContextManager::MakeRebuildTargetSegmentList(void)
{
    return segmentCtx->MakeRebuildTarget();
}

std::set<SegmentId>
ContextManager::GetNvramSegmentList(void)
{
    return segmentCtx->GetNvramSegmentList();
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

void
ContextManager::PrepareVersionedSegmentCtx(IVersionedSegmentContext* versionedSegCtx_)
{
    versionedSegCtx = versionedSegCtx_;
}

void
ContextManager::ResetFlushedInfo(int logGroupId)
{
    POS_TRACE_INFO(EID(JOURNAL_CHECKPOINT_COMPLETED), "ContextManager::ResetFlushedInfo {}", logGroupId);

    if (ALL_LOG_GROUP == logGroupId)
    {
        for (int id = 0; id < versionedSegCtx->GetNumLogGroups(); id++)
        {
            versionedSegCtx->ResetFlushedInfo(id);
        }
    }
    else
    {
        versionedSegCtx->ResetFlushedInfo(logGroupId);
    }

    logGroupIdInProgress = INVALID_LOG_GROUP_ID;
}
} // namespace pos

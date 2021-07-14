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

#include "src/allocator/context_manager/rebuild_ctx/rebuild_ctx.h"

#include <string>
#include <utility>

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
RebuildCtx::RebuildCtx(RebuildCtxHeader* header, AllocatorCtx* allocCtx, AllocatorAddressInfo* info)
: addrInfo(info),
  ctxStoredVersion(0),
  ctxDirtyVersion(0),
  needContinue(false),
  targetSegmentCount(0),
  currentTarget(UINT32_MAX),
  allocatorCtx(allocCtx)
{
    if (header != nullptr)
    {
        // for UT
        ctxHeader.sig = header->sig;
        ctxHeader.ctxVersion = header->ctxVersion;
        ctxHeader.numTargetSegments = header->numTargetSegments;
    }
    else
    {
        ctxHeader.sig = SIG_REBUILD_CTX;
        ctxHeader.ctxVersion = 0;
        ctxHeader.numTargetSegments = 0;
    }
}
RebuildCtx::RebuildCtx(AllocatorCtx* allocCtx, AllocatorAddressInfo* info)
: RebuildCtx(nullptr, allocCtx, info)
{
}

RebuildCtx::~RebuildCtx(void)
{
}

void
RebuildCtx::Init(void)
{
    targetSegmentCount = 0;
    ctxHeader.ctxVersion = 0;
    ctxStoredVersion = 0;
    ctxDirtyVersion = 0;
}

void
RebuildCtx::Close(void)
{
}

void
RebuildCtx::AfterLoad(char* buf)
{
    POS_TRACE_DEBUG(EID(ALLOCATOR_FILE_ERROR), "RebuildCtx file loaded:{}", ctxHeader.ctxVersion);
    ctxDirtyVersion = ctxHeader.ctxVersion + 1;
    targetSegmentCount = ctxHeader.numTargetSegments;

    SegmentId* segmentList = reinterpret_cast<SegmentId*>(buf + sizeof(RebuildCtxHeader));
    for (uint32_t cnt = 0; cnt < targetSegmentCount; ++cnt)
    {
        auto pr = targetSegmentList.emplace(segmentList[cnt]);
        if (pr.second == false)
        {
            POS_TRACE_ERROR(EID(ALLOCATOR_MAKE_REBUILD_TARGET_FAILURE), "Failed to load RebuildCtx, segmentId:{} is already in set", segmentList[cnt]);
            while (addrInfo->IsUT() != true)
            {
                usleep(1); // assert(false);
            }
        }
    }
    POS_TRACE_DEBUG(EID(ALLOCATOR_META_ARCHIVE_LOAD_REBUILD_SEGMENT), "RebuildCtx file loaded, segmentCount:{}", targetSegmentCount);
    if (targetSegmentCount != 0)
    {
        assert(targetSegmentCount == targetSegmentList.size());
        needContinue = true;
    }
}

void
RebuildCtx::BeforeFlush(int section, char* buf)
{
    targetSegmentCount = targetSegmentList.size();
    ctxHeader.numTargetSegments = targetSegmentCount;
    ctxHeader.ctxVersion = ctxDirtyVersion++;
    memcpy(buf, &ctxHeader, sizeof(RebuildCtxHeader));

    int idx = 0;
    SegmentId* segmentList = reinterpret_cast<SegmentId*>(buf + sizeof(RebuildCtxHeader));
    for (const auto targetSegment : targetSegmentList)
    {
        segmentList[idx++] = targetSegment;
    }
    POS_TRACE_DEBUG(EID(ALLOCATOR_META_ARCHIVE_STORE_REBUILD_SEGMENT), "Ready to flush RebuildCtx file:{}, numTargetSegments:{}", ctxHeader.ctxVersion, ctxHeader.numTargetSegments);
}

void
RebuildCtx::FinalizeIo(AsyncMetaFileIoCtx* ctx)
{
    RebuildCtxHeader* header = reinterpret_cast<RebuildCtxHeader*>(ctx->buffer);
    ctxStoredVersion = header->ctxVersion;
    POS_TRACE_DEBUG(EID(ALLOCATOR_META_ARCHIVE_STORE_REBUILD_SEGMENT), "RebuildCtx file stored, version:{}, segmentCount:{}", header->ctxVersion, header->numTargetSegments);
}

char*
RebuildCtx::GetSectionAddr(int section)
{
    char* ret = nullptr;
    switch (section)
    {
        case RC_HEADER:
        {
            ret = (char*)&ctxHeader;
            break;
        }
        case RC_REBUILD_SEGMENT_LIST:
        {
            ret = nullptr;
            break;
        }
    }
    return ret;
}

int
RebuildCtx::GetSectionSize(int section)
{
    int ret = 0;
    switch (section)
    {
        case RC_HEADER:
        {
            ret = sizeof(RebuildCtxHeader);
            break;
        }
        case RC_REBUILD_SEGMENT_LIST:
        {
            ret = addrInfo->GetnumUserAreaSegments() * sizeof(SegmentId);
            break;
        }
    }
    return ret;
}

uint64_t
RebuildCtx::GetStoredVersion(void)
{
    return ctxStoredVersion;
}

void
RebuildCtx::ResetDirtyVersion(void)
{
    ctxDirtyVersion = 0;
}

SegmentId
RebuildCtx::GetRebuildTargetSegment(void)
{
    if (rebuildLock.try_lock() == false)
    {
        return NEED_TO_RETRY;
    }
    POS_TRACE_INFO(EID(ALLOCATOR_START), "@GetRebuildTargetSegment");
    if (targetSegmentList.empty() == true)
    {
        POS_TRACE_INFO(EID(ALLOCATOR_START), "No segment to rebuild: Exit");
        rebuildLock.unlock();
        return UINT32_MAX;
    }

    SegmentId segmentId = UINT32_MAX;
    while (targetSegmentList.empty() == false)
    {
        auto iter = targetSegmentList.begin();
        segmentId = *iter;
        if (allocatorCtx->GetSegmentState(segmentId, true) == SegmentState::FREE) // This segment had been freed by GC
        {
            POS_TRACE_INFO(EID(ALLOCATOR_TARGET_SEGMENT_FREED), "Skip Rebuilding segmentId:{}, Already Freed", segmentId);
            targetSegmentList.erase(iter);
            targetSegmentCount--;
            segmentId = UINT32_MAX;
            continue;
        }

        POS_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "Start to rebuild segmentId:{}", segmentId);
        currentTarget = segmentId;
        break;
    }

    rebuildLock.unlock();
    return segmentId;
}

int
RebuildCtx::ReleaseRebuildSegment(SegmentId segmentId)
{
    if (rebuildLock.try_lock() == false)
    {
        return -1; // Need to retry!
    }
    POS_TRACE_INFO(EID(ALLOCATOR_START), "@ReleaseRebuildSegment");

    auto iter = targetSegmentList.find(segmentId);
    if (iter == targetSegmentList.end())
    {
        POS_TRACE_ERROR(EID(ALLOCATOR_MAKE_REBUILD_TARGET_FAILURE), "There is no segmentId:{} in rebuild target list, seemed to be freed by GC", segmentId);
        currentTarget = UINT32_MAX;
        rebuildLock.unlock();
        return 0;
    }

    POS_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "segmentId:{} Rebuild Done!", segmentId);
    // Delete segmentId in rebuildTargetSegments
    targetSegmentList.erase(iter);
    targetSegmentCount--;
    currentTarget = UINT32_MAX;
    rebuildLock.unlock();
    return 1;
}

bool
RebuildCtx::NeedRebuildAgain(void)
{
    return needContinue;
}

int
RebuildCtx::FreeSegmentInRebuildTarget(SegmentId segId)
{
    if (targetSegmentList.empty() == true) // No need to check below
    {
        return 0;
    }

    std::unique_lock<std::mutex> lock(rebuildLock);
    auto iter = targetSegmentList.find(segId);
    if (iter == targetSegmentList.end())
    {
        return 0;
    }

    if (currentTarget == segId)
    {
        POS_TRACE_INFO(EID(ALLOCATOR_TARGET_SEGMENT_FREED), "segmentId:{} is reclaimed by GC, but still under rebuilding", segId);
        return 0;
    }

    targetSegmentList.erase(iter);
    targetSegmentCount--;
    POS_TRACE_INFO(EID(ALLOCATOR_TARGET_SEGMENT_FREED), "segmentId:{} in Rebuild Target has been Freed by GC", segId);
    return 1;
}

bool
RebuildCtx::IsRebuidTargetSegmentsEmpty(void)
{
    return targetSegmentList.empty();
}

bool
RebuildCtx::IsRebuildTargetSegment(SegmentId segId)
{
    return (targetSegmentList.find(segId) != targetSegmentList.end());
}

uint32_t
RebuildCtx::GetRebuildTargetSegmentCount(void)
{
    return targetSegmentCount;
}

RTSegmentIter
RebuildCtx::GetRebuildTargetSegmentsBegin(void)
{
    return targetSegmentList.begin();
}

RTSegmentIter
RebuildCtx::GetRebuildTargetSegmentsEnd(void)
{
    return targetSegmentList.end();
}

int
RebuildCtx::MakeRebuildTarget(void)
{
    POS_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "@MakeRebuildTarget()");

    if (targetSegmentList.empty() == false)
    {
        POS_TRACE_WARN(EID(ALLOCATOR_REBUILD_TARGET_SET_NOT_EMPTY), "targetSegmentList is NOT empty!");
        for (auto it = targetSegmentList.begin(); it != targetSegmentList.end(); ++it)
        {
            POS_TRACE_WARN(EID(ALLOCATOR_REBUILD_TARGET_SET_NOT_EMPTY), "residue was segmentId:{}", *it);
        }
        targetSegmentList.clear();
        targetSegmentCount = 0;
    }

    // Pick non-free segments and make rebuildTargetSegments
    SegmentId segmentId = 0;
    while (true)
    {
        segmentId = allocatorCtx->GetUsedSegment(segmentId);
        if (segmentId == UNMAP_SEGMENT)
        {
            break;
        }

        auto pr = targetSegmentList.emplace(segmentId);
        POS_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "segmentId:{} is inserted as target to rebuild", segmentId);
        if (pr.second == false)
        {
            POS_TRACE_ERROR(EID(ALLOCATOR_MAKE_REBUILD_TARGET_FAILURE), "segmentId:{} is already in set", segmentId);
            return -EID(ALLOCATOR_MAKE_REBUILD_TARGET_FAILURE);
        }
        ++segmentId;
    }
    targetSegmentCount = targetSegmentList.size();
    return 1;
}

int
RebuildCtx::StopRebuilding(void)
{
    if (targetSegmentList.empty() == true)
    {
        POS_TRACE_INFO(EID(ALLOCATOR_REBUILD_TARGET_SET_EMPTY), "Rebuild was already done or not happen");
        return -EID(ALLOCATOR_REBUILD_TARGET_SET_EMPTY);
    }

    targetSegmentList.clear();
    currentTarget = UINT32_MAX;
    targetSegmentCount = 0;
    return 1;
}

} // namespace pos

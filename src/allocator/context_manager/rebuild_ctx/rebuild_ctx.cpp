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

#include "src/allocator/context_manager/io_ctx/allocator_io_ctx.h"
#include "src/meta_file_intf/meta_file_include.h"
#include "src/metafs/metafs_file_intf.h"

namespace pos
{
RebuildCtx::RebuildCtx(std::string arrayName, AllocatorCtx* allocCtx, MetaFileIntf* rebuildSegFile)
: needRebuildCont(false),
  targetSegmentCnt(0),
  underRebuildSegmentId(UINT32_MAX),
  rebuildSegmentsFile(rebuildSegFile),
  bufferInObj(nullptr),
  initialized(false),
  arrayName(arrayName),
  allocatorCtx(allocCtx)
{
}

RebuildCtx::~RebuildCtx(void)
{
    Dispose();
}

void
RebuildCtx::Init(AllocatorAddressInfo* info)
{
    if (initialized == false)
    {
        targetSegmentCnt = info->GetnumUserAreaSegments();
        uint32_t bufSize = sizeof(uint32_t) + targetSegmentCnt * sizeof(SegmentId);
        bufferInObj = new char[bufSize];

        if (rebuildSegmentsFile == nullptr)
        {
            rebuildSegmentsFile = new FILESTORE("RebuildContext", arrayName);
        }

        if (rebuildSegmentsFile->DoesFileExist() == false)
        {
            rebuildSegmentsFile->Create(bufSize);
            rebuildSegmentsFile->Open();
            _StoreRebuildCtx();
        }
        else
        {
            rebuildSegmentsFile->Open();
            _LoadRebuildCtxSync();
            if (targetSegmentCnt != 0)
            {
                needRebuildCont = true;
            }
        }

        initialized = true;
    }
}

void
RebuildCtx::Dispose(void)
{
    if (initialized == true)
    {
        if (rebuildSegmentsFile != nullptr)
        {
            if (rebuildSegmentsFile->IsOpened() == true)
            {
                rebuildSegmentsFile->Close();
            }
            delete rebuildSegmentsFile;
            rebuildSegmentsFile = nullptr;
        }
        if (bufferInObj != nullptr)
        {
            delete[] bufferInObj;
            bufferInObj = nullptr;
        }

        initialized = false;
    }
}

SegmentId
RebuildCtx::GetRebuildTargetSegment(void)
{
    POS_TRACE_INFO(EID(ALLOCATOR_START), "@GetRebuildTargetSegment");

    if (GetTargetSegmentCnt() == 0)
    {
        return UINT32_MAX;
    }

    SegmentId segmentId = UINT32_MAX;
    while (IsRebuidTargetSegmentsEmpty() == false)
    {
        auto iter = RebuildTargetSegmentsBegin();
        segmentId = *iter;

        // This segment had been freed by GC,
        if (allocatorCtx->GetSegmentState(segmentId, true) == SegmentState::FREE)   // segStateLocks[segId].segLock
        {
            POS_TRACE_INFO(EID(ALLOCATOR_TARGET_SEGMENT_FREED), "This segmentId:{} was target but seemed to be freed by GC", segmentId);
            _EraseRebuildTargetSegments(iter);  // rebuildLock
            segmentId = UINT32_MAX;
            continue;
        }

        POS_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "segmentId:{} is going to be rebuilt", segmentId);
        SetUnderRebuildSegmentId(segmentId);
        break;
    }

    return segmentId;
}

int
RebuildCtx::ReleaseRebuildSegment(SegmentId segmentId)
{
    POS_TRACE_INFO(EID(ALLOCATOR_START), "@ReleaseRebuildSegment");

    auto iter = FindRebuildTargetSegment(segmentId);
    if (iter == RebuildTargetSegmentsEnd())
    {
        POS_TRACE_ERROR(EID(ALLOCATOR_MAKE_REBUILD_TARGET_FAILURE), "There is no segmentId:{} in rebuildTargetSegments, seemed to be freed by GC", segmentId);
        SetUnderRebuildSegmentId(UINT32_MAX);
        return 0;
    }

    POS_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "segmentId:{} Rebuild Done!", segmentId);
    // Delete segmentId in rebuildTargetSegments
    _EraseRebuildTargetSegments(iter);
    FlushRebuildCtx();
    SetUnderRebuildSegmentId(UINT32_MAX);
    return 0;
}

bool
RebuildCtx::NeedRebuildAgain(void)
{
    return needRebuildCont;
}

void
RebuildCtx::ClearRebuildTargetSegments(void)
{
    rebuildTargetSegments.clear();
}

void
RebuildCtx::FlushRebuildCtx(void)
{
    int lenToWrite = _PrepareRebuildCtx();

    AllocatorIoCtx* rebuildStoreRequest = new AllocatorIoCtx(MetaFsIoOpcode::Write,
        rebuildSegmentsFile->GetFd(), 0, lenToWrite, bufferInObj,
        std::bind(&RebuildCtx::_FlushRebuildCtxCompleted, this, std::placeholders::_1));
    rebuildStoreRequest->segmentCnt = targetSegmentCnt;

    rebuildSegmentsFile->AsyncIO(rebuildStoreRequest);
}

bool
RebuildCtx::IsRebuildTargetSegment(SegmentId segId)
{
    return (FindRebuildTargetSegment(segId) != RebuildTargetSegmentsEnd());
}

bool
RebuildCtx::IsRebuidTargetSegmentsEmpty(void)
{
    return rebuildTargetSegments.empty();
}

RTSegmentIter
RebuildCtx::RebuildTargetSegmentsBegin(void)
{
    return rebuildTargetSegments.begin();
}

std::pair<RTSegmentIter, bool>
RebuildCtx::EmplaceRebuildTargetSegment(SegmentId segmentId)
{
    return rebuildTargetSegments.emplace(segmentId);
}

RTSegmentIter
RebuildCtx::FindRebuildTargetSegment(SegmentId segmentId)
{
    return rebuildTargetSegments.find(segmentId);
}

RTSegmentIter
RebuildCtx::RebuildTargetSegmentsEnd(void)
{
    return rebuildTargetSegments.end();
}

void
RebuildCtx::FreeSegmentInRebuildTarget(SegmentId segId)
{
    if (IsRebuidTargetSegmentsEmpty()) // No need to check below
    {
        return;
    }

    auto iter = FindRebuildTargetSegment(segId);
    if (iter == RebuildTargetSegmentsEnd())
    {
        return;
    }

    if (_GetUnderRebuildSegmentId() == segId)
    {
        POS_TRACE_INFO(EID(ALLOCATOR_TARGET_SEGMENT_FREED), "segmentId:{} is reclaimed by GC, but still under rebuilding", segId);
        return;
    }

    _EraseRebuildTargetSegments(iter);
    POS_TRACE_INFO(EID(ALLOCATOR_TARGET_SEGMENT_FREED), "segmentId:{} in Rebuild Target has been Freed by GC", segId);
    FlushRebuildCtx();
}

SegmentId
RebuildCtx::_GetUnderRebuildSegmentId(void)
{
    return underRebuildSegmentId;
}

void
RebuildCtx::SetUnderRebuildSegmentId(SegmentId segmentId)
{
    underRebuildSegmentId = segmentId;
}

int
RebuildCtx::_PrepareRebuildCtx(void)
{
    targetSegmentCnt = rebuildTargetSegments.size();
    int lenToWrite = sizeof(uint32_t) + sizeof(SegmentId) * targetSegmentCnt;
    int buffer_offset = 0;

    uint32_t tgtSegCnt = targetSegmentCnt;
    memcpy(bufferInObj, &tgtSegCnt, sizeof(uint32_t));
    buffer_offset += sizeof(uint32_t);

    for (const auto targetSegment : rebuildTargetSegments)
    {
        memcpy(bufferInObj + buffer_offset, &targetSegment, sizeof(SegmentId));
        buffer_offset += sizeof(SegmentId);
    }
    return lenToWrite;
}

void
RebuildCtx::_StoreRebuildCtx(void)
{
    int lenToWrite = _PrepareRebuildCtx();
    rebuildSegmentsFile->IssueIO(MetaFsIoOpcode::Write, 0, lenToWrite, bufferInObj);
}

void
RebuildCtx::_EraseRebuildTargetSegments(RTSegmentIter iter)
{
    std::unique_lock<std::mutex> lock(rebuildLock);
    rebuildTargetSegments.erase(iter);
}

void
RebuildCtx::_LoadRebuildCtxSync(void)
{
    int lenToRead = sizeof(uint32_t) + sizeof(SegmentId) * targetSegmentCnt;
    rebuildSegmentsFile->IssueIO(MetaFsIoOpcode::Read, 0, lenToRead, bufferInObj);
    _RebuildCtxLoaded();
}

void
RebuildCtx::_RebuildCtxLoaded(void)
{
    int buffer_offset = 0;

    uint32_t tgtSegCnt = 0;
    memcpy(&tgtSegCnt, bufferInObj, sizeof(uint32_t));
    targetSegmentCnt = tgtSegCnt;
    buffer_offset += sizeof(uint32_t);

    for (uint32_t cnt = 0; cnt < targetSegmentCnt; ++cnt)
    {
        SegmentId segmentId;
        memcpy(&segmentId, bufferInObj + buffer_offset, sizeof(SegmentId));
        buffer_offset += sizeof(SegmentId);

        auto pr = rebuildTargetSegments.emplace(segmentId);
        if (pr.second == false)
        {
            POS_TRACE_ERROR(EID(ALLOCATOR_MAKE_REBUILD_TARGET_FAILURE), "segmentId:{} is already in set", segmentId);
            assert(false);
        }
    }

    POS_TRACE_DEBUG(EID(ALLOCATOR_META_ARCHIVE_LOAD_REBUILD_SEGMENT), "Rebuild segment file loaded, segmentCount:{}", targetSegmentCnt);
}

void
RebuildCtx::_FlushRebuildCtxCompleted(AsyncMetaFileIoCtx* ctx)
{
    int segmentCount = ((AllocatorIoCtx*)ctx)->segmentCnt;
    POS_TRACE_DEBUG(EID(ALLOCATOR_META_ARCHIVE_STORE_REBUILD_SEGMENT), "Rebuild Segment File Stored, segmentCount:{}", segmentCount);
    delete ctx;
}

} // namespace pos

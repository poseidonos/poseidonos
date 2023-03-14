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

#include "src/allocator/context_manager/rebuild_ctx/rebuild_ctx.h"

#include <string>
#include <utility>

#include "src/allocator/context_manager/allocator_file_io.h"
#include "src/allocator/context_manager/segment_ctx/segment_list.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"

namespace pos
{
RebuildCtx::RebuildCtx(TelemetryPublisher* tp_, RebuildCtxHeader* header, AllocatorAddressInfo* info)
: totalDataSize(0),
  listSize(0),
  ctxStoredVersion(0),
  ctxDirtyVersion(0),
  initialized(false),
  addrInfo(info),
  tp(tp_),
  fileIo(nullptr)
{
    if (header != nullptr)
    {
        // for UT
        ctxHeader.data = *header;
    }
    else
    {
        ctxHeader.data.sig = SIG_REBUILD_CTX;
        ctxHeader.data.ctxVersion = 0;
        ctxHeader.data.numTargetSegments = 0;
    }
}

RebuildCtx::RebuildCtx(TelemetryPublisher* tp_, AllocatorAddressInfo* info)
: RebuildCtx(tp_, nullptr, info)
{
}

RebuildCtx::~RebuildCtx(void)
{
    Dispose();
}

void
RebuildCtx::Init(void)
{
    if (initialized == true)
    {
        return;
    }

    listSize = 0;
    segmentList.data = new SegmentId[addrInfo->GetnumUserAreaSegments()]();
    ctxHeader.data.ctxVersion = 0;
    ctxStoredVersion = 0;
    ctxDirtyVersion = 0;

    _UpdateSectionInfo();

    initialized = true;
}

void
RebuildCtx::_UpdateSectionInfo(void)
{
    uint64_t currentOffset = 0;
    ctxHeader.InitAddressInfoWithItsData(currentOffset);
    currentOffset += ctxHeader.GetSectionSize();

    segmentList.InitAddressInfo(
        (char*)(segmentList.data),
        currentOffset,
        sizeof(SegmentId) * addrInfo->GetnumUserAreaSegments());
    currentOffset += segmentList.GetSectionSize();

    totalDataSize = currentOffset;
}

void
RebuildCtx::Dispose(void)
{
    if (initialized == false)
    {
        return;
    }

    if (segmentList.data != nullptr)
    {
        delete segmentList.data;
        segmentList.data = nullptr;
    }

    initialized = false;
}

void
RebuildCtx::SetAllocatorFileIo(AllocatorFileIo* fileIo_)
{
    fileIo = fileIo_;
}

void
RebuildCtx::AfterLoad(char* buf)
{
    RebuildCtxHeader* header = reinterpret_cast<RebuildCtxHeader*>(buf);
    assert(header->sig == SIG_REBUILD_CTX);

    // RC_HEADER
    ctxHeader.CopyFrom(buf);    

    // TODO load only listsize
    // RC_REBUILD_SEGMENT_LIST
    segmentList.CopyFrom(buf);

    POS_TRACE_DEBUG(EID(ALLOCATOR_FILE_ERROR), "RebuildCtx file loaded:{}", ctxHeader.data.ctxVersion);
    ctxStoredVersion = ctxHeader.data.ctxVersion;
    ctxDirtyVersion = ctxHeader.data.ctxVersion + 1;

    listSize = ctxHeader.data.numTargetSegments;
    POS_TRACE_DEBUG(EID(ALLOCATOR_META_ARCHIVE_LOAD_REBUILD_SEGMENT),
        "RebuildCtx file loaded, segmentCount:{}", listSize);
}

void
RebuildCtx::BeforeFlush(char* buf, ContextSectionBuffer externalBuf)
{
    // RC_HEADER
    ctxHeader.data.numTargetSegments = listSize;
    ctxHeader.data.ctxVersion = ctxDirtyVersion++;
    ctxHeader.CopyTo(buf);

    // RC_REBUILD_SEGMENT_LIST
    // TODO flush only listsize
    segmentList.CopyTo(buf);

    POS_TRACE_DEBUG(EID(ALLOCATOR_META_ARCHIVE_STORE_REBUILD_SEGMENT),
        "Ready to flush RebuildCtx file:{}, numTargetSegments:{}",
        ctxHeader.data.ctxVersion, ctxHeader.data.numTargetSegments);
}

void
RebuildCtx::AfterFlush(char* buf)
{
    RebuildCtxHeader* header = reinterpret_cast<RebuildCtxHeader*>(buf);
    assert(header->sig == SIG_REBUILD_CTX);

    ctxStoredVersion = header->ctxVersion;
    POS_TRACE_DEBUG(EID(ALLOCATOR_META_ARCHIVE_STORE_REBUILD_SEGMENT),
        "RebuildCtx stored, array_id:{}, version:{}, segmentCount:{}",
        addrInfo->GetArrayId(), header->ctxVersion, header->numTargetSegments);
}

ContextSectionAddr
RebuildCtx::GetSectionInfo(int section)
{
    if (section == RC_HEADER)
    {
        return ctxHeader.GetSectionInfo();
    }
    else if (section == RC_REBUILD_SEGMENT_LIST)
    {
        return segmentList.GetSectionInfo();
    }
    else
    {
        assert(false);
    }
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

int
RebuildCtx::GetNumSections(void)
{
    return NUM_REBUILD_CTX_SECTION;
}

uint64_t
RebuildCtx::GetTotalDataSize(void)
{
    return ctxHeader.GetSectionSize() + segmentList.GetSectionSize();
}

int
RebuildCtx::_FlushContext(void)
{
    FnAllocatorCtxIoCompletion completion = []() {}; // Do nothing on completion
    int ret = fileIo->Flush(completion);

    POS_TRACE_INFO(EID(ALLOCATOR_META_ARCHIVE_STORE),
        "[RebuildCtxFlush] rebuildIssuedCount:{}, start to flush",
        fileIo->GetNumOutstandingFlush());

    return ret;
}

int
RebuildCtx::FlushRebuildSegmentList(std::set<SegmentId> segIdSet)
{
    _UpdateRebuildList(segIdSet);
    return _FlushContext();
}

void
RebuildCtx::_UpdateRebuildList(std::set<SegmentId> segIdSet)
{
    listSize = segIdSet.size();
    int idx = 0;
    for (auto it = segIdSet.begin(); it != segIdSet.end(); it++)
    {
        segmentList.data[idx++] = *it;
    }
}

std::set<SegmentId>
RebuildCtx::GetList(void)
{
    std::set<SegmentId> ret;
    for (uint32_t idx = 0; idx < listSize; idx++)
    {
        ret.insert(segmentList.data[idx]);
    }
    return ret;
}

} // namespace pos

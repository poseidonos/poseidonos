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
: addrInfo(info),
  ctxStoredVersion(0),
  ctxDirtyVersion(0),
  listSize(0),
  segmentList(nullptr),
  tp(tp_),
  fileIo(nullptr),
  initialized(false)
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

RebuildCtx::RebuildCtx(TelemetryPublisher* tp_, AllocatorAddressInfo* info)
: RebuildCtx(tp_, nullptr, info)
{
}

RebuildCtx::~RebuildCtx(void)
{
}

void
RebuildCtx::Init(void)
{
    if (initialized == true)
    {
        return;
    }
    listSize = 0;
    segmentList = new SegmentId[addrInfo->GetnumUserAreaSegments()]();
    ctxHeader.ctxVersion = 0;
    ctxStoredVersion = 0;
    ctxDirtyVersion = 0;

    initialized = true;
}

void
RebuildCtx::Dispose(void)
{
    if (initialized == false)
    {
        return;
    }

    if (segmentList != nullptr)
    {
        delete segmentList;
        segmentList = nullptr;
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
    POS_TRACE_DEBUG(EID(ALLOCATOR_FILE_ERROR), "RebuildCtx file loaded:{}", ctxHeader.ctxVersion);
    ctxStoredVersion = ctxHeader.ctxVersion;
    ctxDirtyVersion = ctxHeader.ctxVersion + 1;

    listSize = ctxHeader.numTargetSegments;
    POS_TRACE_DEBUG(EID(ALLOCATOR_META_ARCHIVE_LOAD_REBUILD_SEGMENT),
        "RebuildCtx file loaded, segmentCount:{}", listSize);
}

void
RebuildCtx::BeforeFlush(char* buf)
{
    // RC_HEADER
    ctxHeader.numTargetSegments = listSize;
    ctxHeader.ctxVersion = ctxDirtyVersion++;
    memcpy(buf, &ctxHeader, sizeof(RebuildCtxHeader));

    // RC_REBUILD_SEGMENT_LIST
    memcpy(buf + sizeof(RebuildCtxHeader), segmentList, sizeof(SegmentId) * listSize);

    POS_TRACE_DEBUG(EID(ALLOCATOR_META_ARCHIVE_STORE_REBUILD_SEGMENT),
        "Ready to flush RebuildCtx file:{}, numTargetSegments:{}",
        ctxHeader.ctxVersion, ctxHeader.numTargetSegments);
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
            ret = (char*)segmentList;
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

std::string
RebuildCtx::GetFilename(void)
{
    return "RebuildContext";
}

uint32_t
RebuildCtx::GetSignature(void)
{
    return SIG_REBUILD_CTX;
}

int
RebuildCtx::GetNumSections(void)
{
    return NUM_REBUILD_CTX_SECTION;
}

int
RebuildCtx::_FlushContext(void)
{
    AllocatorCtxIoCompletion completion = []() {}; // Do nothing on completion
    int ret = fileIo->Flush(completion, INVALID_SECTION_ID);

    POS_TRACE_INFO(EID(ALLOCATOR_META_ARCHIVE_STORE),
        "[RebuildCtxFlush] rebuildIssuedCount:{}, start to flush",
        fileIo->GetNumFilesFlushing());

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
        segmentList[idx++] = *it;
    }
}

std::set<SegmentId>
RebuildCtx::GetList(void)
{
    std::set<SegmentId> ret;
    for (uint32_t idx = 0; idx < listSize; idx++)
    {
        ret.insert(segmentList[idx]);
    }
    return ret;
}

} // namespace pos

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

#include "src/allocator/context_manager/segment_ctx/segment_ctx.h"

#include <mutex>

#include "src/allocator/address/allocator_address_info.h"
#include "src/allocator/context_manager/segment_ctx/segment_info.h"
#include "src/include/meta_const.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
SegmentCtx::SegmentCtx(SegmentInfo* segmentInfo_, AllocatorAddressInfo* addrInfo_, std::string arrayName_)
: numSegments(0),
  addrInfo(addrInfo_),
  arrayName(arrayName_)
{
    // for UT
    segmentInfos = segmentInfo_;
}

SegmentCtx::SegmentCtx(AllocatorAddressInfo* info, std::string arrayName)
: SegmentCtx(nullptr, info, arrayName)
{
    ctxHeader.sig = SIG_SEGMENT_CTX;
}

SegmentCtx::~SegmentCtx(void)
{
}

void
SegmentCtx::Init(void)
{
    ctxHeader.ctxVersion = 0;
    ctxStoredVersion = 0;
    ctxDirtyVersion = 0;

    numSegments = addrInfo->GetnumUserAreaSegments();
    segmentInfos = new SegmentInfo[numSegments];
}

void
SegmentCtx::Close(void)
{
    delete[] segmentInfos;
    segmentInfos = nullptr;
}

uint32_t
SegmentCtx::IncreaseValidBlockCount(SegmentId segId, uint32_t cnt)
{
    uint32_t validCount = segmentInfos[segId].IncreaseValidBlockCount(cnt);
    uint32_t blksPerSegment = addrInfo->GetblksPerSegment();
    if (validCount > blksPerSegment)
    {
        POS_TRACE_ERROR(EID(VALID_COUNT_OVERFLOWED), "segmentId:{} increasedCount:{} total validCount:{} : OVERFLOWED", segId, cnt, validCount);
        assert(false);
    }
    return validCount;
}

int32_t
SegmentCtx::DecreaseValidBlockCount(SegmentId segId, uint32_t cnt)
{
    int32_t validCount = segmentInfos[segId].DecreaseValidBlockCount(cnt);
    if (validCount < 0)
    {
        POS_TRACE_ERROR(EID(VALID_COUNT_UNDERFLOWED), "segmentId:{} decreasedCount:{} total validCount:{} : UNDERFLOWED", segId, cnt, validCount);
        assert(false);
    }
    return validCount;
}

uint32_t
SegmentCtx::GetValidBlockCount(SegmentId segId)
{
    return segmentInfos[segId].GetValidBlockCount();
}

void
SegmentCtx::SetOccupiedStripeCount(SegmentId segId, int count)
{
    segmentInfos[segId].SetOccupiedStripeCount(count);
}

int
SegmentCtx::GetOccupiedStripeCount(SegmentId segId)
{
    return segmentInfos[segId].GetOccupiedStripeCount();
}

int
SegmentCtx::IncreaseOccupiedStripeCount(SegmentId segId)
{
    return segmentInfos[segId].IncreaseOccupiedStripeCount();
}

bool
SegmentCtx::IsSegmentCtxIo(char* buf)
{
    SegmentCtxHeader* header = reinterpret_cast<SegmentCtxHeader*>(buf);
    return (header->sig == (uint32_t)SIG_SEGMENT_CTX);
}

void
SegmentCtx::AfterLoad(char* buf)
{
    if (reinterpret_cast<SegmentCtxHeader*>(buf)->sig != SIG_SEGMENT_CTX)
    {
        POS_TRACE_DEBUG(EID(ALLOCATOR_FILE_ERROR), "segment ctx file signature is not matched:{}", ctxHeader.sig);
        assert(false);
    }
    else
    {
        POS_TRACE_DEBUG(EID(ALLOCATOR_FILE_ERROR), "segment ctx file Integrity check SUCCESS:{}", ctxHeader.ctxVersion);
    }
    ctxStoredVersion = ctxHeader.ctxVersion;
}

void
SegmentCtx::BeforeFlush(int section, char* buf)
{
    ctxHeader.ctxVersion = ctxDirtyVersion++;
}

void
SegmentCtx::FinalizeIo(AsyncMetaFileIoCtx* ctx)
{
    ctxStoredVersion = ((SegmentCtxHeader*)ctx->buffer)->ctxVersion;
}

char*
SegmentCtx::GetSectionAddr(int section)
{
    char* ret = nullptr;
    switch (section)
    {
        case SC_HEADER:
        {
            ret = (char*)&ctxHeader;
            break;
        }
        case SC_SEGMENT_INFO:
        {
            ret = (char*)segmentInfos;
            break;
        }
    }
    return ret;
}

int
SegmentCtx::GetSectionSize(int section)
{
    int ret = 0;
    switch (section)
    {
        case SC_HEADER:
        {
            ret = sizeof(SegmentCtxHeader);
            break;
        }
        case SC_SEGMENT_INFO:
        {
            ret = addrInfo->GetnumUserAreaSegments() * sizeof(SegmentInfo);
            break;
        }
    }
    return ret;
}

uint64_t
SegmentCtx::GetStoredVersion(void)
{
    return ctxStoredVersion;
}

void
SegmentCtx::ResetDirtyVersion(void)
{
    ctxDirtyVersion = 0;
}

void
SegmentCtx::CopySegmentInfoToBufferforWBT(WBTAllocatorMetaType type, char* dstBuf)
{
    uint32_t numSegs = addrInfo->GetnumUserAreaSegments(); // for ut
    uint32_t* dst = reinterpret_cast<uint32_t*>(dstBuf);
    for (uint32_t segId = 0; segId < numSegs; segId++)
    {
        if (type == WBT_SEGMENT_VALID_COUNT)
        {
            dst[segId] = segmentInfos[segId].GetValidBlockCount();
        }
        else
        {
            dst[segId] = segmentInfos[segId].GetOccupiedStripeCount();
        }
    }
}

void
SegmentCtx::CopySegmentInfoFromBufferforWBT(WBTAllocatorMetaType type, char* srcBuf)
{
    uint32_t numSegs = addrInfo->GetnumUserAreaSegments(); // for ut
    uint32_t* src = reinterpret_cast<uint32_t*>(srcBuf);
    for (uint32_t segId = 0; segId < numSegs; segId++)
    {
        if (type == WBT_SEGMENT_VALID_COUNT)
        {
            segmentInfos[segId].SetValidBlockCount(src[segId]);
        }
        else
        {
            segmentInfos[segId].SetOccupiedStripeCount(src[segId]);
        }
    }
}

} // namespace pos

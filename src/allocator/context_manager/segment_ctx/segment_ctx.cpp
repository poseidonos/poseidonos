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
#include "src/allocator/context_manager/segment_ctx/segment_lock.h"
#include "src/allocator/context_manager/segment_ctx/segment_states.h"
#include "src/allocator/context_manager/segment_ctx/segment_info.h"
#include "src/include/meta_const.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
SegmentCtx::SegmentCtx(SegmentCtxHeader* header, SegmentInfo* segmentInfo_, AllocatorAddressInfo* addrInfo_)
: SegmentCtx(header, segmentInfo_, nullptr, nullptr, addrInfo_)
{
}

SegmentCtx::SegmentCtx(SegmentCtxHeader* header, SegmentInfo* segmentInfo_, 
    SegmentStates* segmentStates_, SegmentLock* segmentStateLocks_,
    AllocatorAddressInfo* addrInfo_)
: ctxDirtyVersion(0),
  ctxStoredVersion(0),
  segmentStates(segmentStates_),
  numSegments(0),
  initialized(false),
  addrInfo(addrInfo_),
  segStateLocks(segmentStateLocks_)
{
    segmentInfos = segmentInfo_;
    if (header != nullptr)
    {
        // for UT
        ctxHeader.sig = header->sig;
        ctxHeader.ctxVersion = header->ctxVersion;
    }
    else
    {
        ctxHeader.sig = SIG_SEGMENT_CTX;
        ctxHeader.ctxVersion = 0;
    }
}

SegmentCtx::SegmentCtx(AllocatorAddressInfo* info)
: SegmentCtx(nullptr, nullptr, info)
{
}

SegmentCtx::~SegmentCtx(void)
{
    Dispose();
}

void
SegmentCtx::Init(void)
{
    if (initialized == true)
    {
        return;
    }

    ctxHeader.ctxVersion = 0;
    ctxStoredVersion = 0;
    ctxDirtyVersion = 0;

    numSegments = addrInfo->GetnumUserAreaSegments();
    segmentInfos = new SegmentInfo[numSegments];

    if (segmentStates == nullptr)
    {
        segmentStates = new SegmentStates[numSegments];
    }
    for (uint32_t segmentId = 0; segmentId < numSegments; ++segmentId)
    {
        segmentStates[segmentId].SetSegmentId(segmentId);
    }
    if (segStateLocks == nullptr)
    {
        segStateLocks = new SegmentLock[numSegments];
    }

    initialized = true;
}

void
SegmentCtx::Dispose(void)
{
    if (initialized == false)
    {
        return;
    }

    if (segmentInfos != nullptr)
    {
        delete[] segmentInfos;
        segmentInfos = nullptr;
    }

    if (segmentStates != nullptr)
    {
        delete[] segmentStates;
        segmentStates = nullptr;
    }

    if (segStateLocks != nullptr)
    {
        delete[] segStateLocks;
        segStateLocks = nullptr;
    }

    initialized = false;
}

uint32_t
SegmentCtx::IncreaseValidBlockCount(SegmentId segId, uint32_t cnt)
{
    uint32_t validCount = segmentInfos[segId].IncreaseValidBlockCount(cnt);
    uint32_t blksPerSegment = addrInfo->GetblksPerSegment();
    if (validCount > blksPerSegment)
    {
        POS_TRACE_ERROR(EID(VALID_COUNT_OVERFLOWED), "segmentId:{} increasedCount:{} total validCount:{} : OVERFLOWED", segId, cnt, validCount);
        while (addrInfo->IsUT() != true)
        {
            usleep(1); // assert(false);
        }
    }
    return validCount;
}

bool
SegmentCtx::DecreaseValidBlockCount(SegmentId segId, uint32_t cnt)
{
    bool segmentFreed = false;

    int32_t validCount = segmentInfos[segId].DecreaseValidBlockCount(cnt);
    if (validCount < 0)
    {
        POS_TRACE_ERROR(EID(VALID_COUNT_UNDERFLOWED), "segmentId:{} decreasedCount:{} total validCount:{} : UNDERFLOWED", segId, cnt, validCount);
        while (addrInfo->IsUT() != true)
        {
            usleep(1); // assert(false);
        }
    }

    if (validCount == 0)
    {
        std::lock_guard<std::mutex> lock(segStateLocks[segId].GetLock());
        SegmentState state = segmentStates[segId].GetState();
        if ((state == SegmentState::SSD) || (state == SegmentState::VICTIM))
        {
            assert(segmentInfos[segId].GetOccupiedStripeCount() == addrInfo->GetstripesPerSegment());

            segmentInfos[segId].SetOccupiedStripeCount(0);
            segmentStates[segId].SetState(SegmentState::FREE);

            segmentFreed = true;
        }
    }

    return segmentFreed;
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

bool
SegmentCtx::IncreaseOccupiedStripeCount(SegmentId segId)
{
    bool segmentFreed = false;

    uint32_t occupiedStripeCount = segmentInfos[segId].IncreaseOccupiedStripeCount();
    if (occupiedStripeCount == addrInfo->GetstripesPerSegment())
    {
        std::lock_guard<std::mutex> lock(segStateLocks[segId].GetLock());
        if (segmentInfos[segId].GetValidBlockCount() == 0)
        {
            if (segmentStates[segId].GetState() != SegmentState::FREE)
            {
                segmentInfos[segId].SetOccupiedStripeCount(0);
                segmentStates[segId].SetState(SegmentState::FREE);

                segmentFreed = true;
            }
        }
        else
        {
            segmentStates[segId].SetState(SegmentState::SSD);
        }
    }

    return segmentFreed;
}

void
SegmentCtx::AfterLoad(char* buf)
{
    POS_TRACE_DEBUG(EID(ALLOCATOR_FILE_ERROR), "SegmentCtx file loaded:{}", ctxHeader.ctxVersion);
    ctxDirtyVersion = ctxHeader.ctxVersion + 1;
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
        case AC_SEGMENT_STATES:
        {
            ret = (char*)segmentStates;
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
        case AC_SEGMENT_STATES:
        {
            ret = addrInfo->GetnumUserAreaSegments() * sizeof(SegmentStates);
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
SegmentCtx::SetSegmentState(SegmentId segId, SegmentState state, bool needlock)
{
    if (needlock == true)
    {
        std::lock_guard<std::mutex> lock(segStateLocks[segId].GetLock());
        segmentStates[segId].SetState(state);
    }
    else
    {
        segmentStates[segId].SetState(state);
    }
}

SegmentState
SegmentCtx::GetSegmentState(SegmentId segId, bool needlock)
{
    if (needlock == true)
    {
        std::lock_guard<std::mutex> lock(segStateLocks[segId].GetLock());
        return segmentStates[segId].GetState();
    }
    else
    {
        return segmentStates[segId].GetState();
    }
}

std::mutex&
SegmentCtx::GetSegStateLock(SegmentId segId)
{
    return segStateLocks[segId].GetLock();
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

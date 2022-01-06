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

#include "src/allocator/context_manager/segment_ctx/segment_ctx.h"

#include <mutex>

#include "src/allocator/address/allocator_address_info.h"
#include "src/include/meta_const.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"

namespace pos
{
SegmentCtx::SegmentCtx(TelemetryPublisher* tp_, SegmentCtxHeader* header, SegmentInfo* segmentInfo_, RebuildCtx* rebuildCtx_, AllocatorAddressInfo* addrInfo_)
: SegmentCtx(tp_, header, segmentInfo_, nullptr, rebuildCtx_, addrInfo_)
{
}

SegmentCtx::SegmentCtx(TelemetryPublisher* tp_, SegmentCtxHeader* header, SegmentInfo* segmentInfo_,
    BitMapMutex* segmentBitmap_,
    RebuildCtx* rebuildCtx_,
    AllocatorAddressInfo* addrInfo_)
: ctxDirtyVersion(0),
  ctxStoredVersion(0),
  allocSegBitmap(segmentBitmap_),
  initialized(false),
  addrInfo(addrInfo_),
  rebuildCtx(rebuildCtx_),
  tp(tp_)
{
    segmentInfos = segmentInfo_;
    if (header != nullptr)
    {
        // for UT
        ctxHeader.sig = header->sig;
        ctxHeader.ctxVersion = header->ctxVersion;
        ctxHeader.numValidSegment = header->numValidSegment;
    }
    else
    {
        ctxHeader.sig = SIG_SEGMENT_CTX;
        ctxHeader.ctxVersion = 0;
        ctxHeader.numValidSegment = 0;
    }
}

SegmentCtx::SegmentCtx(TelemetryPublisher* tp_, RebuildCtx* rebuildCtx_, AllocatorAddressInfo* info)
: SegmentCtx(tp_, nullptr, nullptr, rebuildCtx_, info)
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

    uint32_t numSegments = addrInfo->GetnumUserAreaSegments();
    segmentInfos = new SegmentInfo[numSegments];

    if (allocSegBitmap == nullptr)
    {
        allocSegBitmap = new BitMapMutex(numSegments);
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

    if (allocSegBitmap != nullptr)
    {
        delete allocSegBitmap;
        allocSegBitmap = nullptr;
    }

    initialized = false;
}

void
SegmentCtx::IncreaseValidBlockCount(SegmentId segId, uint32_t cnt)
{
    uint32_t increasedValue = segmentInfos[segId].IncreaseValidBlockCount(cnt);
    if (increasedValue > addrInfo->GetblksPerSegment())
    {
        POS_TRACE_ERROR(EID(VALID_COUNT_OVERFLOWED),
            "segmentId:{} increasedCount:{} total validCount:{} : OVERFLOWED", segId, cnt, increasedValue);
        assert(false);
    }
}

bool
SegmentCtx::DecreaseValidBlockCount(SegmentId segId, uint32_t cnt)
{
    uint32_t decreasedValue = segmentInfos[segId].DecreaseValidBlockCount(cnt);

    if (decreasedValue < 0)
    {
        POS_TRACE_ERROR(EID(VALID_COUNT_UNDERFLOWED),
            "segmentId:{} decreasedCount:{} total validCount:{} : UNDERFLOWED", segId, cnt, decreasedValue);
        assert(false);
    }

    if (decreasedValue == 0)
    {
        _SegmentFreed(segId);
        return true;
    }
    return false;
}

uint32_t
SegmentCtx::GetValidBlockCount(SegmentId segId)
{
    return segmentInfos[segId].GetValidBlockCount();
}

bool
SegmentCtx::IncreaseOccupiedStripeCount(SegmentId segId)
{
    uint32_t occupiedStripeCount = segmentInfos[segId].IncreaseOccupiedStripeCount();
    if (occupiedStripeCount == addrInfo->GetstripesPerSegment())
    {
        bool segmentFreed = segmentInfos[segId].MoveToSsdStateOrFreeStateIfItBecomesEmpty();
        if (segmentFreed == true)
        {
            _SegmentFreed(segId);
            return true;
        }
    }
    return false;
}

int
SegmentCtx::GetOccupiedStripeCount(SegmentId segId)
{
    return segmentInfos[segId].GetOccupiedStripeCount();
}

void
SegmentCtx::AfterLoad(char* buf)
{
    POS_TRACE_DEBUG(EID(ALLOCATOR_FILE_ERROR), "SegmentCtx file loaded:{}", ctxHeader.ctxVersion);
    allocSegBitmap->SetNumBitsSet(ctxHeader.numValidSegment);
    ctxStoredVersion = ctxHeader.ctxVersion;
    ctxDirtyVersion = ctxHeader.ctxVersion + 1;
}

void
SegmentCtx::BeforeFlush(char* buf)
{
    ctxHeader.ctxVersion = ctxDirtyVersion++;
    ctxHeader.numValidSegment = allocSegBitmap->GetNumBitsSet();
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
        case AC_ALLOCATE_SEGMENT_BITMAP:
        {
            ret = (char*)allocSegBitmap->GetMapAddr();
            break;
        }
    }
    return ret;
}

int
SegmentCtx::GetSectionSize(int section)
{
    uint64_t ret = 0;
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
        case AC_ALLOCATE_SEGMENT_BITMAP:
        {
            ret = allocSegBitmap->GetNumEntry() * BITMAP_ENTRY_SIZE;
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

std::string
SegmentCtx::GetFilename(void)
{
    return "SegmentContext";
}

uint32_t
SegmentCtx::GetSignature(void)
{
    return SIG_SEGMENT_CTX;
}

int
SegmentCtx::GetNumSections(void)
{
    return NUM_SEGMENT_CTX_SECTION;
}

SegmentState
SegmentCtx::GetSegmentState(SegmentId segId)
{
    return segmentInfos[segId].GetState();
}

void
SegmentCtx::AllocateSegment(SegmentId segId)
{
    segmentInfos[segId].MoveToNvramState();
    allocSegBitmap->SetBit(segId);
}

SegmentId
SegmentCtx::AllocateFreeSegment(void)
{
    SegmentId segId = UNMAP_SEGMENT;

    while (true)
    {
        if (segId == UNMAP_SEGMENT)
        {
            segId = allocSegBitmap->SetNextZeroBit();
        }
        else
        {
            segId = allocSegBitmap->SetFirstZeroBit(segId);
        }

        if (allocSegBitmap->IsValidBit(segId) == false)
        {
            POS_TRACE_ERROR(EID(ALLOCATOR_NO_FREE_SEGMENT),
                "[AllocateSegment] failed to allocate segment, free segment count:{}, rebuild target count: {}",
                GetNumOfFreeSegmentWoLock(),
                rebuildCtx->GetRebuildTargetSegmentCount());

            segId = UNMAP_SEGMENT;
            POS_TRACE_ERROR(EID(ALLOCATOR_NO_FREE_SEGMENT), "[AllocateSegment] failed to allocate segment, free segment count:{}", GetNumOfFreeSegmentWoLock());
            break;
        }
        else if (rebuildCtx->IsRebuildTargetSegment(segId) == true)
        {
            allocSegBitmap->ClearBit(segId);
            segId++;
            continue;
        }
        else
        {
            segmentInfos[segId].MoveToNvramState();

            uint64_t freeSegCount = GetNumOfFreeSegmentWoLock();
            POS_TRACE_INFO(EID(ALLOCATOR_START), "[AllocateSegment] allocate segmentId:{}, free segment count:{}", segId, freeSegCount);
            POSMetricValue v;
            v.gauge = freeSegCount;
            tp->PublishData(TEL30000_ALCT_FREE_SEG_CNT, v, MT_GAUGE);

            break;
        }
    }
    return segId;
}

SegmentId
SegmentCtx::_GetUsedSegment(SegmentId startSegId)
{
    SegmentId segId = allocSegBitmap->FindFirstSetBit(startSegId);
    if (allocSegBitmap->IsValidBit(segId) == false)
    {
        segId = UNMAP_SEGMENT;
    }
    return segId;
}

uint64_t
SegmentCtx::GetNumOfFreeSegment(void)
{
    return allocSegBitmap->GetNumBits() - allocSegBitmap->GetNumBitsSet();
}

uint64_t
SegmentCtx::GetNumOfFreeSegmentWoLock(void)
{
    return allocSegBitmap->GetNumBits() - allocSegBitmap->GetNumBitsSetWoLock();
}

void
SegmentCtx::SetAllocatedSegmentCount(int count)
{
    allocSegBitmap->SetNumBitsSet(count);
}

int
SegmentCtx::GetAllocatedSegmentCount(void)
{
    return allocSegBitmap->GetNumBitsSet();
}

SegmentId
SegmentCtx::AllocateGCVictimSegment(void)
{
    SegmentId victimSegment = _FindMostInvalidSSDSegment();
    if (victimSegment != UNMAP_SEGMENT)
    {
        segmentInfos[victimSegment].MoveToVictimState();

        POSMetricValue validCount;
        validCount.gauge = segmentInfos[victimSegment].GetValidBlockCount();
        tp->PublishData(TEL30010_ALCT_VICTIM_SEG_INVALID_PAGE_CNT, validCount, MT_GAUGE);

        POSMetricValue victimSegmentId;
        victimSegmentId.gauge = victimSegment;
        tp->PublishData(TEL30002_ALCT_GCVICTIM_SEG, victimSegmentId, MT_GAUGE);

        POS_TRACE_INFO(EID(ALLOCATE_GC_VICTIM), "[AllocateSegment] victim segmentId:{}, free segment count:{}",
            victimSegment, GetNumOfFreeSegmentWoLock());
    }

    return victimSegment;
}

SegmentId
SegmentCtx::_FindMostInvalidSSDSegment(void)
{
    uint32_t numUserAreaSegments = addrInfo->GetnumUserAreaSegments();
    SegmentId victimSegment = UNMAP_SEGMENT;
    uint32_t minValidCount = addrInfo->GetblksPerSegment();
    for (SegmentId segId = 0; segId < numUserAreaSegments; ++segId)
    {
        uint32_t cnt = segmentInfos[segId].GetValidBlockCountIfSsdState();
        if (cnt < minValidCount)
        {
            victimSegment = segId;
            minValidCount = cnt;
        }
    }
    return victimSegment;
}

void
SegmentCtx::_SegmentFreed(SegmentId segmentId)
{
    allocSegBitmap->ClearBit(segmentId);

    uint32_t freeSegCount = GetNumOfFreeSegmentWoLock();
    POS_TRACE_INFO(EID(ALLOCATOR_SEGMENT_FREED),
        "[FreeSegment] release segmentId:{} was freed, free segment count:{}", segmentId, freeSegCount);
}

void
SegmentCtx::ResetSegmentsStates(void)
{
    for (uint32_t segId = 0; segId < addrInfo->GetnumUserAreaSegments(); ++segId)
    {
        if (segmentInfos[segId].GetState() == SegmentState::VICTIM)
        {
            segmentInfos[segId].MoveToSsdStateOrFreeStateIfItBecomesEmpty();
            POS_TRACE_INFO(EID(SEGMENT_WAS_VICTIM), "segmentId:{} was VICTIM, so changed to SSD", segId);
        }
        else if (segmentInfos[segId].GetState() == SegmentState::SSD)
        {
            bool freed = segmentInfos[segId].MoveToSsdStateOrFreeStateIfItBecomesEmpty();
            if (freed == true)
            {
                POS_TRACE_INFO(EID(ALLOCATOR_SEGMENT_FREED), "segmentId:{} was All Invalidated, so changed to FREE", segId);
            }
        }
    }
}

SegmentId
SegmentCtx::GetRebuildTargetSegment(void)
{
    POS_TRACE_INFO(EID(ALLOCATOR_START), "@GetRebuildTargetSegment");

    SegmentId segmentId = UNMAP_SEGMENT;

    while (true)
    {
        segmentId = rebuildCtx->GetRebuildTargetSegment();
        if (segmentId == UNMAP_SEGMENT)
        {
            segmentId = UINT32_MAX;
            break;
        }
        else if (segmentInfos[segmentId].GetState() == SegmentState::FREE)
        {
            POS_TRACE_INFO(EID(ALLOCATOR_TARGET_SEGMENT_FREED), "Skip Rebuilding segmentId:{}, Already Freed", segmentId);
            rebuildCtx->EraseRebuildTargetSegment(segmentId);
        }
        else
        {
            POS_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "Start to rebuild segmentId:{}", segmentId);
            break;
        }
    }
    return segmentId;
}

int
SegmentCtx::MakeRebuildTarget(void)
{
    POS_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "@MakeRebuildTarget()");

    std::set<SegmentId> usedSegmentList;
    _GetUsedSegmentList(usedSegmentList);

    return rebuildCtx->InitializeTargetSegmentList(usedSegmentList);
}

void
SegmentCtx::_GetUsedSegmentList(std::set<SegmentId>& segmentList)
{
    int cnt = 0;
    SegmentId segmentId = 0;
    while (true)
    {
        // Pick non-free segments and make rebuildTargetSegments
        segmentId = _GetUsedSegment(segmentId);
        if (segmentId == UNMAP_SEGMENT)
        {
            break;
        }
        else
        {
            segmentList.emplace(segmentId);
            ++cnt;
        }
        ++segmentId;
    }
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

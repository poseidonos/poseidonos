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
: SegmentCtx(tp_, header, segmentInfo_, nullptr, nullptr, rebuildCtx_, addrInfo_)
{
}

SegmentCtx::SegmentCtx(TelemetryPublisher* tp_, SegmentCtxHeader* header, SegmentInfo* segmentInfo_,
    SegmentList* freeSegmentList, SegmentList* rebuildSegmentList,
    RebuildCtx* rebuildCtx_,
    AllocatorAddressInfo* addrInfo_)
: ctxDirtyVersion(0),
  ctxStoredVersion(0),
  freeList(freeSegmentList),
  rebuildList(rebuildSegmentList),
  rebuildingSegment(UNMAP_SEGMENT),
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

    if (freeList == nullptr)
    {
        freeList = new SegmentList();
    }

    if (rebuildList == nullptr)
    {
        rebuildList = new SegmentList();
    }

    _RebuildFreeSegmentList();

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

    if (freeList != nullptr)
    {
        delete freeList;
        freeList = nullptr;
    }

    if (rebuildList != nullptr)
    {
        delete rebuildList;
        rebuildList = nullptr;
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
    ctxStoredVersion = ctxHeader.ctxVersion;
    ctxDirtyVersion = ctxHeader.ctxVersion + 1;

    _RebuildFreeSegmentList();
}

void
SegmentCtx::_RebuildFreeSegmentList(void)
{
    freeList->Reset();

    for (uint32_t segId = 0; segId < addrInfo->GetnumUserAreaSegments(); ++segId)
    {
        if (segmentInfos[segId].GetState() == SegmentState::FREE)
        {
            freeList->AddToList(segId);
        }
    }
}

void
SegmentCtx::BeforeFlush(char* buf)
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
    // Only used by context replayer. Free segment list will be rebuilt after replay is completed
    segmentInfos[segId].MoveToNvramState();
}

SegmentId
SegmentCtx::AllocateFreeSegment(void)
{
    while (true)
    {
        SegmentId segId = freeList->PopSegment();
        if (segId == UNMAP_SEGMENT)
        {
            POS_TRACE_ERROR(EID(ALLOCATOR_NO_FREE_SEGMENT),
                "[AllocateSegment] failed to allocate segment, free segment count:{}, rebuild target count: {}",
                GetNumOfFreeSegmentWoLock(),
                rebuildList->GetNumSegmentsWoLock());

            POS_TRACE_ERROR(EID(ALLOCATOR_NO_FREE_SEGMENT), "[AllocateSegment] failed to allocate segment, free segment count:{}", GetNumOfFreeSegmentWoLock());
            break;
        }
        else if (rebuildList->Contains(segId) == true)
        {
            freeList->AddToList(segId);
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

            return segId;
        }
    }

    return UNMAP_SEGMENT;
}

uint64_t
SegmentCtx::GetNumOfFreeSegment(void)
{
    return freeList->GetNumSegments();
}

uint64_t
SegmentCtx::GetNumOfFreeSegmentWoLock(void)
{
    return freeList->GetNumSegmentsWoLock();
}

int
SegmentCtx::GetAllocatedSegmentCount(void)
{
    return addrInfo->GetnumUserAreaSegments() - freeList->GetNumSegments();
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
    if (rebuildingSegment == segmentId)
    {
        POS_TRACE_INFO(EID(ALLOCATOR_TARGET_SEGMENT_FREED),
            "segmentId:{} is reclaimed by GC, but still under rebuilding", segmentId);
        return;
    }

    bool rebuildListChanged = rebuildList->RemoveFromList(segmentId);
    if (rebuildListChanged == true)
    {
        POS_TRACE_INFO(EID(ALLOCATOR_TARGET_SEGMENT_FREED),
            "segmentId:{} in Rebuild Target has been Freed by GC", segmentId);

        rebuildCtx->FlushRebuildSegmentList(rebuildList->GetList());
    }

    freeList->AddToList(segmentId);

    uint32_t freeSegCount = GetNumOfFreeSegmentWoLock();
    POS_TRACE_INFO(EID(ALLOCATOR_SEGMENT_FREED),
        "[FreeSegment] release segmentId:{} was freed, free segment count:{}", segmentId, freeSegCount);
}

void
SegmentCtx::ResetSegmentsStates(void)
{
    for (uint32_t segId = 0; segId < addrInfo->GetnumUserAreaSegments(); ++segId)
    {
        bool segmentFreed = false;
        if (segmentInfos[segId].GetState() == SegmentState::VICTIM)
        {
            segmentFreed = segmentInfos[segId].MoveToSsdStateOrFreeStateIfItBecomesEmpty();
            if (segmentFreed == false)
            {
                POS_TRACE_INFO(EID(SEGMENT_WAS_VICTIM), "segmentId:{} was VICTIM, so changed to SSD", segId);
            }
        }
        else if (segmentInfos[segId].GetState() == SegmentState::SSD)
        {
            segmentFreed = segmentInfos[segId].MoveToSsdStateOrFreeStateIfItBecomesEmpty();
        }

        if (segmentFreed == true)
        {
            POS_TRACE_INFO(EID(ALLOCATOR_SEGMENT_FREED), "segmentId:{} was All Invalidated, so changed to FREE", segId);
        }
    }

    _RebuildFreeSegmentList();
}

SegmentId
SegmentCtx::GetRebuildTargetSegment(void)
{
    SegmentId segmentId = UNMAP_SEGMENT;

    while (true)
    {
        segmentId = rebuildList->GetFrontSegment();
        if (segmentId == UNMAP_SEGMENT)
        {
            segmentId = UINT32_MAX;
            break;
        }
        else if (segmentInfos[segmentId].GetState() == SegmentState::FREE)
        {
            POS_TRACE_INFO(EID(ALLOCATOR_TARGET_SEGMENT_FREED), "Skip Rebuilding segmentId:{}, Already Freed", segmentId);
            rebuildList->RemoveFromList(segmentId);
            rebuildCtx->FlushRebuildSegmentList(rebuildList->GetList());
        }
        else
        {
            rebuildingSegment = segmentId;

            POS_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "GetRebuildTargetSegment: Start to rebuild segmentId:{}", segmentId);
            break;
        }
    }

    return segmentId;
}

int
SegmentCtx::SetRebuildCompleted(SegmentId segId)
{
    _ResetSegmentIdInRebuilding();

    if (rebuildList->RemoveFromList(segId) == true)
    {
        int ret = rebuildCtx->FlushRebuildSegmentList(rebuildList->GetList());

        POS_TRACE_INFO(EID(ALLOCATOR_REBUILD_SEGMENT_COMPLETED),
            "Segment {} is removed from the rebuild list", segId);

        if (segmentInfos[segId].GetState() == SegmentState::FREE)
        {
            freeList->AddToList(segId);
        }
        return ret;
    }
    else
    {
        POS_TRACE_ERROR(EID(ALLOCATOR_MAKE_REBUILD_TARGET_FAILURE),
            "There is no segmentId:{} in rebuild target list, seemed to be freed by GC",
            segId);

        return 0;
    }
}

int
SegmentCtx::MakeRebuildTarget(std::set<SegmentId>& segmentList)
{
    _BuildRebuildSegmentList();

    segmentList = rebuildList->GetList();

    POS_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET),
        "MakeRebuildTarget: rebuild target segment is built, num:{}", segmentList.size());

    return rebuildCtx->FlushRebuildSegmentList(segmentList);
}

void
SegmentCtx::_BuildRebuildSegmentList(void)
{
    uint32_t numSegments = addrInfo->GetnumUserAreaSegments();
    for (uint32_t segId = 0; segId < numSegments; segId++)
    {
        if (segmentInfos[segId].GetState() != SegmentState::FREE)
        {
            rebuildList->AddToList(segId);
        }
    }
    _ResetSegmentIdInRebuilding();
}

void
SegmentCtx::_ResetSegmentIdInRebuilding(void)
{
    rebuildingSegment = UNMAP_SEGMENT;
}

int
SegmentCtx::StopRebuilding(void)
{
    uint32_t remaining = rebuildList->GetNumSegments();
    POS_TRACE_INFO(EID(ALLOCATOR_START), "StopRebuilding: remaining segments: {}", remaining);
    if (remaining == 0)
    {
        POS_TRACE_INFO(EID(ALLOCATOR_REBUILD_TARGET_SET_EMPTY),
            "Rebuild was already done or not happen");
        return -EID(ALLOCATOR_REBUILD_TARGET_SET_EMPTY);
    }

    _ResetSegmentIdInRebuilding();

    return rebuildCtx->FlushRebuildSegmentList(rebuildList->GetList());
}

uint32_t
SegmentCtx::GetRebuildTargetSegmentCount(void)
{
    return rebuildList->GetNumSegments();
}

bool
SegmentCtx::LoadRebuildList(void)
{
    rebuildList->SetList(rebuildCtx->GetList());
    POS_TRACE_INFO(EID(ALLOCATOR_REBUILD_CTX_LOADED),
        "Rebuild list is loaded. size {}", rebuildList->GetNumSegments());
    return (rebuildList->GetNumSegments() != 0);
}

std::set<SegmentId>
SegmentCtx::GetRebuildSegmentList(void)
{
    return rebuildList->GetList();
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

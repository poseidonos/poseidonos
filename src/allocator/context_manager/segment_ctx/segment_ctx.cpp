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
#include "src/qos/qos_manager.h"
namespace pos
{
SegmentCtx::SegmentCtx(TelemetryPublisher* tp_, SegmentCtxHeader* header, SegmentInfo* segmentInfo_,
    RebuildCtx* rebuildCtx_, AllocatorAddressInfo* addrInfo_, GcCtx* gcCtx_, BlockAllocationStatus* blockAllocStatus_)
: SegmentCtx(tp_, header, segmentInfo_, nullptr, rebuildCtx_, addrInfo_, gcCtx_, blockAllocStatus_)
{
}

SegmentCtx::SegmentCtx(TelemetryPublisher* tp_, SegmentCtxHeader* header, SegmentInfo* segmentInfo_,
    SegmentList* rebuildSegmentList, RebuildCtx* rebuildCtx_, AllocatorAddressInfo* addrInfo_, GcCtx* gcCtx_,
    BlockAllocationStatus* blockAllocStatus_)
: ctxDirtyVersion(0),
  ctxStoredVersion(0),
  rebuildList(rebuildSegmentList),
  rebuildingSegment(UNMAP_SEGMENT),
  initialized(false),
  addrInfo(addrInfo_),
  rebuildCtx(rebuildCtx_),
  gcCtx(gcCtx_),
  blockAllocStatus(blockAllocStatus_),
  tp(tp_)
{
    for (int state = SegmentState::START; state < SegmentState::NUM_STATES; state++)
    {
        segmentList[state] = nullptr;
    }

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

SegmentCtx::SegmentCtx(TelemetryPublisher* tp_, RebuildCtx* rebuildCtx_, AllocatorAddressInfo* info,
    GcCtx* gcCtx_, BlockAllocationStatus* blockAllocStatus_)
: SegmentCtx(tp_, nullptr, nullptr, rebuildCtx_, info, gcCtx_, blockAllocStatus_)
{
}

SegmentCtx::~SegmentCtx(void)
{
    Dispose();
}

// Only for UT
void
SegmentCtx::SetSegmentList(SegmentState state, SegmentList* list)
{
    segmentList[state] = list;
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

    for (int state = SegmentState::START; state < SegmentState::NUM_STATES; state++)
    {
        if (segmentList[state] == nullptr)
        {
            segmentList[state] = new SegmentList();
        }
    }

    if (rebuildList == nullptr)
    {
        rebuildList = new SegmentList();
    }

    _RebuildSegmentList();

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

    for (int state = SegmentState::FREE; state < SegmentState::NUM_STATES; state++)
    {
        if (segmentList[state] != nullptr)
        {
            delete segmentList[state];
            segmentList[state] = nullptr;
        }
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
    auto result = segmentInfos[segId].DecreaseValidBlockCount(cnt);

    bool segmentFreed = result.first;
    if (segmentFreed == true)
    {
        SegmentState prevState = result.second;
        bool removed = segmentList[prevState]->RemoveFromList(segId);

        POS_TRACE_DEBUG(EID(ALLOCATOR_DEBUG),
            "Segment is freed, segmentId: {}, prevState: {}, removed from the list: {}",
            segId, prevState, removed);
        _SegmentFreed(segId);
    }

    return segmentFreed;
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
    bool segmentFreed = false;

    if (occupiedStripeCount == addrInfo->GetstripesPerSegment())
    {
        // Only 1 thread reaches here
        SegmentState prevState = segmentInfos[segId].GetState();
        segmentList[prevState]->RemoveFromList(segId);

        segmentFreed = segmentInfos[segId].MoveToSsdStateOrFreeStateIfItBecomesEmpty();
        if (segmentFreed == true)
        {
            _SegmentFreed(segId);
        }
        else
        {
            segmentList[SegmentState::SSD]->AddToList(segId);
        }
    }

    return segmentFreed;
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

    _RebuildSegmentList();
}

void
SegmentCtx::_RebuildSegmentList(void)
{
    for (int state = SegmentState::START; state < SegmentState::NUM_STATES; ++state)
    {
        segmentList[state]->Reset();
    }

    for (uint32_t segId = 0; segId < addrInfo->GetnumUserAreaSegments(); ++segId)
    {
        SegmentState state = segmentInfos[segId].GetState();
        segmentList[state]->AddToList(segId);
        POS_TRACE_DEBUG(EID(ALLOCATOR_DEBUG),
            "Segment is added to the list, segmentId {}, state {}", segId, state);
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
        SegmentId segId = segmentList[SegmentState::FREE]->PopSegment();
        if (segId == UNMAP_SEGMENT)
        {
            POS_TRACE_ERROR(EID(ALLOCATOR_NO_FREE_SEGMENT),
                "[AllocateSegment] failed to allocate segment, free segment count:{}", GetNumOfFreeSegmentWoLock());
            break;
        }
        else
        {
            segmentInfos[segId].MoveToNvramState();
            segmentList[SegmentState::NVRAM]->AddToList(segId);

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
    return segmentList[SegmentState::FREE]->GetNumSegments();
}

uint64_t
SegmentCtx::GetNumOfFreeSegmentWoLock(void)
{
    return segmentList[SegmentState::FREE]->GetNumSegmentsWoLock();
}

int
SegmentCtx::GetAllocatedSegmentCount(void)
{
    return addrInfo->GetnumUserAreaSegments() - segmentList[SegmentState::FREE]->GetNumSegments();
}

SegmentId
SegmentCtx::AllocateGCVictimSegment(void)
{
    SegmentId victimSegmentId = UNMAP_SEGMENT;
    while ((victimSegmentId = _FindMostInvalidSSDSegment()) != UNMAP_SEGMENT)
    {
        bool successToSetVictim = _SetVictimSegment(victimSegmentId);
        if (successToSetVictim == true) break;
    }

    return victimSegmentId;
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

bool
SegmentCtx::_SetVictimSegment(SegmentId victimSegment)
{
    assert(victimSegment != UNMAP_SEGMENT);

    bool stateChanged = segmentInfos[victimSegment].MoveToVictimState();
    if (stateChanged == true)
    {
        // This segment is in SSD LIST or REBUILD LIST
        if (rebuildList->Contains(victimSegment) == true)
        {
            // do nothing. this segment will be return to the victim list when rebuidl is completed
        }
        else
        {
            segmentList[SegmentState::VICTIM]->AddToList(victimSegment);
        }

        _UpdateTelemetryOnVictimSegmentAllocation(victimSegment);

        POS_TRACE_INFO(EID(ALLOCATE_GC_VICTIM), "[AllocateSegment] victim segmentId:{}, free segment count:{}",
            victimSegment, GetNumOfFreeSegmentWoLock());
    }

    return stateChanged;
}

void
SegmentCtx::_UpdateTelemetryOnVictimSegmentAllocation(SegmentId victimSegment)
{
    POSMetricValue validCount;
    validCount.gauge = segmentInfos[victimSegment].GetValidBlockCount();
    tp->PublishData(TEL30010_ALCT_VICTIM_SEG_INVALID_PAGE_CNT, validCount, MT_GAUGE);

    POSMetricValue victimSegmentId;
    victimSegmentId.gauge = victimSegment;
    tp->PublishData(TEL30002_ALCT_GCVICTIM_SEG, victimSegmentId, MT_GAUGE);
}

void
SegmentCtx::_SegmentFreed(SegmentId segmentId)
{
    if (rebuildingSegment != segmentId)
    {
        if (true == rebuildList->RemoveFromList(segmentId))
        {
            POS_TRACE_INFO(EID(ALLOCATOR_TARGET_SEGMENT_FREED),
            "segmentId:{} in Rebuild Target has been Freed by GC", segmentId);

            _FlushRebuildSegmentList();
        }
    }
    else
    {
        POS_TRACE_INFO(EID(ALLOCATOR_TARGET_SEGMENT_FREED),
            "segmentId:{} is reclaimed by GC, but still under rebuilding", segmentId);
        return;
    }

    segmentList[SegmentState::FREE]->AddToList(segmentId);

    int numOfFreeSegments = GetNumOfFreeSegment();
    POS_TRACE_INFO(EID(ALLOCATOR_SEGMENT_FREED),
        "[FreeSegment] release segmentId:{} was freed, free segment count:{}", segmentId, numOfFreeSegments);

    if (MODE_URGENT_GC != gcCtx->GetCurrentGcMode(numOfFreeSegments))
    {
        blockAllocStatus->PermitUserBlockAllocation();
    }
}

void
SegmentCtx::ResetSegmentsStates(void)
{
    for (uint32_t segId = 0; segId < addrInfo->GetnumUserAreaSegments(); ++segId)
    {
        bool segmentFreed = false;
        SegmentState state = segmentInfos[segId].GetState();
        if ((state == SegmentState::SSD) || (state == SegmentState::VICTIM))
        {
            segmentFreed = segmentInfos[segId].MoveToSsdStateOrFreeStateIfItBecomesEmpty();
        }

        if (segmentFreed == true)
        {
            POS_TRACE_INFO(EID(ALLOCATOR_SEGMENT_FREED), "segmentId:{} was All Invalidated, so changed to FREE", segId);
        }
    }

    _RebuildSegmentList();
}

SegmentId
SegmentCtx::GetRebuildTargetSegment(void)
{
    SegmentId segmentId = UNMAP_SEGMENT;

    while (true)
    {
        segmentId = rebuildList->PopSegment();
        if (segmentId == UNMAP_SEGMENT)
        {
            segmentId = UINT32_MAX;
            break;
        }
        else if (segmentInfos[segmentId].GetState() == SegmentState::FREE)
        {
            segmentList[SegmentState::FREE]->AddToList(segmentId);

            POS_TRACE_INFO(EID(ALLOCATOR_TARGET_SEGMENT_FREED), "Skip Rebuilding segmentId:{}, Already Freed", segmentId);
            _FlushRebuildSegmentList();
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
SegmentCtx::_FlushRebuildSegmentList(void)
{
    auto list = rebuildList->GetList();
    if (rebuildingSegment != UNMAP_SEGMENT)
    {
        list.insert(rebuildingSegment);
    }

    return rebuildCtx->FlushRebuildSegmentList(list);
}

int
SegmentCtx::SetRebuildCompleted(SegmentId segId)
{
    if (rebuildingSegment == segId)
    {
        SegmentState state = segmentInfos[segId].GetState();
        segmentList[state]->AddToList(segId);

        _ResetSegmentIdInRebuilding();

        POS_TRACE_INFO(EID(ALLOCATOR_REBUILD_SEGMENT_COMPLETED),
            "Rebuild segment released, segmentId: {}", segId);
        return _FlushRebuildSegmentList();
    }
    else
    {
        POS_TRACE_ERROR(EID(UNKNOWN_ALLOCATOR_ERROR),
            "Rebuild is completed, but it's not rebuilding target. Completed segment {}, rebuilding segment {}",
            segId, rebuildingSegment);
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

    return _FlushRebuildSegmentList();
}

void
SegmentCtx::_BuildRebuildSegmentList(void)
{
    // New segment is not allocated while make rebuild segment target by design
    _BuildRebuildSegmentListFromTheList(SegmentState::SSD);
    _BuildRebuildSegmentListFromTheList(SegmentState::VICTIM);
    _BuildRebuildSegmentListFromTheList(SegmentState::NVRAM); // TODO(huijeong.kim) remove this

    _ResetSegmentIdInRebuilding();
}

void
SegmentCtx::_BuildRebuildSegmentListFromTheList(SegmentState state)
{
    while (segmentList[state]->GetNumSegments() != 0)
    {
        SegmentId targetSegment = segmentList[state]->PopSegment();
        if (targetSegment != UNMAP_SEGMENT)
        {
            POS_TRACE_DEBUG(EID(ALLOCATOR_DEBUG),
                "Segment is added to the rebuild target, segmentId {}, state {}",
                targetSegment, segmentInfos[targetSegment].GetState());
            rebuildList->AddToList(targetSegment);
        }
    }
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

    while (rebuildList->GetNumSegments() != 0)
    {
        SegmentId segmentId = rebuildList->PopSegment();
        if (segmentId != UNMAP_SEGMENT)
        {
            SegmentState state = segmentInfos[segmentId].GetState();
            segmentList[state]->AddToList(segmentId);
        }
    }

    _ResetSegmentIdInRebuilding();

    return _FlushRebuildSegmentList();
}

uint32_t
SegmentCtx::GetRebuildTargetSegmentCount(void)
{
    return rebuildList->GetNumSegments();
}

bool
SegmentCtx::LoadRebuildList(void)
{
    auto list = rebuildCtx->GetList();

    for (auto it = list.begin(); it != list.end(); it++)
    {
        SegmentId segmentId = *it;
        SegmentState state = segmentInfos[segmentId].GetState();
        bool removed = segmentList[state]->RemoveFromList(segmentId);
        assert(removed == true);

        rebuildList->AddToList(segmentId);
    }

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

void
SegmentCtx::UpdateGcFreeSegment(uint32_t arrayId)
{
    int numFreeSegments = GetNumOfFreeSegment();
    QosManagerSingleton::Instance()->SetGcFreeSegment(numFreeSegments, arrayId);
}

} // namespace pos

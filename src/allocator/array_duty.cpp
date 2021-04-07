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

#include "array_duty.h"

#include "common_duty.h"
#include "io_duty.h"
#include "main_duty.h"
#include "src/mapper/mapper.h"
#include "src/volume/volume_manager.h"

namespace ibofos
{
ArrayDuty::ArrayDuty(AllocatorAddressInfo& addrInfoI,
    AllocatorMetaArchive* metaI, CommonDuty* commonDutyI, MainDuty* mainDutyI,
    IoDuty* ioDutyI)
: addrInfo(addrInfoI),
  meta(metaI),
  commonDuty(commonDutyI),
  mainDuty(mainDutyI),
  ioDuty(ioDutyI)
{
}

ArrayDuty::~ArrayDuty(void)
{
}

int
ArrayDuty::PrepareRebuild(void)
{
    std::vector<Stripe*> stripesToFlush;
    std::vector<StripeId> vsidToCheckFlushDone;

    IBOF_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "Start @PrepareRebuild()");
    // Block allocation
    ioDuty->TurnOffBlkAllocation();

    // Check rebuildTargetSegments data structure
    int ret = _MakeRebuildTarget();
    if (ret <= NO_REBUILD_TARGET_USER_SEGMENT)
    {
        ioDuty->TurnOnBlkAllocation();
        return ret;
    }
    IBOF_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "MakeRebuildTarget Done @PrepareRebuild()");

    // Let nextSsdLsid point non-rebuild target segment
    ret = _SetNextSsdLsid();
    if (ret < 0)
    {
        ioDuty->TurnOnBlkAllocation();
        return ret;
    }
    IBOF_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "SetNextSsdLsid Done @PrepareRebuild()");

    // Online stripes beyond target segments should be flushed
    ret = _FlushOnlineStripes(vsidToCheckFlushDone);
    if (ret < 0)
    {
        ioDuty->TurnOnBlkAllocation();
        return ret;
    }
    ret = commonDuty->CheckAllActiveStripes(stripesToFlush, vsidToCheckFlushDone);
    mainDuty->FinalizeWriteIO(stripesToFlush, vsidToCheckFlushDone);
    IBOF_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "Stripes Flush Done @PrepareRebuild()");

    // Unblock allocation
    ioDuty->TurnOnBlkAllocation();
    IBOF_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "End @PrepareRebuild()");

    return 0;
}

int
ArrayDuty::StopRebuilding(void)
{
    std::unique_lock<std::mutex> lock(meta->GetallocatorMetaLock());
    IBOF_TRACE_INFO(EID(ALLOCATOR_START), "@StopRebuilding");

    if (meta->GetTargetSegmentCnt() == 0)
    {
        IBOF_TRACE_INFO(EID(ALLOCATOR_REBUILD_TARGET_SET_EMPTY),
            "Rebuild was already done or not happen");
        return -EID(ALLOCATOR_REBUILD_TARGET_SET_EMPTY);
    }

    // Clear rebuildTargetSegments
    meta->ClearRebuildTargetSegments();
    meta->StoreRebuildSegment();

    return 0;
}

bool
ArrayDuty::NeedRebuildAgain(void)
{
    return meta->GetNeedRebuildCont();
}

SegmentId
ArrayDuty::GetRebuildTargetSegment(void)
{
    std::unique_lock<std::mutex> lock(meta->GetallocatorMetaLock());
    IBOF_TRACE_INFO(EID(ALLOCATOR_START), "@GetRebuildTargetSegment");

    if (meta->GetTargetSegmentCnt() == 0)
    {
        return UINT32_MAX;
    }

    SegmentId segmentId = UINT32_MAX;
    while (meta->IsRebuidTargetSegmentsEmpty() == false)
    {
        auto iter = meta->RebuildTargetSegmentsBegin();
        segmentId = *iter;

        SegmentInfo& segmentInfo = meta->GetSegmentInfo(segmentId);
        if (segmentInfo.Getstate() ==
            SegmentState::FREE) // This segment had been freed by GC
        {
            IBOF_TRACE_INFO(EID(ALLOCATOR_TARGET_SEGMENT_FREED),
                "This segmentId:{} was target but seemed to be freed by GC", segmentId);
            meta->EraseRebuildTargetSegments(iter);
            segmentId = UINT32_MAX;
            continue;
        }

        IBOF_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET),
            "segmentId:{} is going to be rebuilt", segmentId);
        break;
    }

    return segmentId;
}

int
ArrayDuty::ReleaseRebuildSegment(SegmentId segmentId)
{
    std::unique_lock<std::mutex> lock(meta->GetallocatorMetaLock());
    IBOF_TRACE_INFO(EID(ALLOCATOR_START), "@ReleaseRebuildSegment");

    auto iter = meta->FindRebuildTargetSegment(segmentId);
    if (iter == meta->RebuildTargetSegmentsEnd())
    {
        IBOF_TRACE_INFO(EID(ALLOCATOR_TARGET_SEGMENT_FREED),
            "There is no segmentId:{} in rebuildTargetSegments seemed to be freed by GC", segmentId);
        return 0;
    }

    IBOF_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET),
        "segmentId:{} Rebuild Done!", segmentId);

    // Delete segmentId in rebuildTargetSegments
    meta->EraseRebuildTargetSegments(iter);
    meta->StoreRebuildSegment();

    return 0;
}

int
ArrayDuty::_MakeRebuildTarget(void)
{
    IBOF_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "@MakeRebuildTarget()");

    if (meta->IsRebuidTargetSegmentsEmpty() == false)
    {
        IBOF_TRACE_WARN(EID(ALLOCATOR_REBUILD_TARGET_SET_NOT_EMPTY),
            "rebuildTargetSegments is NOT empty!");
        for (auto it = meta->RebuildTargetSegmentsBegin();
             it != meta->RebuildTargetSegmentsEnd(); ++it)
        {
            IBOF_TRACE_WARN(EID(ALLOCATOR_REBUILD_TARGET_SET_NOT_EMPTY),
                "residue was segmentId:{}", *it);
        }
        meta->ClearRebuildTargetSegments();
    }

    // Pick non-free segments and make rebuildTargetSegments
    SegmentId segmentId = 0;
    while (true)
    {
        segmentId = meta->segmentBitmap->FindFirstSetBit(segmentId);
        if (meta->segmentBitmap->IsValidBit(segmentId) == false)
        {
            break;
        }

        auto pr = meta->EmplaceRebuildTargetSegment(segmentId);
        IBOF_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET),
            "segmentId:{} is inserted as target to rebuild", segmentId);
        if (pr.second == false)
        {
            IBOF_TRACE_ERROR(EID(ALLOCATOR_MAKE_REBUILD_TARGET_FAILURE),
                "segmentId:{} is already in set", segmentId);
            return -EID(ALLOCATOR_MAKE_REBUILD_TARGET_FAILURE);
        }
        ++segmentId;
    }

    meta->StoreRebuildSegment(); // Store target segments info to MFS
    return meta->GetTargetSegmentCnt();
}

int
ArrayDuty::_SetNextSsdLsid(void)
{
    std::unique_lock<std::mutex> lock(meta->GetallocatorMetaLock());
    IBOF_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "@SetNextSsdLsid");

    SegmentId newSegmentId = ioDuty->AllocateUserDataSegmentId();
    if (newSegmentId == UNMAP_SEGMENT)
    {
        IBOF_TRACE_ERROR(EID(ALLOCATOR_NO_FREE_SEGMENT), "Free segmentId exhausted");
        return -EID(ALLOCATOR_NO_FREE_SEGMENT);
    }

    meta->SetPrevSsdLsid(meta->GetCurrentSsdLsid());
    meta->SetCurrentSsdLsid(newSegmentId * addrInfo.GetstripesPerSegment());
    ioDuty->UsedSegmentStateChange(newSegmentId, SegmentState::NVRAM);

    return 0;
}

int
ArrayDuty::_FlushOnlineStripes(std::vector<StripeId>& vsidToCheckFlushDone)
{
    // Flush Online Stripes Beyond Target Segment
    Mapper& mapper = *MapperSingleton::Instance();

    for (auto it = meta->RebuildTargetSegmentsBegin();
         it != meta->RebuildTargetSegmentsEnd(); ++it)
    {
        StripeId startVsid = *it * addrInfo.GetstripesPerSegment();
        StripeId endVsid = startVsid + addrInfo.GetstripesPerSegment();
        for (StripeId vsid = startVsid; vsid < endVsid; ++vsid)
        {
            StripeAddr lsa = mapper.GetLSA(vsid);
            if (lsa.stripeId != UNMAP_STRIPE && mapper.IsInWriteBufferArea(lsa))
            {
                Stripe* stripe = commonDuty->GetStripe(lsa);
                if (stripe != nullptr && stripe->IsFinished() == false && stripe->GetBlksRemaining() == 0)
                {
                    vsidToCheckFlushDone.push_back(stripe->GetVsid());
                }
            }
        }
    }

    return 0;
}

} // namespace ibofos

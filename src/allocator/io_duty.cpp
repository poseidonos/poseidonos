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

#include "io_duty.h"

#include "active_stripe_index_info.h"
#include "allocator_address_info.h"
#include "common_duty.h"
#include "gc_duty.h"
#include "src/logger/logger.h"
#include "src/mapper/mapper.h"
#include "stripe.h"
#if defined QOS_ENABLED_BE
#include "src/qos/qos_manager.h"
#endif

namespace ibofos
{
IoDuty::IoDuty(AllocatorAddressInfo& addrInfoI, AllocatorMetaArchive* metaI,
    GcDuty* gcDutyI, CommonDuty* commonDutyI)
: addrInfo(addrInfoI),
  meta(metaI),
  commonDuty(commonDutyI),
  gcDuty(gcDutyI),
  blockWholeBlockAllocation(false)
{
}

IoDuty::~IoDuty(void)
{
}

VirtualBlks
IoDuty::AllocateWriteBufferBlks(VolumeId volumeId, uint32_t numBlks)
{
    VirtualBlks allocatedBlks;
    if (gcDuty->IsBlockedForUserSegmentAlloc() || blockWholeBlockAllocation)
    {
        allocatedBlks.startVsa = UNMAP_VSA;
    }
    else
    {
        ActiveStripeTailArrIdxInfo info = {.volumeId = volumeId, .isGc = false};
        allocatedBlks = _AllocateBlks(info.GetActiveStripeTailArrIdx(), numBlks);
    }
    return allocatedBlks;
}

VirtualBlks
IoDuty::AllocateGcBlk(VolumeId volumeId, uint32_t numBlks)
{
    ActiveStripeTailArrIdxInfo info = {.volumeId = volumeId, .isGc = true};
    ASTailArrayIdx asTailArrayIdx = info.GetActiveStripeTailArrIdx();
    VirtualBlks allocatedBlks;

    if (blockWholeBlockAllocation)
    {
        allocatedBlks.startVsa = UNMAP_VSA;
    }
    else
    {
        allocatedBlks = _AllocateBlks(asTailArrayIdx, numBlks);
    }
    return allocatedBlks;
}

StripeId
IoDuty::AllocateUserDataStripeId(StripeId vsid)
{
    return vsid;
}

VirtualBlks
IoDuty::AllocateWriteBufferBlksFromNewStripe(ASTailArrayIdx asTailArrayIdx,
    StripeId vsid, int numBlks)
{
    VirtualBlkAddr curVsa = {.stripeId = vsid, .offset = 0};

    VirtualBlks allocatedBlks;
    allocatedBlks.startVsa = curVsa;

    if (_IsValidOffset(numBlks))
    {
        allocatedBlks.numBlks = numBlks;
    }
    else
    {
        allocatedBlks.numBlks = addrInfo.GetblksPerStripe();
    }
    curVsa.offset = allocatedBlks.numBlks;

    // Temporally no lock required, as AllocateBlks and this function cannot be executed in parallel
    // TODO(jk.man.kim): add or move lock to wbuf tail manager
    meta->SetActiveStripeTail(asTailArrayIdx, curVsa);

    return allocatedBlks;
}

SegmentId
IoDuty::AllocateUserDataSegmentId(void)
{
    SegmentId segmentId = meta->segmentBitmap->SetFirstZeroBit();

    // This 'segmentId' should not be a target of rebuilding
    while (meta->segmentBitmap->IsValidBit(segmentId) && (meta->FindRebuildTargetSegment(segmentId) != meta->RebuildTargetSegmentsEnd()))
    {
        IBOF_TRACE_DEBUG(EID(ALLOCATOR_REBUILDING_SEGMENT),
            "segmentId:{} is already rebuild target!", segmentId);
        meta->segmentBitmap->ClearBit(segmentId);
        ++segmentId;
        segmentId = meta->segmentBitmap->SetFirstZeroBit(segmentId);
    }

    // In case of all segments are used
    if (meta->segmentBitmap->IsValidBit(segmentId) == false)
    {
        IBOF_TRACE_ERROR(EID(ALLOCATOR_NO_FREE_SEGMENT),
            "Free segmentId exhausted, segmentId:{}", segmentId);
        return UNMAP_SEGMENT;
    }

    IBOF_TRACE_INFO(EID(ALLOCATOR_START), "segmentId:{} @AllocateUserDataSegmentId",
        segmentId);
    return segmentId;
}

StripeId
IoDuty::_AllocateUserDataStripeIdInternal(bool isUserStripeAlloc)
{
    std::lock_guard<std::mutex> lock(meta->GetallocatorMetaLock());
    meta->SetPrevSsdLsid(meta->GetCurrentSsdLsid());
    StripeId ssdLsid = meta->GetCurrentSsdLsid() + 1;

    if (_IsSegmentFull(ssdLsid))
    {
        uint32_t freeSegments = gcDuty->GetNumOfFreeUserDataSegment();
        if (gcDuty->GetUrgentThreshold() >= freeSegments)
        {
            gcDuty->SetUpBlockSegmentAllocForUser(true);
            if (isUserStripeAlloc)
            {
                return UNMAP_STRIPE;
            }
        }

        SegmentId segmentId = AllocateUserDataSegmentId();
        if (segmentId == UNMAP_SEGMENT)
        {
            // Under Rebuiling...
            if (meta->IsRebuidTargetSegmentsEmpty() == false)
            {
                IBOF_TRACE_INFO(EID(ALLOCATOR_REBUILDING_SEGMENT),
                    "Couldn't Allocate a SegmentId, seems Under Rebuiling");
                return UNMAP_STRIPE;
            }
            else
            {
                assert(false);
            }
        }
        ssdLsid = segmentId * addrInfo.GetstripesPerSegment();
    }

    meta->SetCurrentSsdLsid(ssdLsid);

    return ssdLsid;
}

void
IoDuty::_RollBackStripeIdAllocation(StripeId wbLsid, StripeId arrayLsid)
{
    if (wbLsid != UINT32_MAX)
    {
        meta->wbLsidBitmap->ClearBit(wbLsid);
    }

    if (arrayLsid != UINT32_MAX)
    {
        if (_IsSegmentFull(arrayLsid))
        {
            SegmentId SegmentIdToClear = arrayLsid / addrInfo.GetstripesPerSegment();
            meta->segmentBitmap->ClearBit(SegmentIdToClear);
            UsedSegmentStateChange(SegmentIdToClear, SegmentState::FREE);
        }
        meta->SetCurrentSsdLsid(meta->GetPrevSsdLsid());
    }
}

void
IoDuty::UsedSegmentStateChange(SegmentId segmentId, SegmentState to)
{
    SegmentInfo& segmentInfo = meta->GetSegmentInfo(segmentId);

    SegmentState from = segmentInfo.Getstate();
    if (from == to)
    {
        return;
    }
    segmentInfo.Setstate(to);
}

void
IoDuty::TurnOffBlkAllocation(void)
{
    lockBlockAlloc.lock();
    blockWholeBlockAllocation = true;
}

void
IoDuty::TurnOnBlkAllocation(void)
{
    blockWholeBlockAllocation = false;
    lockBlockAlloc.unlock();
}

VirtualBlks
IoDuty::_AllocateBlks(ASTailArrayIdx asTailArrayIdx, int numBlks)
{
    assert(numBlks != 0);
    std::unique_lock<std::mutex> volLock(meta->GetActiveStripeTailLock(
        asTailArrayIdx));
    VirtualBlks allocatedBlks;
    VirtualBlkAddr curVsa = meta->GetActiveStripeTail(asTailArrayIdx);

    if (_IsStripeFull(curVsa) || IsUnMapStripe(curVsa.stripeId))
    {
        StripeId newVsid = UNMAP_STRIPE;
        int ret = _AllocateStripe(asTailArrayIdx, newVsid);
        if (likely(ret == 0))
        {
            allocatedBlks = AllocateWriteBufferBlksFromNewStripe(asTailArrayIdx, newVsid,
                numBlks);
        }
        else
        {
            allocatedBlks.startVsa = UNMAP_VSA;
            allocatedBlks.numBlks = UINT32_MAX;
            return allocatedBlks;
        }
    }
    else if (_IsValidOffset(curVsa.offset + numBlks - 1) == false)
    {
        allocatedBlks.startVsa = curVsa;
        allocatedBlks.numBlks = addrInfo.GetblksPerStripe() - curVsa.offset;

        VirtualBlkAddr vsa = {.stripeId = curVsa.stripeId,
            .offset = addrInfo.GetblksPerStripe()};
        meta->SetActiveStripeTail(asTailArrayIdx, vsa);
    }
    else
    {
        allocatedBlks.startVsa = curVsa;
        allocatedBlks.numBlks = numBlks;

        VirtualBlkAddr vsa = {.stripeId = curVsa.stripeId,
            .offset = curVsa.offset + numBlks};
        meta->SetActiveStripeTail(asTailArrayIdx, vsa);
    }

    return allocatedBlks;
}

int
IoDuty::_AllocateStripe(ASTailArrayIdx asTailArrayIdx, StripeId& vsid)
{
    // 1. WriteBuffer Logical StripeId Allocation
    StripeId wbLsid = _AllocateWriteBufferStripeId();
    if (wbLsid == UNMAP_STRIPE)
    {
        return -EID(ALLOCATOR_CANNOT_ALLOCATE_STRIPE);
    }

    // 2. SSD Logical StripeId Allocation
    bool isUserStripeAlloc = _IsUserStripeAllocation(asTailArrayIdx);
    StripeId arrayLsid = _AllocateUserDataStripeIdInternal(isUserStripeAlloc);
    if (IsUnMapStripe(arrayLsid))
    {
        std::lock_guard<std::mutex> lock(meta->GetallocatorMetaLock());
        _RollBackStripeIdAllocation(wbLsid, UINT32_MAX);
        return -EID(ALLOCATOR_CANNOT_ALLOCATE_STRIPE);
    }
    // If arrayLsid is the front and first stripe of Segment
    if (_IsSegmentFull(arrayLsid))
    {
        SegmentId segId = arrayLsid / addrInfo.GetstripesPerSegment();
        UsedSegmentStateChange(segId, SegmentState::NVRAM);
    }
    StripeId newVsid = arrayLsid;

    // 3. Get Stripe object for wbLsid and link it with reverse map for vsid
    Stripe* stripe = commonDuty->GetStripe(wbLsid);
    stripe->Assign(newVsid, wbLsid);
    stripe->SetAsTailArrayIdx(asTailArrayIdx);
    Mapper& mapper = *MapperSingleton::Instance();
    if (unlikely(mapper.LinkReverseMap(stripe, wbLsid, newVsid) < 0))
    {
        std::lock_guard<std::mutex> lock(meta->GetallocatorMetaLock());
        _RollBackStripeIdAllocation(wbLsid, arrayLsid);
        return -EID(ALLOCATOR_CANNOT_LINK_REVERSE_MAP);
    }

    // 4. Update the stripe map
    mapper.UpdateStripeMap(newVsid, wbLsid, IN_WRITE_BUFFER_AREA);

    vsid = newVsid;
    return 0;
}

StripeId
IoDuty::_AllocateWriteBufferStripeId(void)
{
    std::lock_guard<std::mutex> lock(meta->GetallocatorMetaLock());
    StripeId wbLsid = meta->wbLsidBitmap->SetFirstZeroBit();
    if (meta->wbLsidBitmap->IsValidBit(wbLsid) == false)
    {
        // IBOF_TRACE_INFO(ALLOCATOR_NO_FREE_WB_STRIPE, "WB stripeId exhausted, wbLsid:{}", wbLsid);
        return UNMAP_STRIPE;
    }

#if defined QOS_ENABLED_BE
    QosManagerSingleton::Instance()->IncreaseUsedStripeCnt();
#endif
    return wbLsid;
}

} // namespace ibofos

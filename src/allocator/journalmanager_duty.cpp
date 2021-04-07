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

#include "journalmanager_duty.h"

#include "active_stripe_index_info.h"
#include "common_duty.h"
#include "main_duty.h"
#include "src/include/meta_const.h"
#include "src/io/general_io/io_submit_handler.h"
#include "src/mapper/mapper.h"

namespace ibofos
{
JournalManagerDuty::JournalManagerDuty(AllocatorAddressInfo& addrInfoI,
    AllocatorMetaArchive* metaI, CommonDuty* commonDutyI, MainDuty* mainDutyI,
    IoDuty* ioDutyI)
: pendingFullStripes(nullptr),
  addrInfo(addrInfoI),
  meta(metaI),
  commonDuty(commonDutyI),
  ioDuty(ioDutyI),
  mainDuty(mainDutyI)
{
}

JournalManagerDuty::~JournalManagerDuty(void)
{
}

int
JournalManagerDuty::FlushMetadata(EventSmartPtr callback)
{
    char* data;
    {
        std::lock_guard<std::mutex> lock(meta->GetallocatorMetaLock());
        data = meta->GetCopiedMetaBuffer();
    }

    for (int index = 0; index < ACTIVE_STRIPE_TAIL_ARRAYLEN; index++)
    {
        std::unique_lock<std::mutex> volLock(meta->GetActiveStripeTailLock(index));
        meta->CopyWbufTail(data, index);
    }

    return meta->Flush(data, callback);
}

int
JournalManagerDuty::FlushFullActiveStripes(void)
{
    int ret = 0;

    if (nullptr != pendingFullStripes)
    {
        for (Stripe* stripe : *pendingFullStripes)
        {
            ret = mainDuty->RequestStripeFlush(*stripe);
            if (ret < 0)
            {
                return ret;
            }

            IBOF_TRACE_DEBUG(EID(ALLOCATOR_TRIGGER_FLUSH), "Request stripe flush, vsid {} lsid {} remaining {}",
                stripe->GetVsid(), stripe->GetWbLsid(), stripe->GetBlksRemaining());
        }

        delete pendingFullStripes;
        pendingFullStripes = nullptr;
    }

    return ret;
}

int
JournalManagerDuty::FlushStripe(VolumeId volumeId, StripeId wbLsid, VirtualBlkAddr tailVsa)
{
    int ret = 0;

    Stripe* stripe;
    ret = _ReconstructActiveStripe(tailVsa.stripeId, wbLsid, tailVsa.offset, stripe);
    if (ret < 0)
    {
        return ret;
    }

    ret = _ReconstructReverseMap(volumeId, stripe, tailVsa.offset);
    if (ret == 0)
    {
        stripe = commonDuty->FinishStripe(wbLsid, tailVsa);
    }

    return ret;
}

void
JournalManagerDuty::ReplaySsdLsid(StripeId currentSsdLsid)
{
    meta->SetCurrentSsdLsid(currentSsdLsid);
}

void
JournalManagerDuty::ReplayStripeAllocation(StripeId vsid, StripeId wbLsid)
{
    meta->wbLsidBitmap->SetBit(wbLsid);

    StripeId userLsid = ioDuty->AllocateUserDataStripeId(vsid);
    if (_IsFirstStripeOfSegment(userLsid))
    {
        SegmentId segId = userLsid / addrInfo.GetstripesPerSegment();
        meta->segmentBitmap->SetBit(segId);
    }
}

void
JournalManagerDuty::ReplayStripeFlushed(StripeId wbLsid)
{
    meta->wbLsidBitmap->ClearBit(wbLsid);
}

std::vector<VirtualBlkAddr>
JournalManagerDuty::GetActiveStripeTail(void)
{
    std::vector<VirtualBlkAddr> asTails;
    for (ASTailArrayIdx asTailArrayIdx = 0; asTailArrayIdx < ACTIVE_STRIPE_TAIL_ARRAYLEN; asTailArrayIdx++)
    {
        asTails.push_back(meta->GetActiveStripeTail(asTailArrayIdx));
    }
    return asTails;
}

void
JournalManagerDuty::ReplaySegmentAllocation(StripeId userLsid)
{
    if (userLsid % addrInfo.GetstripesPerSegment() == 0)
    {
        SegmentId segmentId = userLsid / addrInfo.GetstripesPerSegment();
        SegmentInfo& segmentInfo = meta->GetSegmentInfo(segmentId);
        if (segmentInfo.Getstate() != SegmentState::FREE)
        {
            IBOF_TRACE_DEBUG(EID(ALLOCATOR_REPLAY_SEGMENT_STATUS),
                "This segmentId:{} is already released", segmentId);
        }
        else
        {
            ioDuty->UsedSegmentStateChange(segmentId, SegmentState::NVRAM);
            IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::JOURNAL_REPLAY_STATUS,
                "Segment {} is allocated", segmentId);
        }
    }
}

void
JournalManagerDuty::ResetActiveStripeTail(int index)
{
    meta->SetActiveStripeTail(index, UNMAP_VSA);
}

int
JournalManagerDuty::RestoreActiveStripeTail(int index, VirtualBlkAddr tail, StripeId wbLsid)
{
    int ret = 0;

    meta->SetActiveStripeTail(index, tail);

    Stripe* stripe;
    ret = _ReconstructActiveStripe(tail.stripeId, wbLsid, tail.offset, stripe);
    if (ret < 0)
    {
        return ret;
    }

    VolumeId volumeId = ActiveStripeTailArrIdxInfo::GetVolumeId(index);
    ret = _ReconstructReverseMap(volumeId, stripe, tail.offset);
    if (ret < 0)
    {
        return ret;
    }

    return ret;
}

int
JournalManagerDuty::_ReconstructActiveStripe(StripeId vsid, StripeId wbLsid,
    uint64_t blockCount, Stripe*& stripe)
{
    if (0 == blockCount)
    {
        IBOF_TRACE_ERROR(EID(WRONG_BLOCK_COUNT), "Wrong blockCount:{}", blockCount);
        return -EID(WRONG_BLOCK_COUNT);
    }

    stripe = commonDuty->GetStripe(wbLsid);
    stripe->Assign(vsid, wbLsid);

    uint32_t remainingBlks = stripe->DecreseBlksRemaining(blockCount);
    if (remainingBlks == 0)
    {
        if (nullptr == pendingFullStripes)
        {
            pendingFullStripes = new StripeVec;
        }

        pendingFullStripes->push_back(stripe);

        IBOF_TRACE_DEBUG(EID(ALLOCATOR_REPLAYED_STRIPE_IS_FULL),
            "Stripe (vsid {}, wbLsid {}) is waiting to be flushed", vsid, wbLsid);
    }

    IBOF_TRACE_DEBUG(EID(ALLOCATOR_RECONSTRUCT_STRIPE),
        "Stripe (vsid {}, wbLsid {}, blockCount {}, remainingBlks {}) is reconstructed",
        vsid, wbLsid, blockCount, remainingBlks);

    return 0;
}

int
JournalManagerDuty::_ReconstructReverseMap(VolumeId volumeId, Stripe* stripe, uint64_t blockCount)
{
    int ret = 0;
    Mapper& mapper = *MapperSingleton::Instance();

    ret = mapper.LinkReverseMap(stripe, stripe->GetWbLsid(), stripe->GetVsid());
    if (unlikely(ret < 0))
    {
        return ret;
    }

    ret = stripe->ReconstructReverseMap(volumeId, blockCount);
    if (ret < 0)
    {
        IBOF_TRACE_INFO(EID(REVMAP_RECONSTRUCT_NOT_FOUND_RBA),
            "There was no vsa map entry for some blocks");
    }

    return ret;
}

} // namespace ibofos

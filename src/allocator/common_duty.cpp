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

#include "common_duty.h"

#include <fstream>
#include <vector>

#include "gc_duty.h"
#include "segment_info.h"
#include "src/array/free_buffer_pool.h"
#include "src/include/meta_const.h"
#include "src/io/backend_io/flush_read_submission.h"
#include "src/mapper/mapper.h"
#include "stripe.h"

namespace ibofos
{
CommonDuty::CommonDuty(AllocatorAddressInfo& addrInfoI,
    AllocatorMetaArchive* metaI, GcDuty* gcDutyI)
: addrInfo(addrInfoI),
  meta(metaI),
  gcDuty(gcDutyI)
{
    uint32_t totalNvmStripes = addrInfo.GetnumWbStripes();
    uint32_t chunksPerStripe = addrInfo.GetchunksPerStripe();

    stripeBufferPool = new FreeBufferPool(totalNvmStripes * chunksPerStripe,
        CHUNK_SIZE);

    for (uint32_t stripeCnt = 0; stripeCnt < totalNvmStripes; ++stripeCnt)
    {
        Stripe* stripe = new Stripe;

        for (uint32_t chunkCnt = 0; chunkCnt < chunksPerStripe; ++chunkCnt)
        {
            void* buffer = stripeBufferPool->GetBuffer();
            stripe->AddDataBuffer(buffer);
        }
        wbStripeArray.push_back(stripe);
    }
}

CommonDuty::~CommonDuty()
{
    for (auto& stripeToClear : wbStripeArray)
    {
        delete stripeToClear;
        stripeToClear = nullptr;
    }

    delete stripeBufferPool;
    stripeBufferPool = nullptr;
}

int
CommonDuty::Store(void)
{
    // No lock: assume that this is never called during normal IO
    return meta->StoreSync();
}

Stripe*
CommonDuty::GetStripe(StripeAddr& lsa)
{
    Mapper& mapper = *MapperSingleton::Instance();
    if (mapper.IsInUserDataArea(lsa))
    {
        return nullptr;
    }
    return wbStripeArray[lsa.stripeId];
}

Stripe*
CommonDuty::GetStripe(StripeId wbLsid)
{
    return wbStripeArray[wbLsid];
}

void
CommonDuty::InvalidateBlks(VirtualBlks blks)
{
    SegmentId segId = blks.startVsa.stripeId / addrInfo.GetstripesPerSegment();
    _IncreaseInvCount(segId, blks.numBlks);
}

void
CommonDuty::TryToResetSegmentState(StripeId lsid, bool replay)
{
    SegmentId segmentId = lsid / STRIPES_PER_SEGMENT;
    SegmentInfo& segmentInfo = meta->GetSegmentInfo(segmentId);

    if (segmentInfo.DecreaseOccupiedStripeCount() == 0)
    {
        gcDuty->FreeUserDataSegmentId(segmentId);
    }
}

void
CommonDuty::FreeUserDataStripeId(StripeId lsid)
{
    // VSID is same with LSID when it is a user data stripe
}

bool
CommonDuty::IsValidWriteBufferStripeId(StripeId lsid)
{
    return meta->wbLsidBitmap->IsSetBit(lsid);
}

bool
CommonDuty::IsValidUserDataSegmentId(SegmentId segId)
{
    return meta->segmentBitmap->IsSetBit(segId);
}

WbStripeIter
CommonDuty::GetwbStripeArrayBegin()
{
    return wbStripeArray.begin();
}

WbStripeIter
CommonDuty::GetwbStripeArrayEnd()
{
    return wbStripeArray.end();
}

int
CommonDuty::CheckAllActiveStripes(std::vector<Stripe*>& stripesToFlush,
    std::vector<StripeId>& vsidToCheckFlushDone)
{
    for (uint32_t volumeId = 0; volumeId < MAX_VOLUME_COUNT; ++volumeId)
    {
        PickActiveStripe(volumeId, stripesToFlush, vsidToCheckFlushDone);
    }

    return 0;
}

void
CommonDuty::_IncreaseInvCount(SegmentId segId, int count)
{
    uint32_t increasedCount =
        meta->GetSegmentInfo(segId).IncreaseinValidBlockCount(count);
    assert(addrInfo.GetblksPerSegment() >= increasedCount);
}

void
CommonDuty::PickActiveStripe(VolumeId volumeId,
    std::vector<Stripe*>& stripesToFlush,
    std::vector<StripeId>& vsidToCheckFlushDone)
{
    Stripe* activeStripe = nullptr;

    for (ASTailArrayIdx index = volumeId; index < ACTIVE_STRIPE_TAIL_ARRAYLEN;
         index += MAX_VOLUME_COUNT)
    {
        activeStripe = _FinishActiveStripe(index);
        if (activeStripe != nullptr)
        {
            IBOF_TRACE_INFO(EID(PICKUP_ACTIVE_STRIPE),
                "Picked Active Stripe: index:{}  wbLsid:{}  vsid:{}  remaining:{}", index,
                activeStripe->GetWbLsid(), activeStripe->GetVsid(),
                activeStripe->GetBlksRemaining());
            stripesToFlush.push_back(activeStripe);
            vsidToCheckFlushDone.push_back(activeStripe->GetVsid());
        }
    }
}

Stripe*
CommonDuty::_FinishActiveStripe(ASTailArrayIdx index)
{
    VirtualBlks remainingVsaRange = _AllocateRemainingBlocks(index);
    return _FinishRemainingBlocks(remainingVsaRange);
}

Stripe*
CommonDuty::FinishStripe(StripeId wbLsid, VirtualBlkAddr tail)
{
    VirtualBlks remainingVsaRange = _AllocateRemainingBlocks(tail);
    return _FinishRemainingBlocks(remainingVsaRange);
}

Stripe*
CommonDuty::_FinishRemainingBlocks(VirtualBlks remainingVsaRange)
{
    Stripe* activeStripe = nullptr;
    StripeId vsid = remainingVsaRange.startVsa.stripeId;

    if (vsid != UNMAP_STRIPE)
    {
        InvalidateBlks(remainingVsaRange);
        StripeAddr lsa = MapperSingleton::Instance()->GetLSA(vsid);
        activeStripe = wbStripeArray[lsa.stripeId];

        uint32_t startBlock = remainingVsaRange.startVsa.offset;
        uint32_t lastBlock = startBlock + remainingVsaRange.numBlks - 1;
        for (uint32_t block = startBlock; block <= lastBlock; ++block)
        {
            activeStripe->UpdateReverseMap(block, INVALID_RBA, UINT32_MAX);
        }

        uint32_t remain =
            activeStripe->DecreseBlksRemaining(remainingVsaRange.numBlks);
        if (remain == 0)
        {
            IBOF_TRACE_DEBUG(EID(ALLOCATOR_TRIGGER_FLUSH),
                "Flush stripe (vsid {})", vsid);

            EventSmartPtr event(new FlushReadSubmission(activeStripe));
            int ret = activeStripe->Flush(event);
            if (ret != 0)
            {
                IBOF_TRACE_DEBUG(EID(ALLOCATOR_TRIGGER_FLUSH),
                    "request stripe flush failed");
            }
        }
    }

    return activeStripe;
}

VirtualBlks
CommonDuty::_AllocateRemainingBlocks(ASTailArrayIdx index)
{
    std::unique_lock<std::mutex> lock(meta->GetActiveStripeTailLock(index));
    VirtualBlkAddr tail = meta->GetActiveStripeTail(index);

    VirtualBlks remainingBlocks = _AllocateRemainingBlocks(tail);
    meta->SetActiveStripeTail(index, UNMAP_VSA);

    return remainingBlocks;
}

VirtualBlks
CommonDuty::_AllocateRemainingBlocks(VirtualBlkAddr tail)
{
    VirtualBlks remainingBlks;

    // Nothing to do, 'ActiveStripeTailArray Index' is unused or already done
    if (UNMAP_OFFSET == tail.offset)
    {
        remainingBlks.startVsa = UNMAP_VSA;
        remainingBlks.numBlks = 0;
        return remainingBlks;
    }
    else if (tail.offset > addrInfo.GetblksPerStripe())
    {
        IBOF_TRACE_ERROR(EID(PICKUP_ACTIVE_STRIPE),
            "offsetInTail:{} > blksPerStirpe:{}", tail.offset,
            addrInfo.GetblksPerStripe());
        assert(false);
    }

    remainingBlks.numBlks = addrInfo.GetblksPerStripe() - tail.offset;
    // if remaining blocks is empty, stripe wouldn't be in StripePool
    if (remainingBlks.numBlks == 0)
    {
        remainingBlks.startVsa = UNMAP_VSA;
    }
    else
    {
        remainingBlks.startVsa = tail;
    }

    return remainingBlks;
}

} // namespace ibofos

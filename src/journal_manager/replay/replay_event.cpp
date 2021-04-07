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

#include "replay_event.h"

#include "src/allocator/allocator.h"
#include "src/include/ibof_event_id.h"
#include "src/logger/logger.h"
#include "src/mapper/mapper.h"

namespace ibofos
{
ReplayEvent::ReplayEvent(void)
{
}

ReplayEvent::~ReplayEvent(void)
{
}

ReplayBlockMapUpdate::ReplayBlockMapUpdate(Mapper* mapper, Allocator* allocator, BlockWriteDoneLog dat)
: mapper(mapper),
  allocator(allocator),
  logData(dat)
{
}

ReplayBlockMapUpdate::~ReplayBlockMapUpdate(void)
{
}

void
ReplayBlockMapUpdate::_ReadBlockMap(void)
{
    readMap.resize(logData.numBlks);

    for (uint32_t offset = 0; offset < logData.numBlks; offset++)
    {
        int shouldRetry = CALLER_NOT_EVENT;
        readMap[offset] = mapper->GetVSAInternal(logData.volId,
            logData.startRba + offset, shouldRetry);
        assert(shouldRetry == OK_READY);
    }
}

int
ReplayBlockMapUpdate::Replay(void)
{
    _ReadBlockMap();

    int result = 0;
    for (uint32_t offset = 0; offset < logData.numBlks; offset++)
    {
        VirtualBlkAddr currentVsa = _GetVsa(offset);
        VirtualBlkAddr read = readMap[offset];

        if (IsSameVsa(read, currentVsa) == false)
        {
            bool needToUpdateMap = _InvalidateOldBlock(offset);
            if (needToUpdateMap == true)
            {
                result = _UpdateMap(offset);
            }
        }
    }

    return result;
}

bool
ReplayBlockMapUpdate::_InvalidateOldBlock(uint32_t offset)
{
    bool isCurrentBlockValid = true;

    VirtualBlkAddr old = _GetOldVsa(offset);
    VirtualBlkAddr read = readMap[offset];
    VirtualBlkAddr current = _GetVsa(offset);

    VirtualBlks blksToInvalidate = {
        .startVsa = UNMAP_VSA,
        .numBlks = 1};

    if (logData.isGC == true)
    {
        if (IsSameVsa(read, old))
        {
            blksToInvalidate.startVsa = read;
        }
        else
        {
            assert(read.stripeId != UNMAP_STRIPE);
            blksToInvalidate.startVsa = current;
            isCurrentBlockValid = false;
        }
    }
    else
    {
        if (read.stripeId != UNMAP_STRIPE)
        {
            blksToInvalidate.startVsa = read;
        }
    }

    if (IsSameVsa(blksToInvalidate.startVsa, UNMAP_VSA) == false)
    {
        allocator->InvalidateBlks(blksToInvalidate);

        IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::JOURNAL_REPLAY_STATUS,
            "Replay blk invalidation, RBA {} VSA stripe {} offset {}",
            logData.startRba + offset, logData.startVsa.stripeId, logData.startVsa.offset + offset);
    }

    return isCurrentBlockValid;
}

int
ReplayBlockMapUpdate::_UpdateMap(uint32_t offset)
{
    BlkAddr rba = _GetRba(offset);
    VirtualBlks virtualBlks = {
        .startVsa = _GetVsa(offset),
        .numBlks = 1};

    int result = mapper->SetVsaMapInternal(logData.volId, rba, virtualBlks);

    IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::JOURNAL_REPLAY_STATUS,
        "[vsid {}] Block map request (retcode {}), rba {}, stripe offset {}",
        virtualBlks.startVsa.stripeId, result, rba, virtualBlks.startVsa.offset);

    return result;
}

ReplayStripeMapUpdate::ReplayStripeMapUpdate(Mapper* mapper, StripeMapUpdatedLog dat)
: mapper(mapper),
  logData(dat)
{
}

ReplayStripeMapUpdate::~ReplayStripeMapUpdate(void)
{
}

int
ReplayStripeMapUpdate::Replay(void)
{
    int ret = mapper->UpdateStripeMap(logData.vsid, logData.newMap.stripeId,
        logData.newMap.stripeLoc);

    IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::JOURNAL_REPLAY_STATUS,
        "[vsid {}] Stripe map updated, lsid {}, location {}",
        logData.vsid, logData.newMap.stripeId, logData.newMap.stripeLoc);

    return ret;
}

ReplayStripeAllocation::ReplayStripeAllocation(Mapper* mapper, Allocator* allocator,
    StripeId vsid, StripeId wbLsid)
: mapper(mapper),
  allocator(allocator),
  vsid(vsid),
  wbLsid(wbLsid)
{
}

ReplayStripeAllocation::~ReplayStripeAllocation(void)
{
}

int
ReplayStripeAllocation::Replay(void)
{
    int result = 0;

    result = mapper->UpdateStripeMap(vsid, wbLsid, IN_WRITE_BUFFER_AREA);
    if (result != 0)
    {
        return result;
    }
    IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::JOURNAL_REPLAY_STATUS,
        "[vsid {}] Stripe map updated, wblsid {}", vsid, wbLsid);

    allocator->ReplayStripeAllocation(vsid, wbLsid);
    IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::JOURNAL_REPLAY_STATUS,
        "[vsid {}] wblsid {} is allocated", vsid, wbLsid);
    return result;
}

ReplaySegmentAllocation::ReplaySegmentAllocation(Allocator* allocator, StripeId userLsid)
: allocator(allocator),
  userLsid(userLsid)
{
}

ReplaySegmentAllocation::~ReplaySegmentAllocation(void)
{
}

int
ReplaySegmentAllocation::Replay(void)
{
    allocator->ReplaySegmentAllocation(userLsid);
    return 0;
}

ReplayStripeFlush::ReplayStripeFlush(Allocator* allocator, StripeId vsid, StripeId wbLsid, StripeId userLsid)
: allocator(allocator),
  vsid(vsid),
  wbLsid(wbLsid),
  userLsid(userLsid)
{
}

ReplayStripeFlush::~ReplayStripeFlush(void)
{
}

int
ReplayStripeFlush::Replay(void)
{
    allocator->ReplayStripeFlushed(wbLsid);
    allocator->TryToUpdateSegmentValidBlks(userLsid);

    IBOF_TRACE_DEBUG(EID(JOURNAL_REPLAY_STRIPE_FLUSHED),
        "[vsid {}] Write buffer lsid {} is freed", vsid, wbLsid);

    return 0;
}

} // namespace ibofos

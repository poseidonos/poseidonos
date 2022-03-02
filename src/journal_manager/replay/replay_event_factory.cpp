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

#include "src/journal_manager/replay/replay_event_factory.h"

#include "src/journal_manager/replay/replay_block_map_update.h"
#include "src/journal_manager/replay/replay_segment_allocation.h"
#include "src/journal_manager/replay/replay_stripe_allocation.h"
#include "src/journal_manager/replay/replay_stripe_flush.h"
#include "src/journal_manager/replay/replay_stripe_map_update.h"

namespace pos
{
ReplayEventFactory::ReplayEventFactory(StripeReplayStatus* status,
    IVSAMap* vsaMap, IStripeMap* stripeMap, IContextReplayer* ctxReplayer,
    ISegmentCtx* segmentCtx, IArrayInfo* info, ActiveWBStripeReplayer* wbReplayer)
: status(status),
  vsaMap(vsaMap),
  stripeMap(stripeMap),
  contextReplayer(ctxReplayer),
  segmentCtx(segmentCtx),
  arrayInfo(info),
  wbReplayer(wbReplayer)
{
}

ReplayEvent*
ReplayEventFactory::CreateBlockWriteReplayEvent(int volId, BlkAddr startRba,
    VirtualBlkAddr startVsa, uint64_t numBlks, bool replaySegmentInfo)
{
    ReplayBlockMapUpdate* blockMapUpdate = new ReplayBlockMapUpdate(vsaMap, segmentCtx, status,
        wbReplayer, volId, startRba, startVsa, numBlks, replaySegmentInfo);
    return blockMapUpdate;
}

ReplayEvent*
ReplayEventFactory::CreateStripeMapUpdateReplayEvent(StripeId vsid, StripeAddr dest)
{
    ReplayEvent* stripeMapUpdate = new ReplayStripeMapUpdate(stripeMap, status, vsid, dest);
    return stripeMapUpdate;
}

ReplayEvent*
ReplayEventFactory::CreateStripeFlushReplayEvent(StripeId vsid, StripeId wbLsid, StripeId userLsid)
{
    ReplayEvent* stripeFlush = new ReplayStripeFlush(contextReplayer,
        status, vsid, wbLsid, userLsid);
    return stripeFlush;
}

ReplayEvent*
ReplayEventFactory::CreateStripeAllocationReplayEvent(StripeId vsid, StripeId wbLsid)
{
    ReplayEvent* stripeAllocation = new ReplayStripeAllocation(stripeMap,
        contextReplayer, status, vsid, wbLsid);
    return stripeAllocation;
}

ReplayEvent*
ReplayEventFactory::CreateSegmentAllocationReplayEvent(StripeId userLsid)
{
    ReplayEvent* segmentAllocation = new ReplaySegmentAllocation(contextReplayer,
        arrayInfo, status, userLsid);
    return segmentAllocation;
}

} // namespace pos

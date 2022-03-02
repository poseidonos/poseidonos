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

#include "src/journal_manager/replay/user_replay_stripe.h"

#include "src/include/pos_event_id.h"
#include "src/journal_manager/log/log_handler.h"
#include "src/journal_manager/replay/active_user_stripe_replayer.h"
#include "src/journal_manager/replay/active_wb_stripe_replayer.h"
#include "src/journal_manager/replay/replay_event_factory.h"
#include "src/logger/logger.h"
#include "src/mapper/i_stripemap.h"

namespace pos
{
// Constructor for product code
UserReplayStripe::UserReplayStripe(StripeId vsid, IVSAMap* vsaMap, IStripeMap* stripeMap,
    IContextReplayer* contextReplayer,
    ISegmentCtx* segmentCtx, IArrayInfo* arrayInfo,
    ActiveWBStripeReplayer* wbReplayer, ActiveUserStripeReplayer* userReplayer)
: ReplayStripe(vsid, vsaMap, stripeMap, contextReplayer, segmentCtx,
      arrayInfo, wbReplayer, userReplayer)
{
}

// Constructor for unit test
UserReplayStripe::UserReplayStripe(IVSAMap* vsaMap, IStripeMap* stripeMap,
    ActiveWBStripeReplayer* wbReplayer, ActiveUserStripeReplayer* userReplayer,
    StripeReplayStatus* status, ReplayEventFactory* factory, ReplayEventList* replayEvents)
: ReplayStripe(vsaMap, stripeMap, wbReplayer, userReplayer, status, factory, replayEvents)
{
}

void
UserReplayStripe::AddLog(ReplayLog replayLog)
{
    ReplayStripe::AddLog(replayLog);
    _AddLog(replayLog.log);
}

void
UserReplayStripe::_AddLog(LogHandlerInterface* log)
{
    if (log->GetType() == LogType::BLOCK_WRITE_DONE)
    {
        BlockWriteDoneLog dat = *(reinterpret_cast<BlockWriteDoneLog*>(log->GetData()));
        status->BlockLogFound(dat);
        _CreateBlockWriteReplayEvent(dat);
    }
    else if (log->GetType() == LogType::STRIPE_MAP_UPDATED)
    {
        StripeMapUpdatedLog dat = *(reinterpret_cast<StripeMapUpdatedLog*>(log->GetData()));
        status->StripeLogFound(dat);
        // Stripe flush log will be added in Replay
    }
}

void
UserReplayStripe::_CreateBlockWriteReplayEvent(BlockWriteDoneLog dat)
{
    ReplayEvent* blockWriteEvent =
        replayEventFactory->CreateBlockWriteReplayEvent(dat.volId, dat.startRba,
            dat.startVsa, dat.numBlks, replaySegmentInfo);
    replayEvents.push_back(blockWriteEvent);
}

int
UserReplayStripe::Replay(void)
{
    _CreateStripeEvents();

    userStripeReplayer->Update(status->GetUserLsid());
    wbStripeReplayer->Update(*status);

    int result = ReplayStripe::Replay();
    return result;
}

void
UserReplayStripe::_CreateStripeEvents(void)
{
    StripeAddr readStripeAddr = stripeMap->GetLSA(status->GetVsid());
    StripeAddr wbStripeAddr = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = status->GetWbLsid()};
    StripeAddr userStripeAddr = {
        .stripeLoc = IN_USER_AREA,
        .stripeId = status->GetUserLsid()};

    if (status->IsFlushed() == false)
    {
        if (!(readStripeAddr == wbStripeAddr))
        {
            _CreateStripeAllocationEvent();
        }
    }
    else
    {
        if (readStripeAddr == wbStripeAddr)
        {
            _CreateStripeFlushReplayEvent();
        }
        else if (readStripeAddr == userStripeAddr)
        {
            // Map is already upated to user stripe
        }
        else
        {
            _CreateStripeAllocationEvent();
            _CreateStripeFlushReplayEvent();
        }
    }

    _CreateSegmentAllocationEvent();
}

} // namespace pos

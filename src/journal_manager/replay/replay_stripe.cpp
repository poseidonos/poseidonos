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

#include "src/journal_manager/replay/active_user_stripe_replayer.h"
#include "src/journal_manager/replay/active_wb_stripe_replayer.h"
#include "src/journal_manager/replay/replay_stripe.h"
#include "src/journal_manager/replay/replay_event_factory.h"

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
ReplayStripe::ReplayStripe(StripeId vsid, IVSAMap* vsaMap, IStripeMap* stripeMap,
    IWBStripeCtx* wbStripeCtx, ISegmentCtx* segmentCtx,
    IBlockAllocator* blockAllocator, IArrayInfo* arrayInfo,
    ActiveWBStripeReplayer* wbReplayer, ActiveUserStripeReplayer* userReplayer)
: wbStripeReplayer(wbReplayer),
  userStripeReplayer(userReplayer),
  vsaMap(vsaMap),
  stripeMap(stripeMap)
{
    status = new StripeReplayStatus(vsid);
    replayEventFactory = new ReplayEventFactory(status,
        vsaMap, stripeMap, wbStripeCtx, segmentCtx, blockAllocator, arrayInfo);
}

ReplayStripe::~ReplayStripe(void)
{
    for (auto replayEvent : replayEvents)
    {
        delete replayEvent;
    }
    replayEvents.clear();

    delete status;
    delete replayEventFactory;
}

void
ReplayStripe::AddLog(LogHandlerInterface* log)
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
ReplayStripe::_CreateBlockWriteReplayEvent(BlockWriteDoneLog dat)
{
    ReplayEvent* blockWriteEvent = replayEventFactory->CreateBlockWriteReplayEvent(dat);
    replayEvents.push_back(blockWriteEvent);
}

int
ReplayStripe::Replay(void)
{
    int result = 0;

    _CreateStripeEvents();

    if ((result = _ReplayEvents()) != 0)
    {
        return result;
    }
    status->Print();

    if ((result = _UpdateActiveStripeInfo()) != 0)
    {
        return result;
    }
    return result;
}

void
ReplayStripe::_CreateStripeEvents(void)
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
}

void
ReplayStripe::_CreateStripeAllocationEvent(void)
{
    ReplayEvent* stripeAllocation =
        replayEventFactory->CreateStripeAllocationReplayEvent(status->GetVsid(), status->GetWbLsid());
        replayEvents.push_front(stripeAllocation);

    ReplayEvent* segmentAllocation =
        replayEventFactory->CreateSegmentAllocationReplayEvent(status->GetUserLsid());
    replayEvents.push_front(segmentAllocation);
}


void
ReplayStripe::_CreateStripeFlushReplayEvent(void)
{
    StripeAddr dest = {
        .stripeLoc = IN_USER_AREA,
        .stripeId = status->GetUserLsid()
    };
    ReplayEvent* stripeMapUpdate =
        replayEventFactory->CreateStripeMapUpdateReplayEvent(status->GetVsid(), dest);
    replayEvents.push_back(stripeMapUpdate);

    ReplayEvent* flushEvent =
        replayEventFactory->CreateStripeFlushReplayEvent(status->GetVsid(),
        status->GetWbLsid(), status->GetUserLsid());
    replayEvents.push_back(flushEvent);
}

int
ReplayStripe::_ReplayEvents(void)
{
    int result = 0;
    for (auto replayEvent : replayEvents)
    {
        result = replayEvent->Replay();

        if (result != 0)
        {
            return result;
        }
    }
    return result;
}

int
ReplayStripe::_UpdateActiveStripeInfo(void)
{
    wbStripeReplayer->Update(*status);
    userStripeReplayer->Update(status->GetUserLsid());

    return 0;
}

void
ReplayStripe::DeleteBlockMapReplayEvents(void)
{
    int numErasedLogs = 0;

    for (auto it = replayEvents.begin(); it != replayEvents.end();)
    {
        ReplayEvent* replayEvent = *it;

        if (replayEvent->GetType() == ReplayEventType::BLOCK_MAP_UPDATE)
        {
            it = replayEvents.erase(it);
            delete replayEvent;

            numErasedLogs++;
        }
        else
        {
            it++;
        }
    }

    int eventId = static_cast<int>(POS_EVENT_ID::JOURNAL_REPLAY_VOLUME_EVENT);
    POS_TRACE_DEBUG(eventId, "[Replay] {} block log of volume {} is skipped",
        numErasedLogs, status->GetVolumeId());
}

} // namespace pos

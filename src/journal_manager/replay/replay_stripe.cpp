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

#include "src/journal_manager/replay/replay_stripe.h"

#include "src/include/pos_event_id.h"
#include "src/journal_manager/replay/replay_event_factory.h"
#include "src/logger/logger.h"

namespace pos
{
// Constructor for product code
ReplayStripe::ReplayStripe(StripeId vsid, IVSAMap* inputVsaMap, IStripeMap* inputStripeMap,
    IContextReplayer* ctxReplayer,
    ISegmentCtx* segmentCtx, IArrayInfo* arrayInfo,
    ActiveWBStripeReplayer* wbReplayer, ActiveUserStripeReplayer* userReplayer)
: ReplayStripe(inputVsaMap, inputStripeMap, wbReplayer, userReplayer, nullptr, nullptr, nullptr)
{
    status = new StripeReplayStatus(vsid);
    replayEventFactory = new ReplayEventFactory(status, inputVsaMap, inputStripeMap, ctxReplayer, segmentCtx, arrayInfo, wbReplayer);
}

// Constructor for unit test
ReplayStripe::ReplayStripe(IVSAMap* inputVsaMap, IStripeMap* inputStripeMap,
    ActiveWBStripeReplayer* wbReplayer,
    ActiveUserStripeReplayer* userReplayer,
    StripeReplayStatus* inputStatus, ReplayEventFactory* factory, ReplayEventList* inputReplayEventList)
: status(inputStatus),
  replayEventFactory(factory),
  wbStripeReplayer(wbReplayer),
  userStripeReplayer(userReplayer),
  vsaMap(inputVsaMap),
  stripeMap(inputStripeMap),
  replaySegmentInfo(true)
{
    if (inputReplayEventList != nullptr)
    {
        replayEvents = *inputReplayEventList;
    }
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
ReplayStripe::AddLog(ReplayLog replayLog)
{
    if (replayLog.segInfoFlushed == true)
    {
        replaySegmentInfo = false;
    }
    status->RecordLogFoundTime(replayLog.time);
}

int
ReplayStripe::Replay(void)
{
    int result = 0;

    if ((result = _ReplayEvents()) != 0)
    {
        return result;
    }
    status->Print();

    return result;
}

void
ReplayStripe::_CreateSegmentAllocationEvent(void)
{
    ReplayEvent* segmentAllocation =
        replayEventFactory->CreateSegmentAllocationReplayEvent(status->GetUserLsid());
    replayEvents.push_front(segmentAllocation);
}

void
ReplayStripe::_CreateStripeAllocationEvent(void)
{
    ReplayEvent* stripeAllocation =
        replayEventFactory->CreateStripeAllocationReplayEvent(status->GetVsid(), status->GetWbLsid());
    replayEvents.push_front(stripeAllocation);
}

void
ReplayStripe::_CreateStripeFlushReplayEvent(void)
{
    StripeAddr dest = {
        .stripeLoc = IN_USER_AREA,
        .stripeId = status->GetUserLsid()};
    ReplayEvent* stripeMapUpdate =
        replayEventFactory->CreateStripeMapUpdateReplayEvent(status->GetVsid(), dest);
    replayEvents.push_back(stripeMapUpdate);

    if (replaySegmentInfo == true)
    {
        ReplayEvent* flushEvent =
            replayEventFactory->CreateStripeFlushReplayEvent(status->GetVsid(),
                status->GetWbLsid(), status->GetUserLsid());
        replayEvents.push_back(flushEvent);
    }
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

    int eventId = static_cast<int>(EID(JOURNAL_REPLAY_VOLUME_EVENT));
    POS_TRACE_DEBUG(eventId, "[Replay] {} block log of volume {} is skipped",
        numErasedLogs, status->GetVolumeId());
}

} // namespace pos

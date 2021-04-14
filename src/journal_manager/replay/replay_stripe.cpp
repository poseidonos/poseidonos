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

#include "replay_stripe.h"

#include "active_user_stripe_replayer.h"
#include "active_wb_stripe_replayer.h"

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
ReplayStripe::ReplayStripe(StripeId vsid, IVSAMap* vsaMap, IStripeMap* stripeMap,
    IWBStripeCtx* wbStripeCtx, ISegmentCtx* segmentCtx,
    IBlockAllocator* blockAllocator, IArrayInfo* arrayInfo,
    ActiveWBStripeReplayer* wbReplayer, ActiveUserStripeReplayer* userReplayer)
: status(new StripeReplayStatus(vsid)),
  wbStripeReplayer(wbReplayer),
  userStripeReplayer(userReplayer),
  vsaMap(vsaMap),
  stripeMap(stripeMap),
  wbStripeCtx(wbStripeCtx),
  segmentCtx(segmentCtx),
  blockAllocator(blockAllocator),
  arrayInfo(arrayInfo)
{
}

ReplayStripe::~ReplayStripe(void)
{
    for (auto replayEvent : replayEvents)
    {
        delete replayEvent;
    }
    replayEvents.clear();

    delete status;
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
        _CreateStripeFlushReplayEvent(dat);
    }
}

void
ReplayStripe::_CreateBlockWriteReplayEvent(BlockWriteDoneLog dat)
{
    ReplayBlockMapUpdate* replayEvent = new ReplayBlockMapUpdate(vsaMap, blockAllocator,
        status, dat);
    replayEvents.push_back(replayEvent);
}

void
ReplayStripe::_CreateStripeFlushReplayEvent(StripeMapUpdatedLog dat)
{
    assert(dat.oldMap.stripeLoc == IN_WRITE_BUFFER_AREA);
    assert(dat.newMap.stripeLoc == IN_USER_AREA);

    ReplayEvent* replayEvent = new ReplayStripeMapUpdate(stripeMap, status, dat);
    replayEvents.push_back(replayEvent);

    ReplayEvent* flushEvent = new ReplayStripeFlush(wbStripeCtx, segmentCtx,
        status, dat.vsid, dat.oldMap.stripeId, dat.oldMap.stripeId);
    replayEvents.push_back(flushEvent);
}

bool
ReplayStripe::_IsSegmentAllocationReplayRequired(void)
{
    const PartitionLogicalSize* userDataSize =
        arrayInfo->GetSizeInfo(PartitionType::USER_DATA);
    StripeId stripesPerSegment = userDataSize->stripesPerSegment;
    return (status->GetUserLsid() % stripesPerSegment == 0);
}

// TODO (huijeong.kim) Currently these 2 functions are not used, change required
bool
ReplayStripe::_IsStripeAllocationReplayRequired(void)
{
    StripeAddr readStripeAddr = stripeMap->GetLSA(status->GetVsid());
    bool stripeNotAllocated =
        (readStripeAddr.stripeLoc != IN_WRITE_BUFFER_AREA ||
            readStripeAddr.stripeId != status->GetWbLsid());

    // TODO(huijeong.kim) gc handler doesn't set UNMAP after clear the lsid
    // Check if always' UNMAP when LSID is not set before

    return stripeNotAllocated;
}

bool
ReplayStripe::_IsStripeFlushReplayRequired(StripeMapUpdatedLog dat)
{
    StripeAddr readStripeAddr = stripeMap->GetLSA(dat.vsid);

    if (readStripeAddr.stripeId == dat.newMap.stripeId &&
        readStripeAddr.stripeLoc == IN_USER_AREA)
    {
        return false;
    }
    else
    {
        return true;
    }
}


int
ReplayStripe::Replay(void)
{
    int result = 0;

    _CreateStripeAllocationEvent();

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
ReplayStripe::_CreateStripeAllocationEvent(void)
{
    ReplayEvent* replayEvent = new ReplayStripeAllocation(stripeMap, wbStripeCtx,
        status, status->GetVsid(), status->GetWbLsid());
    replayEvents.push_front(replayEvent);

    if (_IsSegmentAllocationReplayRequired())
    {
        ReplayEvent* replayEvent = new ReplaySegmentAllocation(segmentCtx,
            status, status->GetUserLsid());
        replayEvents.push_front(replayEvent);
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

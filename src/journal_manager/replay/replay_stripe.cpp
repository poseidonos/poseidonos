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
#include "src/allocator/allocator.h"
#include "src/array/array.h"
#include "src/array/partition/partition.h"
#include "src/array/partition/partition_size_info.h"
#include "src/include/ibof_event_id.h"
#include "src/logger/logger.h"
#include "src/mapper/mapper.h"

namespace ibofos
{
ReplayStripe::ReplayStripe(StripeId vsid, Mapper* mapper, Allocator* allocator, Array* array,
    ActiveWBStripeReplayer* wbReplayer, ActiveUserStripeReplayer* userReplayer)
: info(vsid),
  wbStripeReplayer(wbReplayer),
  userStripeReplayer(userReplayer),
  mapper(mapper),
  allocator(allocator),
  array(array)
{
}

ReplayStripe::~ReplayStripe(void)
{
    for (auto replayEvent : logs)
    {
        delete replayEvent;
    }
    logs.clear();
}

void
ReplayStripe::AddLog(LogHandlerInterface* log)
{
    if (log->GetType() == LogType::BLOCK_WRITE_DONE)
    {
        BlockWriteDoneLog dat = *(reinterpret_cast<BlockWriteDoneLog*>(log->GetData()));
        _UpdateStripeInfo(dat);
        _AddBlockWriteReplayEvent(dat);
    }
    else if (log->GetType() == LogType::STRIPE_MAP_UPDATED)
    {
        StripeMapUpdatedLog dat = *(reinterpret_cast<StripeMapUpdatedLog*>(log->GetData()));
        _UpdateStripeInfo(dat);
        _AddStripeFlushReplayEvent(dat);
    }
}

void
ReplayStripe::_AddBlockWriteReplayEvent(BlockWriteDoneLog dat)
{
    if (logs.size() == 0 && _IsStripeAllocationReplayRequired())
    {
        if (_IsSegmentAllocationReplayRequired())
        {
            ReplayEvent* replayEvent = new ReplaySegmentAllocation(allocator,
                info.GetUserLsid());
            logs.push_back(replayEvent);
        }

        ReplayEvent* replayEvent = new ReplayStripeAllocation(mapper, allocator,
            info.GetVsid(), info.GetWbLsid());
        logs.push_back(replayEvent);

        assert(dat.writeBufferStripeAddress.stripeLoc == IN_WRITE_BUFFER_AREA);
    }

    ReplayEvent* replayEvent = new ReplayBlockMapUpdate(mapper, allocator, dat);
    logs.push_back(replayEvent);
}

void
ReplayStripe::_AddStripeFlushReplayEvent(StripeMapUpdatedLog dat)
{
    assert(dat.oldMap.stripeLoc == IN_WRITE_BUFFER_AREA);
    assert(dat.newMap.stripeLoc == IN_USER_AREA);

    if (_IsStripeFlushReplayRequired(dat))
    {
        ReplayEvent* replayEvent = new ReplayStripeMapUpdate(mapper, dat);
        logs.push_back(replayEvent);

        ReplayEvent* flushEvent = new ReplayStripeFlush(allocator,
            dat.vsid, dat.oldMap.stripeId, dat.oldMap.stripeId);
        logs.push_back(flushEvent);
    }
}

bool
ReplayStripe::_IsSegmentAllocationReplayRequired(void)
{
    const PartitionLogicalSize* userDataSize =
        array->GetSizeInfo(PartitionType::USER_DATA);
    StripeId stripesPerSegment = userDataSize->stripesPerSegment;
    return (info.GetUserLsid() % stripesPerSegment == 0);
}

bool
ReplayStripe::_IsStripeAllocationReplayRequired(void)
{
    StripeAddr readStripeAddr = mapper->GetLSA(info.GetVsid());
    bool stripeNotAllocated =
        (readStripeAddr.stripeLoc != IN_WRITE_BUFFER_AREA ||
            readStripeAddr.stripeId != info.GetWbLsid());

    if (stripeNotAllocated == true)
    {
        IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::JOURNAL_DEBUG,
            "Stripe allocation required, vsid {} prev lsid {} prev loc {}",
            info.GetVsid(), readStripeAddr.stripeId, readStripeAddr.stripeLoc);
    }
    // TODO(huijeong.kim) gc handler doesn't set UNMAP after clear the lsid
    // Check if always' UNMAP when LSID is not set before

    return stripeNotAllocated;
}

bool
ReplayStripe::_IsStripeFlushReplayRequired(StripeMapUpdatedLog dat)
{
    StripeAddr readStripeAddr = mapper->GetLSA(dat.vsid);

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

void
ReplayStripe::_UpdateStripeInfo(BlockWriteDoneLog log)
{
    BlkOffset curEndOffset = log.startVsa.offset + log.numBlks - 1;
    info.UpdateLastOffset(curEndOffset);
    info.UpdateVolumeId(log.volId);
    info.UpdateWbLsid((log.writeBufferStripeAddress).stripeId);
    info.UpdateWbIndex(log.wbIndex);
}

void
ReplayStripe::_UpdateStripeInfo(StripeMapUpdatedLog log)
{
    info.UpdateWbLsid(log.oldMap.stripeId);
    info.UpdateUserLsid(log.newMap.stripeId);
    info.ResetOffset();
}

int
ReplayStripe::Replay(void)
{
    int result = 0;
    if ((result = _ReplayEvents()) != 0)
    {
        return result;
    }
    if ((result = _UpdateActiveStripeInfo()) != 0)
    {
        return result;
    }
    return result;
}

int
ReplayStripe::_ReplayEvents(void)
{
    int result = 0;
    for (auto replayEvent : logs)
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
    wbStripeReplayer->Update(info);
    userStripeReplayer->Update(info.GetUserLsid());

    IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::JOURNAL_REPLAY_STATUS,
        "[vsid {}] Update stripe info to active stripe finder, wblsid {}, lastOffset {}, userlsid {}",
        info.GetVsid(), info.GetWbLsid(), info.GetLastOffset(), info.GetUserLsid());

    return 0;
}

void
ReplayStripe::DeleteBlockMapReplayEvents(void)
{
    for (auto it = logs.begin(); it != logs.end();)
    {
        ReplayEvent* replayEvent = *it;

        if (replayEvent->GetType() == ReplayEventType::BLOCK_MAP_UPDATE)
        {
            it = logs.erase(it);
            delete replayEvent;

            IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::JOURNAL_REPLAY_VOLUME_EVENT,
                "[Replay] Block write done log of volume {} is skipped",
                info.GetVolumeId());
        }
        else
        {
            it++;
        }
    }
}

} // namespace ibofos

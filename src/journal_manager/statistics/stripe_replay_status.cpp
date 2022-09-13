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

#include "stripe_replay_status.h"

#include <iomanip>
#include <iostream>

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
StripeReplayStatus::StripeReplayStatus(StripeId vsid)
: StripeLogWriteStatus(vsid),
  numUpdatedBlockMaps(0),
  numInvalidatedBlocks(0),
  segmentAllocated(false),
  stripeAllocated(false),
  stripeMapReplayed(false),
  minTime(UINT64_MAX),
  maxTime(0)
{
}

StripeReplayStatus::~StripeReplayStatus(void)
{
}

void
StripeReplayStatus::Print(void)
{
    int eventId = static_cast<int>(EID(JOURNAL_REPLAY_STATUS));

    std::ostringstream os;

    os << "[Replay vsid " << GetVsid() << "] ";

    if (stripeAllocated == true)
    {
        if (IsWbIndexValid() == true)
        {
            os << "wbIndex " << GetWbIndex() << ", ";
        }

        if (stripeFlushed == true)
        {
            os << "wbstripe alloced/flushed ";
        }
        else
        {
            os << "wbstripe alloced";
        }
        os << "(lsid " << GetWbLsid() << "), ";

        if (segmentAllocated == true)
        {
            os << "userstripe/segment alloced ";
        }
        else
        {
            os << "userstripe alloced ";
        }
        os << "(lsid " << GetUserLsid() << "), ";
    }
    else if (stripeFlushed == true)
    {
        os << "wbstripe flushed (lsid " << GetWbLsid() << "), ";
    }

    if (numFoundBlockMaps != 0)
    {
        os << "RBA " << smallestRba << " to " << largestRba << ", ";
        os << numUpdatedBlockMaps << " block updated";
        os << "(" << firstBlockOffset << "-" << lastBlockOffset << "), ";

        int numSkippedStripeMapUpdate = (stripeMapReplayed == true) ? 0 : 1;
        os << (numFoundBlockMaps - numUpdatedBlockMaps) << " blk/";
        os << numSkippedStripeMapUpdate << "stripe update skipped";
    }

    POS_TRACE_DEBUG(eventId, os.str());
    POS_TRACE_DEBUG_IN_MEMORY(ModuleInDebugLogDump::JOURNAL, eventId, os.str());
}

void
StripeReplayStatus::BlockWritten(BlkOffset startOffset, uint32_t numBlks)
{
    assert(startOffset >= firstBlockOffset && startOffset <= lastBlockOffset);
    numUpdatedBlockMaps += numBlks;
}

void
StripeReplayStatus::StripeFlushed(void)
{
    if (stripeMapReplayed == true)
    {
        int eventId = static_cast<int>(EID(JOURNAL_REPLAY_STATUS));
        POS_TRACE_ERROR(eventId, "[Replay] Stripe flush replayed more than once");
    }
    stripeMapReplayed = true;
}

void
StripeReplayStatus::BlockInvalidated(uint32_t numBlks)
{
    numInvalidatedBlocks += numBlks;
}

void
StripeReplayStatus::SegmentAllocated(void)
{
    segmentAllocated = true;
}

void
StripeReplayStatus::StripeAllocated(void)
{
    stripeAllocated = true;
}

void
StripeReplayStatus::RecordLogFoundTime(uint64_t time)
{
    if (time < minTime)
    {
        minTime = time;
    }

    if (time > maxTime)
    {
        maxTime = time;
    }
}
} // namespace pos

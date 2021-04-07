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

#include "active_user_stripe_replayer.h"

#include "src/allocator/allocator.h"
#include "src/array/array.h"
#include "src/array/partition/partition.h"
#include "src/array/partition/partition_size_info.h"
#include "src/logger/logger.h"

namespace ibofos
{
ActiveUserStripeReplayer::ActiveUserStripeReplayer(Allocator* allocator, Array* array)
: allocator(allocator),
  array(array)
{
    userLsids.clear();
    lastLsid.clear();
}

ActiveUserStripeReplayer::~ActiveUserStripeReplayer(void)
{
    _Reset();
}

void
ActiveUserStripeReplayer::_Reset(void)
{
    userLsids.clear();
    lastLsid.clear();
}

void
ActiveUserStripeReplayer::Update(StripeId userLsid)
{
    userLsids.push_back(userLsid);
}

int
ActiveUserStripeReplayer::Replay(void)
{
    const PartitionLogicalSize* userDataSize =
        array->GetSizeInfo(PartitionType::USER_DATA);
    StripeId stripesPerSegment = userDataSize->stripesPerSegment;
    _FindLastStripeOfSegment(stripesPerSegment);
    _EraseFullSegmentLsid(stripesPerSegment);

    StripeId currentSsdLsid = _FindLastLsid(stripesPerSegment);
    allocator->ReplaySsdLsid(currentSsdLsid);

    IBOF_TRACE_INFO(EID(JOURNAL_REPLAY_SSD_LSID),
        "[Replay] SSD LSID is updated to {}", currentSsdLsid);
    return 0;
}

void
ActiveUserStripeReplayer::_FindLastStripeOfSegment(uint32_t stripesPerSegment)
{
    for (StripeId ssdLsid : userLsids)
    {
        SegmentId segId = ssdLsid / stripesPerSegment;

        if (lastLsid.find(segId) == lastLsid.end())
        {
            lastLsid.emplace(segId, ssdLsid);
            IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::JOURNAL_DEBUG,
                "currentSsdLsid new candiate inserted : lsid {}",
                ssdLsid);
        }
        else
        {
            if (lastLsid[segId] < ssdLsid)
            {
                lastLsid[segId] = ssdLsid;
                IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::JOURNAL_DEBUG,
                    "currentSsdLsid candiate updated : lsid {}",
                    ssdLsid);
            }
        }
    }
}

void
ActiveUserStripeReplayer::_EraseFullSegmentLsid(uint32_t stripesPerSegment)
{
    for (auto it = lastLsid.begin(); it != lastLsid.end();)
    {
        if (_IsLastStripe(it->second, stripesPerSegment))
        {
            it = lastLsid.erase(it);
        }
        else
        {
            it++;
        }
    }
}

StripeId
ActiveUserStripeReplayer::_FindLastLsid(uint32_t stripesPerSegment)
{
    StripeId lsid;
    if (lastLsid.empty())
    {
        lsid = stripesPerSegment - 1;
    }
    else if (lastLsid.size() > 1)
    {
        IBOF_TRACE_ERROR(
            EID(JOURNAL_REPLAY_SSD_LSID),
            "Cannot find latest LSID - number of active segment found is {}",
            lastLsid.size());

        lsid = lastLsid.begin()->second;
    }
    else
    {
        lsid = lastLsid.begin()->second;
    }

    return lsid;
}
} // namespace ibofos

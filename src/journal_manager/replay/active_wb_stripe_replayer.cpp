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

#include "active_wb_stripe_replayer.h"

#include "src/allocator/allocator.h"
#include "src/logger/logger.h"

namespace ibofos
{
ActiveWBStripeReplayer::ActiveWBStripeReplayer(Allocator* allocator, PendingStripeList& pendingStripeList)
: pendingStripes(pendingStripeList),
  allocator(allocator)
{
    readTails = allocator->GetActiveStripeTail();
    foundActiveStripes.resize(readTails.size());
}

ActiveWBStripeReplayer::~ActiveWBStripeReplayer(void)
{
}

void
ActiveWBStripeReplayer::Update(StripeInfo stripeInfo)
{
    int index = _FindWbufIndex(stripeInfo);

    if (_IsFlushedStripe(stripeInfo))
    {
        if (index != INDEX_NOT_FOUND)
        {
            _ResetWbufTail(index);
        }
    }
    else
    {
        VirtualBlkAddr tailVsa = {
            .stripeId = stripeInfo.GetVsid(),
            .offset = stripeInfo.GetLastOffset() + 1};

        if (index == INDEX_NOT_FOUND)
        {
            _AddToPendingFlushList(stripeInfo.GetVolumeId(),
                stripeInfo.GetWbLsid(), tailVsa);
        }
        else
        {
            ActiveStripeAddr tailAddr(stripeInfo.GetVolumeId(), tailVsa,
                stripeInfo.GetWbLsid());
            _UpdateWbufTail(index, tailAddr);

            IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::JOURNAL_REPLAY_STATUS,
                "[vsid {}] Updated to the active stripe list (index {} wb lsid {})",
                stripeInfo.GetVsid(), index, stripeInfo.GetWbLsid());
        }
    }
}

bool
ActiveWBStripeReplayer::_IsFlushedStripe(StripeInfo stripeInfo)
{
    return (stripeInfo.IsLastOffsetValid() == false);
}

int
ActiveWBStripeReplayer::_FindWbufIndex(StripeInfo stripeInfo)
{
    if (stripeInfo.IsWbIndexValid() == true)
    {
        return stripeInfo.GetWbIndex();
    }
    else
    {
        for (auto it = readTails.begin(); it != readTails.end(); it++)
        {
            if (it->stripeId == stripeInfo.GetVsid())
            {
                return (it - readTails.begin());
            }
        }
    }
    return INDEX_NOT_FOUND;
}

void
ActiveWBStripeReplayer::_AddToPendingFlushList(int volId, StripeId wbLsid, VirtualBlkAddr tail)
{
    pendingStripes.push_back(new PendingStripe(volId, wbLsid, tail));

    IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::JOURNAL_REPLAY_STATUS,
        "[vsid {}] Added to the pending flush stripe list (wb lsid {})",
        tail.stripeId, wbLsid);
}

void
ActiveWBStripeReplayer::_ResetWbufTail(int index)
{
    ActiveStripeAddr resetAddr;
    resetAddr.Reset();

    foundActiveStripes[index].push_back(resetAddr);
}

void
ActiveWBStripeReplayer::_UpdateWbufTail(int index, ActiveStripeAddr newAddr)
{
    foundActiveStripes[index].push_back(newAddr);
}

int
ActiveWBStripeReplayer::Replay(void)
{
    for (uint32_t index = 0; index < foundActiveStripes.size(); index++)
    {
        if (foundActiveStripes[index].size() == 0)
        {
            continue;
        }

        ActiveStripeAddr current = _ReplayStripesExceptActive(index);

        if (current.IsValid() == true)
        {
            int ret = allocator->RestoreActiveStripeTail(index,
                current.GetTail(), current.GetWbLsid());

            if (ret < 0)
            {
                IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::JOURNAL_REPLAY_WB_TAIL,
                    "Restore active stripe failed");
                return ret;
            }
            else
            {
                IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::JOURNAL_REPLAY_WB_TAIL,
                    "Restore active stripe, index {} vsid {} wbLsid {} offset {})",
                    index, current.GetTail().stripeId, current.GetWbLsid(), current.GetTail().offset);
            }
        }
        else
        {
            allocator->ResetActiveStripeTail(index);
            IBOF_TRACE_INFO((int)IBOF_EVENT_ID::JOURNAL_REPLAY_WB_TAIL,
                "[Replay] wbuf tail [{}] is reset", index);
        }
    }
    return 0;
}

ActiveStripeAddr
ActiveWBStripeReplayer::_ReplayStripesExceptActive(int index)
{
    ActiveStripeAddr lastActiveStripe;

    for (auto it = foundActiveStripes[index].begin();
         it != foundActiveStripes[index].end();)
    {
        if (it->IsValid() == false)
        {
            it = foundActiveStripes[index].erase(it);
        }
        else
        {
            if (lastActiveStripe.IsValid() == true)
            {
                _AddToPendingFlushList(lastActiveStripe.GetVolumeId(),
                    lastActiveStripe.GetWbLsid(), lastActiveStripe.GetTail());
            }

            lastActiveStripe = *it;
            ++it;
        }
    }

    return lastActiveStripe;
}

} // namespace ibofos

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

#include "flush_pending_stripes.h"

#include "src/allocator/allocator.h"
#include "src/include/ibof_event_id.h"
#include "src/logger/logger.h"

namespace ibofos
{
FlushPendingStripes::FlushPendingStripes(PendingStripeList& pendingStripes,
    Allocator* allocator, ReplayProgressReporter* reporter)
: ReplayTask(reporter),
  pendingStripes(pendingStripes),
  allocator(allocator)
{
}

FlushPendingStripes::~FlushPendingStripes(void)
{
    for (auto pStripe : pendingStripes)
    {
        delete pStripe;
    }
    pendingStripes.clear();
}

int
FlushPendingStripes::GetNumSubTasks(void)
{
    return 2;
}

int
FlushPendingStripes::Start(void)
{
    int ret = 0;
    for (auto pStripe : pendingStripes)
    {
        ret = allocator->FlushStripe(pStripe->volumeId, pStripe->wbLsid, pStripe->tailVsa);
        if (ret < 0)
        {
            IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::JOURNAL_REPLAY_STRIPE_FLUSH_FAILED,
                "Failed to flush stripe, wb lsid {}, tail offset {}",
                pStripe->wbLsid, pStripe->tailVsa.offset);
        }
        else
        {
            IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::JOURNAL_REPLAY_STRIPE,
                "Request to flush stripe, wb lsid {}, tail offset {}",
                pStripe->wbLsid, pStripe->tailVsa.offset);
        }
    }
    reporter->SubTaskCompleted(GetId(), 1);

    allocator->FlushFullActiveStripes();
    reporter->SubTaskCompleted(GetId(), 1);
    return ret;
}

ReplayTaskId
FlushPendingStripes::GetId(void)
{
    return ReplayTaskId::FLUSH_PENDING_STRIPES;
}

int
FlushPendingStripes::GetWeight(void)
{
    return 10;
}

} // namespace ibofos

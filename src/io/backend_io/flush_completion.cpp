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

#include "flush_completion.h"

#include "src/allocator/allocator.h"
#include "src/allocator/stripe.h"
#include "src/allocator/stripe_put_event.h"
#include "src/include/ibof_event_id.hpp"
#include "src/logger/logger.h"
#include "src/mapper/mapper.h"
#include "src/scheduler/event_argument.h"

namespace ibofos
{
FlushCompletion::FlushCompletion(Stripe* inputStripe)
: Event(true),
  stripe(inputStripe)
{
}

FlushCompletion::~FlushCompletion()
{
}

bool
FlushCompletion::Execute()
{
    bool wrapupSuccessful = true;

    Mapper& mapper = *MapperSingleton::Instance();
    StripeAddr userAreaStripeAddr = mapper.GetLSA(stripe->GetVsid());
    bool userArea = mapper.IsInUserDataArea(userAreaStripeAddr);

    IBOF_EVENT_ID eventId =
        IBOF_EVENT_ID::FLUSH_DEBUG_COMPLETION;
    IBOF_TRACE_DEBUG_IN_MEMORY(ModuleInDebugLogDump::IO_FLUSH, static_cast<int>(eventId),
        IbofEventId::GetString(eventId), stripe->GetVsid(),
        userAreaStripeAddr.stripeId,
        static_cast<uint32_t>(userArea));

    if (likely(true == userArea))
    {
        StripeId nvmStripeId = stripe->GetWbLsid();
        StripePutEvent event(*stripe, nvmStripeId);

        bool done = event.Execute();

        if (false == done)
        {
            EventSmartPtr eventForSchedule(
                new StripePutEvent(*stripe, nvmStripeId));
            EventArgument::GetEventScheduler()->EnqueueEvent(eventForSchedule);
        }
    }
    else
    {
        IBOF_EVENT_ID eventId =
            IBOF_EVENT_ID::FLUSH_WRAPUP_STRIPE_NOT_IN_USER_AREA;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId), stripe->GetVsid());

        wrapupSuccessful = false;
    }

    return wrapupSuccessful;
}

} // namespace ibofos

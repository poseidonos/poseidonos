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

#include "flush_completion.h"

#include "src/allocator/event/stripe_put_event.h"
#include "src/allocator/stripe/stripe.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/io/backend_io/flush_count.h"
#include "src/logger/logger.h"
#include "src/mapper_service/mapper_service.h"

namespace pos
{
FlushCompletion::FlushCompletion(Stripe* stripe, int arrayId)
: FlushCompletion(stripe, MapperServiceSingleton::Instance()->GetIStripeMap(arrayId),
      EventSchedulerSingleton::Instance(), arrayId)
{
}

FlushCompletion::FlushCompletion(Stripe* stripe,
    IStripeMap* stripeMap,
    EventScheduler* eventScheduler,
    int arrayId)
: Callback(true, CallbackType_FlushCompletion),
  stripe(stripe),
  iStripeMap(stripeMap),
  eventScheduler(eventScheduler),
  arrayId(arrayId)
{
}

FlushCompletion::~FlushCompletion(void)
{
}

bool
FlushCompletion::_DoSpecificJob(void)
{
    bool wrapupSuccessful = true;

    StripeAddr userAreaStripeAddr = iStripeMap->GetLSA(stripe->GetVsid());
    bool userArea = iStripeMap->IsInUserDataArea(userAreaStripeAddr);

    POS_EVENT_ID eventId =
        EID(FLUSH_DEBUG_COMPLETION);
    POS_TRACE_DEBUG_IN_MEMORY(ModuleInDebugLogDump::IO_FLUSH, static_cast<int>(eventId),
        "Flush Completion vsid : {} userstripeid : {} userArea : {}", stripe->GetVsid(),
        userAreaStripeAddr.stripeId,
        static_cast<uint32_t>(userArea));

    if (likely(true == userArea))
    {
        StripeId nvmStripeId = stripe->GetWbLsid();
        StripePutEvent event(*stripe, nvmStripeId, arrayId);

        bool done = event.Execute();
        FlushCountSingleton::Instance()->pendingFlush--;
        if (false == done)
        {
            EventSmartPtr eventForSchedule(new StripePutEvent(*stripe, nvmStripeId, arrayId));
            eventScheduler->EnqueueEvent(eventForSchedule);
        }
    }
    else
    {
        POS_EVENT_ID eventId =
            EID(FLUSH_WRAPUP_STRIPE_NOT_IN_USER_AREA);
        POS_TRACE_ERROR(static_cast<int>(eventId),
            "Stripe #{} is not in UserArea.", stripe->GetVsid());

        wrapupSuccessful = false;
    }

    return wrapupSuccessful;
}

} // namespace pos

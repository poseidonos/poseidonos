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

#include "stripe_map_update_completion.h"

#include "src/allocator/i_context_manager.h"
#include "src/allocator/i_wbstripe_allocator.h"
#include "src/allocator_service/allocator_service.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/include/branch_prediction.h"
#include "src/include/backend_event.h"
#include "src/io/backend_io/flush_completion.h"
#include "src/mapper_service/mapper_service.h"
#include "src/logger/logger.h"

#include <string>

namespace pos
{
StripeMapUpdateCompletion::StripeMapUpdateCompletion(Stripe* inputStripe, std::string& arrayName)
: StripeMapUpdateCompletion(inputStripe,
    AllocatorServiceSingleton::Instance()->GetIContextManager(arrayName),
    MapperServiceSingleton::Instance()->GetIStripeMap(arrayName),
    EventSchedulerSingleton::Instance(),
    arrayName)
{
}

StripeMapUpdateCompletion::StripeMapUpdateCompletion(Stripe* inputStripe,
    IContextManager* ctxManager,
    IStripeMap* iStripeMap,
    EventScheduler* eventScheduler,
    std::string& arrayName)
: Event(false, BackendEvent_Flush),
  stripe(inputStripe),
  iContextManager(ctxManager),
  iStripeMap(iStripeMap),
  eventScheduler(eventScheduler),
  arrayName(arrayName)
{
    SetEventType(BackendEvent_Flush);
}

StripeMapUpdateCompletion::~StripeMapUpdateCompletion(void)
{
}

bool
StripeMapUpdateCompletion::Execute(void)
{
    bool wrapupSuccessful = true;
    StripeId currentLsid = stripe->GetUserLsid();
    iStripeMap->SetLSA(stripe->GetVsid(), stripe->GetUserLsid(), IN_USER_AREA);
    iContextManager->UpdateOccupiedStripeCount(currentLsid);

    FlushCompletion event(stripe, iStripeMap, eventScheduler, arrayName);
    bool done = event.Execute();

    if (unlikely(false == done))
    {
        EventSmartPtr eventForSchedule(new FlushCompletion(stripe, arrayName));
        if (likely(eventForSchedule != nullptr))
        {
            eventScheduler->EnqueueEvent(eventForSchedule);
        }
        else
        {
            POS_TRACE_ERROR(static_cast<int>(POS_EVENT_ID::MAP_UPDATE_HANDLER_EVENT_ALLOCATE_FAIL),
                "Failed to allocate flush wrapup event");
            wrapupSuccessful = false;
        }
    }

    return wrapupSuccessful;
}

} // namespace pos

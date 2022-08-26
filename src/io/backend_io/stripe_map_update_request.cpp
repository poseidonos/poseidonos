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

#include "stripe_map_update_request.h"

#include <memory>
#include <sstream>

#include "flush_completion.h"
#include "src/allocator/event/stripe_put_event.h"
#include "src/allocator/stripe/stripe.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/include/backend_event.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/io/backend_io/flush_completion.h"
#include "src/io/backend_io/flush_count.h"
#include "src/logger/logger.h"
#include "src/mapper_service/mapper_service.h"
#include "src/meta_service/meta_service.h"

namespace pos
{
StripeMapUpdateRequest::StripeMapUpdateRequest(Stripe* stripe, int arrayIdInput)
: StripeMapUpdateRequest(stripe, MapperServiceSingleton::Instance()->GetIStripeMap(arrayIdInput),
      MetaServiceSingleton::Instance()->GetMetaUpdater(arrayIdInput),
      EventSchedulerSingleton::Instance(), make_shared<FlushCompletion>(stripe, arrayIdInput), arrayIdInput)
{
}

StripeMapUpdateRequest::StripeMapUpdateRequest(Stripe* stripe, IStripeMap* stripeMap,
    IMetaUpdater* metaUpdater, EventScheduler* eventScheduler, CallbackSmartPtr event, int arrayIdInput)
: Callback(false, CallbackType_StripeMapUpdateRequest),
  stripe(stripe),
  iStripeMap(stripeMap),
  iMetaUpdater(metaUpdater),
  eventScheduler(eventScheduler),
  completionEvent(event)
{
    arrayId = arrayIdInput;
    SetEventType(BackendEvent_Flush);
}

StripeMapUpdateRequest::~StripeMapUpdateRequest(void)
{
}

bool
StripeMapUpdateRequest::_DoSpecificJob(void)
{
    StripeAddr wbStripeAddr = iStripeMap->GetLSA(stripe->GetVsid());
    bool writeBufferArea = iStripeMap->IsInWriteBufferArea(wbStripeAddr);

    POS_EVENT_ID eventId = EID(NFLSH_STRIPE_DEBUG);
    POS_TRACE_DEBUG_IN_MEMORY(ModuleInDebugLogDump::IO_FLUSH, eventId,
        "Stripe Map Update Request : stripe.vsid : {} writeBufferArea : {} wbStripeid : {}",
        stripe->GetVsid(),
        static_cast<uint32_t>(writeBufferArea),
        wbStripeAddr.stripeId);

    if (_GetErrorCount() > 0)
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
        POS_EVENT_ID eventId =
            EID(NFLSH_ERROR_DETECT);
        POS_TRACE_ERROR(static_cast<int>(eventId),
            "Failed to proceed stripe map update request event: {}", _GetErrorCount());
        FlushCountSingleton::Instance()->pendingFlush--;
        FlushCountSingleton::Instance()->callbackNotCalledCount++;
        return true;
    }

    if (unlikely(false == writeBufferArea))
    {
        POS_EVENT_ID eventId = EID(NFLSH_STRIPE_NOT_IN_WRITE_BUFFER);
        POS_TRACE_ERROR(static_cast<int>(eventId),
            "Stripe #{} is not in WriteBuffer.", stripe->GetVsid());
        FlushCountSingleton::Instance()->pendingFlush--;
        FlushCountSingleton::Instance()->callbackNotCalledCount++;
        return true;
    }

    if (unlikely(nullptr == completionEvent))
    {
        POS_EVENT_ID eventId =
            EID(NFLSH_EVENT_ALLOCATION_FAILED);
        std::stringstream message;
        message << "FlushCompletion for vsid: " << stripe->GetVsid() << ", wbLsid: " << stripe->GetWbLsid() << ", userAreaLsid: " << stripe->GetUserLsid();
        POS_TRACE_ERROR(static_cast<int>(eventId),
            "Failed to allocate event: {}", message.str());
        FlushCountSingleton::Instance()->pendingFlush--;
        FlushCountSingleton::Instance()->callbackNotCalledCount++;
        return true;
    }

    eventId = EID(NFLSH_STRIPE_DEBUG_UPDATE);
    POS_TRACE_DEBUG_IN_MEMORY(ModuleInDebugLogDump::IO_FLUSH, eventId, "Stripe Map Update Request : stripe.vsid : {}",
        stripe->GetVsid());

    int result = iMetaUpdater->UpdateStripeMap(stripe, completionEvent);
    if (unlikely(EID(SUCCESS) != result))
    {
        // TODO (dh.ihm) Need to make fatal error (ret < 0) handle path.
        POS_EVENT_ID eventId =
            EID(NFLSH_EVENT_MAP_UPDATE_FAILED);
        std::stringstream message;
        message << "FlushCompletion for vsid: " << stripe->GetVsid() << ", wbLsid: " << stripe->GetWbLsid() << ", userAreaLsid: " << stripe->GetUserLsid();
        POS_TRACE_ERROR(static_cast<int>(eventId),
            "Failed to update map: {}", message.str());

        return false;
    }

    return true;
}
} // namespace pos

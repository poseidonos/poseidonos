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

#include "stripe_map_update_request.h"

#include <sstream>

#include "flush_completion.h"
#include "src/allocator/stripe/stripe.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/include/backend_event.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"
#include "src/mapper_service/mapper_service.h"

namespace pos
{
StripeMapUpdateRequest::StripeMapUpdateRequest(Stripe* stripe, int arrayId)
: StripeMapUpdateRequest(stripe, MapperServiceSingleton::Instance()->GetIStripeMap(arrayId),
      EventSchedulerSingleton::Instance(), EventSmartPtr(new StripeMapUpdate(stripe, arrayId)), arrayId)
{
    SetEventType(BackendEvent_Flush);
}

StripeMapUpdateRequest::StripeMapUpdateRequest(Stripe* stripe, IStripeMap* stripeMap,
    EventScheduler* eventScheduler, EventSmartPtr event, int arrayId)
: Callback(false),
  stripe(stripe),
  iStripeMap(stripeMap),
  eventScheduler(eventScheduler),
  event(event)
{
}

StripeMapUpdateRequest::~StripeMapUpdateRequest(void)
{
}

bool
StripeMapUpdateRequest::_DoSpecificJob(void)
{
    StripeAddr wbStripeAddr = iStripeMap->GetLSA(stripe->GetVsid());
    bool writeBufferArea = iStripeMap->IsInWriteBufferArea(wbStripeAddr);

    POS_EVENT_ID eventId = POS_EVENT_ID::NFLSH_STRIPE_DEBUG;
    POS_TRACE_DEBUG_IN_MEMORY(ModuleInDebugLogDump::IO_FLUSH, eventId, PosEventId::GetString(eventId),
        stripe->GetVsid(),
        static_cast<uint32_t>(writeBufferArea),
        wbStripeAddr.stripeId);

    if (_GetErrorCount() > 0)
    {
        POS_EVENT_ID eventId =
            POS_EVENT_ID::NFLSH_ERROR_DETECT;
        POS_TRACE_ERROR(static_cast<int>(eventId),
            PosEventId::GetString(eventId), _GetErrorCount());
        return true;
    }

    if (unlikely(false == writeBufferArea))
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::NFLSH_STRIPE_NOT_IN_WRITE_BUFFER;
        POS_TRACE_ERROR(static_cast<int>(eventId),
            PosEventId::GetString(eventId), stripe->GetVsid());
        return true;
    }

    if (unlikely(nullptr == event))
    {
        POS_EVENT_ID eventId =
            POS_EVENT_ID::NFLSH_EVENT_ALLOCATION_FAILED;
        std::stringstream message;
        message << "FlushCompletion for vsid: " << stripe->GetVsid() << ", wbLsid: " << stripe->GetWbLsid() << ", userAreaLsid: " << stripe->GetUserLsid();
        POS_TRACE_ERROR(static_cast<int>(eventId),
            PosEventId::GetString(eventId), message.str());
        return true;
    }

    bool mapUpdateSuccessful = event->Execute();

    if (unlikely(false == mapUpdateSuccessful))
    {
        POS_EVENT_ID eventId =
            POS_EVENT_ID::NFLSH_EVENT_MAP_UPDATE_FAILED;
        std::stringstream message;
        message << "FlushCompletion for vsid: " << stripe->GetVsid() << ", wbLsid: " << stripe->GetWbLsid() << ", userAreaLsid: " << stripe->GetUserLsid();
        POS_TRACE_ERROR(static_cast<int>(eventId),
            PosEventId::GetString(eventId), message.str());

        eventScheduler->EnqueueEvent(event);
    }

    return true;
}
} // namespace pos

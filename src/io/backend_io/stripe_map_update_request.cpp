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
#include "src/allocator/stripe.h"
#include "src/include/ibof_event_id.hpp"
#include "src/io/backend_io/stripe_map_update.h"
#include "src/mapper/mapper.h"
#include "src/scheduler/event_argument.h"
#include "src/scheduler/event_scheduler.h"

namespace ibofos
{
StripeMapUpdateRequest::StripeMapUpdateRequest(Stripe* stripe)
: Callback(false),
  stripe(stripe)
{
#if defined QOS_ENABLED_BE
    SetEventType(BackendEvent_Flush);
#endif
}

StripeMapUpdateRequest::~StripeMapUpdateRequest(void)
{
}

bool
StripeMapUpdateRequest::_DoSpecificJob(void)
{
    Mapper& mapper = *MapperSingleton::Instance();
    StripeAddr wbStripeAddr = mapper.GetLSA(stripe->GetVsid());
    bool writeBufferArea = mapper.IsInWriteBufferArea(wbStripeAddr);

    IBOF_EVENT_ID eventId = IBOF_EVENT_ID::NFLSH_STRIPE_DEBUG;
    IBOF_TRACE_DEBUG_IN_MEMORY(ModuleInDebugLogDump::IO_FLUSH, eventId, IbofEventId::GetString(eventId),
        stripe->GetVsid(),
        static_cast<uint32_t>(writeBufferArea),
        wbStripeAddr.stripeId);
    if (_GetErrorCount() > 0)
    {
        IBOF_EVENT_ID eventId =
            IBOF_EVENT_ID::NFLSH_ERROR_DETECT;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId), _GetErrorCount());
        return true;
    }

    if (unlikely(false == writeBufferArea))
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::NFLSH_STRIPE_NOT_IN_WRITE_BUFFER;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId), stripe->GetVsid());
        return true;
    }

    EventSmartPtr event(new StripeMapUpdate(stripe));
    if (unlikely(nullptr == event))
    {
        IBOF_EVENT_ID eventId =
            IBOF_EVENT_ID::NFLSH_EVENT_ALLOCATION_FAILED;
        std::stringstream message;
        message << "FlushCompletion for vsid: " << stripe->GetVsid() << ", wbLsid: " << stripe->GetWbLsid() << ", userAreaLsid: " << stripe->GetUserLsid();
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId), message.str());
        return true;
    }

    bool mapUpdateSuccessful = event->Execute();

    if (unlikely(false == mapUpdateSuccessful))
    {
        IBOF_EVENT_ID eventId =
            IBOF_EVENT_ID::NFLSH_EVENT_MAP_UPDATE_FAILED;
        std::stringstream message;
        message << "FlushCompletion for vsid: " << stripe->GetVsid() << ", wbLsid: " << stripe->GetWbLsid() << ", userAreaLsid: " << stripe->GetUserLsid();
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId), message.str());
        EventArgument::GetEventScheduler()->EnqueueEvent(event);
    }

    return true;
}
} // namespace ibofos

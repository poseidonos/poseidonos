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

#include "src/event_scheduler/spdk_event_scheduler.h"

#include "spdk/env.h"
#include "spdk/event.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/spdk_wrapper/event_framework_api.h"

namespace pos
{
bool
SpdkEventScheduler::SendSpdkEvent(uint32_t core, EventSmartPtr event)
{
    if (unlikely(nullptr == event))
    {
        POS_EVENT_ID eventId = EID(EVENTFRAMEWORK_INVALID_EVENT);
        POS_TRACE_ERROR(eventId, "Invalid Event to send");

        return false;
    }

    EventSmartPtr* argument = new EventSmartPtr(event);

    return EventFrameworkApiSingleton::Instance()->SendSpdkEvent(core, InvokeEvent, argument);
}

void
SpdkEventScheduler::ExecuteOrScheduleEvent(uint32_t core, EventSmartPtr event)
{
    bool done = event->Execute();

    if (done == false)
    {
        SendSpdkEvent(core, event);
    }
}

bool
SpdkEventScheduler::SendSpdkEvent(EventSmartPtr event)
{
    if (unlikely(nullptr == event))
    {
        POS_EVENT_ID eventId = EID(EVENTFRAMEWORK_INVALID_EVENT);
        POS_TRACE_ERROR(eventId, "Invalid Event to send");

        return false;
    }

    EventSmartPtr* argument = new EventSmartPtr(event);

    return EventFrameworkApiSingleton::Instance()->SendSpdkEvent(InvokeEvent, argument);
}

void
SpdkEventScheduler::InvokeEvent(void* voidTypeEvent)
{
    EventSmartPtr event = *static_cast<EventSmartPtr*>(voidTypeEvent);
    uint32_t core = EventFrameworkApiSingleton::Instance()->GetCurrentReactor();
    ExecuteOrScheduleEvent(core, event);
    delete static_cast<EventSmartPtr*>(voidTypeEvent);
}
} // namespace pos


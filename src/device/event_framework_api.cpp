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

#include "src/device/event_framework_api.h"

#include "spdk/env.h"
#include "spdk/event.h"
#include "src/include/branch_prediction.h"
#include "src/include/ibof_event_id.hpp"
#include "src/logger/logger.h"
#include "src/scheduler/event.h"

namespace ibofos
{
thread_local uint32_t EventFrameworkApi::targetReactor = UINT32_MAX;

bool
EventFrameworkApi::SendSpdkEvent(uint32_t core, EventFunc func, void* arg1,
    void* arg2)
{
    spdk_event* event = spdk_event_allocate(core, func, arg1, arg2);
    if (unlikely(nullptr == event))
    {
        IBOF_EVENT_ID eventId =
            IBOF_EVENT_ID::EVENTFRAMEWORK_FAIL_TO_ALLOCATE_EVENT;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId));
        return false;
    }

    bool eventCallSuccess = spdk_event_call_try(event);

    if (unlikely(false == eventCallSuccess))
    {
        IBOF_EVENT_ID eventId =
            IBOF_EVENT_ID::EVENTFRAMEWORK_FAIL_TO_ALLOCATE_EVENT;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId));
        spdk_event_put(event);

        return false;
    }

    return true;
}

bool
EventFrameworkApi::SendSpdkEvent(uint32_t core, EventSmartPtr event)
{
    if (unlikely(nullptr == event))
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::EVENTFRAMEWORK_INVALID_EVENT;
        IBOF_TRACE_ERROR((int)eventId, IbofEventId::GetString(eventId));

        return false;
    }

    EventSmartPtr* argument = new EventSmartPtr(event);

    return SendSpdkEvent(core, _InvokeEvent, argument, nullptr);
}

void
EventFrameworkApi::ExecuteOrScheduleEvent(uint32_t core, EventSmartPtr event)
{
    bool done = event->Execute();

    if (done == false)
    {
        SendSpdkEvent(core, event);
    }
}

void
EventFrameworkApi::_InvokeEvent(void* voidTypeEvent, void* nullArgument)
{
    EventSmartPtr event = *static_cast<EventSmartPtr*>(voidTypeEvent);
    uint32_t core = GetCurrentReactor();
    ExecuteOrScheduleEvent(core, event);
    delete static_cast<EventSmartPtr*>(voidTypeEvent);
}

uint32_t
EventFrameworkApi::GetTargetReactor(void)
{
    targetReactor = spdk_env_get_next_core(targetReactor);
    if (targetReactor == INVALID_CORE)
    {
        targetReactor = spdk_env_get_first_core();
    }
    return targetReactor;
}

uint32_t
EventFrameworkApi::GetNextTargetReactor(uint32_t prevReactor)
{
    uint32_t nextReactor = spdk_env_get_next_core(prevReactor);
    if (nextReactor == INVALID_CORE)
    {
        nextReactor = spdk_env_get_first_core();
    }
    return nextReactor;
}

uint32_t
EventFrameworkApi::GetFirstReactor(void)
{
    return spdk_env_get_first_core();
}

bool
EventFrameworkApi::IsLastReactorNow(void)
{
    return GetCurrentReactor() == spdk_env_get_last_core();
}

uint32_t
EventFrameworkApi::GetCurrentReactor(void)
{
    return spdk_env_get_current_core();
}

uint32_t
EventFrameworkApi::GetNextReactor(void)
{
    uint32_t currentCore = GetCurrentReactor();
    return spdk_env_get_next_core(currentCore);
}

bool
EventFrameworkApi::IsReactorNow(void)
{
    return GetCurrentReactor() != INVALID_CORE;
}

bool
EventFrameworkApi::IsSameReactorNow(uint32_t reactor)
{
    return (reactor == GetCurrentReactor());
}

} // namespace ibofos

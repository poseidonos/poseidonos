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

#include "src/spdk_wrapper/event_framework_api.h"

#include "spdk/env.h"
#include "spdk/event.h"
#include "spdk/nvmf.h"
#include "spdk/pos.h"
#include "spdk/pos_volume.h"
#include "spdk/thread.h"
#include "src/cpu_affinity/affinity_manager.h"
#include "src/include/branch_prediction.h"
#include "src/include/core_const.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"

namespace pos
{
thread_local uint32_t EventFrameworkApi::targetReactor = UINT32_MAX;
const uint32_t EventFrameworkApi::MAX_REACTOR_COUNT;
const uint32_t EventFrameworkApi::MAX_PROCESSABLE_EVENTS = 16;
std::array<EventFrameworkApi::EventQueue,
    EventFrameworkApi::MAX_REACTOR_COUNT>
    EventFrameworkApi::eventQueues;
std::array<EventFrameworkApi::EventQueueLock,
    EventFrameworkApi::MAX_REACTOR_COUNT>
    EventFrameworkApi::eventQueueLocks;

static inline void
EventFuncWrapper(void* ctx)
{
    EventWrapper* eventWrapper = static_cast<EventWrapper*>(ctx);
    eventWrapper->func(eventWrapper->arg1, eventWrapper->arg2);
    delete eventWrapper;
}

EventFrameworkApi::EventFrameworkApi(void)
{
}

EventFrameworkApi::~EventFrameworkApi(void)
{
}

bool
EventFrameworkApi::SendSpdkEvent(uint32_t core, EventFuncFourParams func, void* arg1,
    void* arg2)
{
    EventWrapper* eventWrapper = new EventWrapper;
    eventWrapper->func = (EventFuncTwoParams)func;
    eventWrapper->arg1 = arg1;
    eventWrapper->arg2 = arg2;
    bool success = SendSpdkEvent(core, EventFuncWrapper, eventWrapper);
    return success;
}

bool
EventFrameworkApi::SendSpdkEvent(uint32_t core, EventFuncTwoParams func, void* arg1,
    void* arg2)
{
    EventWrapper* eventWrapper = new EventWrapper;
    eventWrapper->func = func;
    eventWrapper->arg1 = arg1;
    eventWrapper->arg2 = arg2;
    bool success = SendSpdkEvent(core, EventFuncWrapper, eventWrapper);
    return success;
}

bool
EventFrameworkApi::SendSpdkEvent(uint32_t core, EventFuncOneParam func, void* arg1)
{
    if (unlikely(core >= MAX_REACTOR_COUNT))
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::EVENTFRAMEWORK_INVALID_REACTOR;
        POS_TRACE_ERROR(eventId, "Reactor {} is not processable", core);
        return false;
    }

    struct spdk_thread* thread = get_nvmf_thread_from_reactor(core);

    // If nvmf target module is initialized, we can utilize.
    if (unlikely(thread == nullptr))
    {
        POS_EVENT_ID eventId =
            POS_EVENT_ID::EVENTFRAMEWORK_FAIL_TO_ALLOCATE_EVENT;
        POS_TRACE_WARN(eventId, "Spdk Event Not Initialized");
        return false;
    }

    int eventCallSuccess = spdk_thread_send_msg(thread, func, arg1);

    if (0 != eventCallSuccess)
    {
        POS_EVENT_ID eventId =
            POS_EVENT_ID::EVENTFRAMEWORK_FAIL_TO_ALLOCATE_EVENT;
        POS_TRACE_WARN(eventId, "Fail to allocate spdk event");
        _SendEventToSpareQueue(core, func, arg1);
    }
    return true;
}

void
EventFrameworkApi::CompleteEvents(void)
{
    if (IsReactorNow() == false)
    {
        return;
    }

    uint32_t core = GetCurrentReactor();
    std::lock_guard<EventQueueLock> lock(eventQueueLocks[core]);
    EventQueue& eventQueue = eventQueues[core];
    uint32_t processedEvents = 0;
    while (eventQueue.empty() == false)
    {
        EventArgument eventArgument = eventQueue.front();
        EventFuncOneParam func = std::get<0>(eventArgument);
        void* arg1 = std::get<1>(eventArgument);
        func(arg1);
        eventQueue.pop();
        processedEvents++;
        if (processedEvents >= MAX_PROCESSABLE_EVENTS)
        {
            break;
        }
    }
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

void
EventFrameworkApi::_SendEventToSpareQueue(uint32_t core, EventFuncOneParam func,
    void* arg1)
{
    EventArgument eventArgument = std::make_tuple(func, arg1);
    std::lock_guard<EventQueueLock> lock(eventQueueLocks[core]);
    eventQueues[core].push(eventArgument);
}
} // namespace pos

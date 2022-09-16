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
#include "src/master_context/config_manager.h"

namespace pos
{
thread_local uint32_t EventFrameworkApi::targetReactor = UINT32_MAX;
const uint32_t EventFrameworkApi::MAX_REACTOR_COUNT;
const uint32_t EventFrameworkApi::MAX_PROCESSABLE_EVENTS = 16;

static inline void
EventFuncWrapper(void* ctx)
{
    EventWrapper* eventWrapper = static_cast<EventWrapper*>(ctx);
    eventWrapper->func(eventWrapper->arg1, eventWrapper->arg2);
    delete eventWrapper;
}

EventFrameworkApi::EventFrameworkApi(SpdkThreadCaller* spdkThreadCaller,
    SpdkEnvCaller* spdkEnvCaller)
: spdkThreadCaller(spdkThreadCaller),
  spdkEnvCaller(spdkEnvCaller)
{
    bool enable = false;
    int ret = ConfigManagerSingleton::Instance()->GetValue("performance",
        "numa_dedicated", &enable, CONFIG_TYPE_BOOL);
    numaDedicatedSchedulingPolicy = false;
    if (ret == EID(SUCCESS))
    {
        numaDedicatedSchedulingPolicy = enable;
    }
}

EventFrameworkApi::~EventFrameworkApi(void)
{
    if (spdkThreadCaller != nullptr)
    {
        delete spdkThreadCaller;
    }
    if (spdkEnvCaller != nullptr)
    {
        delete spdkEnvCaller;
    }
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
        POS_EVENT_ID eventId = EID(EVENTFRAMEWORK_INVALID_REACTOR);
        POS_TRACE_ERROR(eventId, "Reactor {} is not processable", core);
        return false;
    }
    _SendEventToSpareQueue(core, func, arg1);
    return true;
}

void
EventFrameworkApi::_SendEventToSingleQueue(EventFuncOneParam func, void* arg1)
{
    EventArgument eventArgument = std::make_tuple(func, arg1);
    uint32_t numaIndex = 0;
    if (numaDedicatedSchedulingPolicy)
    {
        numaIndex = AffinityManagerSingleton::Instance()->GetNumaIdFromCurrentThread();
    }
    eventSingleQueue[numaIndex].push(eventArgument);
}

bool
EventFrameworkApi::SendSpdkEvent(EventFuncOneParam func, void* arg1)
{
    _SendEventToSingleQueue(func, arg1);
    return true;
}

bool
EventFrameworkApi::CompleteEvents(void)
{
    if (IsReactorNow() == false)
    {
        return false;
    }

    uint32_t core = GetCurrentReactor();
    EventQueue& eventQueue = eventQueues[core];
    uint32_t processedEvents = 0;
    EventArgument eventArgument;
    while (eventQueue.try_pop(eventArgument) == true)
    {
        EventFuncOneParam func = std::get<0>(eventArgument);
        void* arg1 = std::get<1>(eventArgument);
        func(arg1);
        processedEvents++;
        if (processedEvents >= MAX_PROCESSABLE_EVENTS)
        {
            break;
        }
    }
    if (eventQueue.empty() == true)
    {
	    return true;
    }
    return false;
}

bool
EventFrameworkApi::CompleteSingleQueueEvents(void)
{
    if (IsReactorNow() == false)
    {
        return false;
    }
    uint32_t numaIndex = 0;
    if (numaDedicatedSchedulingPolicy)
    {
        numaIndex = AffinityManagerSingleton::Instance()->GetNumaIdFromCurrentThread();
    }
    EventQueue& eventQueue = eventSingleQueue[numaIndex];
    uint32_t processedEvents = 0;
    EventArgument eventArgument;
    while (eventQueue.try_pop(eventArgument) == true)
    {
        EventFuncOneParam func = std::get<0>(eventArgument);
        void* arg1 = std::get<1>(eventArgument);
        func(arg1);
        processedEvents++;
        if (processedEvents >= MAX_PROCESSABLE_EVENTS)
        {
            break;
        }
    }
    if (eventQueue.empty() == true)
    {
        return true;
    }
    return false;
}

uint32_t
EventFrameworkApi::GetFirstReactor(void)
{
    return spdkEnvCaller->SpdkEnvGetFirstCore();
}

bool
EventFrameworkApi::IsLastReactorNow(void)
{
    return GetCurrentReactor() == spdkEnvCaller->SpdkEnvGetLastCore();
}

uint32_t
EventFrameworkApi::GetCurrentReactor(void)
{
    return spdkEnvCaller->SpdkEnvGetCurrentCore();
}

uint32_t
EventFrameworkApi::GetNextReactor(void)
{
    uint32_t currentCore = GetCurrentReactor();
    return spdkEnvCaller->SpdkEnvGetNextCore(currentCore);
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
    eventQueues[core].push(eventArgument);
}
} // namespace pos

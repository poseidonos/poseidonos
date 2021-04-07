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

#pragma once

#include <stdint.h>

#include <atomic>

#include "src/scheduler/event.h"

namespace ibofos
{
using EventFunc = void (*)(void*, void*);
class Event;

class EventFrameworkApi
{
public:
    static bool
    SendSpdkEvent(uint32_t core, EventFunc func, void* arg1, void* arg2);
    static bool SendSpdkEvent(uint32_t core, EventSmartPtr event);
    static uint32_t GetTargetReactor(void);
    static uint32_t GetNextTargetReactor(uint32_t prevReactor);
    static uint32_t GetFirstReactor(void);
    static uint32_t GetCurrentReactor(void);
    static uint32_t GetNextReactor(void);
    static bool IsReactorNow(void);
    static bool IsLastReactorNow(void);
    static bool IsSameReactorNow(uint32_t reactor);

    template<class T, typename... Args>
    static void
    CreateAndExecuteOrScheduleEvent(uint32_t core, const Args&... args)
    {
        T event(args...);
        bool done = event.Execute();

        if (done == false)
        {
            EventSmartPtr event = std::make_shared<T>(args...);
            SendSpdkEvent(core, event);
        }
    }

    static void ExecuteOrScheduleEvent(uint32_t core, EventSmartPtr event);

    static const uint32_t INVALID_CORE = UINT32_MAX;

private:
    static thread_local uint32_t targetReactor;
    static std::atomic<uint32_t> transmissionCount;

    static void _InvokeEvent(void* voidTypeEvent, void* nullArgument);
};
} // namespace ibofos

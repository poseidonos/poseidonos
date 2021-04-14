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
#include <array>
#include <mutex>
#include <queue>
#include <tuple>
#include <string>

namespace pos
{
using EventFuncTwoParams = void (*)(void*, void*);
using EventFuncOneParam = void (*)(void*);
using SpdkPollerFunction = int (*)(void*);

struct EventWrapper
{
    EventFuncTwoParams func;
    void *arg1;
    void *arg2;
};

class EventFrameworkApi
{
public:
    static bool SendSpdkEvent(uint32_t core, EventFuncTwoParams func, void* arg1,
            void* arg2);
    static bool SendSpdkEvent(uint32_t core, EventFuncOneParam func, void* arg1);
    static void CompleteEvents(void);

    static uint32_t GetTargetReactor(void);
    static uint32_t GetNextTargetReactor(uint32_t prevReactor);
    static uint32_t GetFirstReactor(void);
    static uint32_t GetCurrentReactor(void);
    static uint32_t GetNextReactor(void);
    static bool IsReactorNow(void);
    static bool IsLastReactorNow(void);
    static bool IsSameReactorNow(uint32_t reactor);
    static uint32_t GetMaxReactor(void);
    static void SpdkNvmfInitializeReactorSubsystemMapping(void);
    static uint32_t SpdkNvmfGetReactorSubsystemMapping(uint32_t reactor, uint32_t id);
    static void SpdkPollerUnregister(void* poller);
    static uint64_t SpdkGetTicksHz(void);
    static uint64_t SpdkGetTicks(void);
    static void* SpdkPollerRegister(SpdkPollerFunction func, void* arg, uint64_t period_microseconds, std::string pollerName);
    static uint32_t GetAttachedSubsystemId(const char* bdev_name);

private:
    static thread_local uint32_t targetReactor;

    using EventArgument = std::tuple<EventFuncOneParam, void* >;
    using EventQueue = std::queue<EventArgument>;
    using EventQueueLock = std::recursive_mutex;

    static const uint32_t MAX_REACTOR_COUNT = 256;
    static const uint32_t MAX_PROCESSABLE_EVENTS;

    static std::array<EventQueue, MAX_REACTOR_COUNT> eventQueues;
    static std::array<EventQueueLock, MAX_REACTOR_COUNT> eventQueueLocks;

    static void _SendEventToSpareQueue(uint32_t core, EventFuncOneParam func, void* arg1);
};
} // namespace pos

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

#pragma once

#include <atomic>

#include "src/event_scheduler/event.h"
#include "src/include/smart_ptr_type.h"

namespace std
{
class thread;
}   // namespace std

namespace pos
{
class EventQueue;
class EventScheduler;

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Execute Event from EventScheduler
 *           Affinited to the set of cores
 */
/* --------------------------------------------------------------------------*/
class EventWorker
{
public:
    EventWorker(cpu_set_t eventCPUPoolInput,
        EventScheduler* eventSchedulerInput, uint32_t id);
    ~EventWorker(void);

    void EnqueueEvent(EventSmartPtr Input);
    EventSmartPtr DequeueEvent();
    uint32_t GetQueueSize(void);
    void Run(void);

private:
    EventQueue* eventQueue;
    std::thread* thread;
    std::atomic<bool> exit;
    cpu_set_t eventCPUPool;
    EventScheduler* eventScheduler;
    uint32_t id;
    std::atomic<bool> running;
};
} // namespace pos

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

#include "src/event_scheduler/event_worker.h"

#include <unistd.h>

#include <iomanip>
#include <thread>

#include "src/event_scheduler/event.h"
#include "src/event_scheduler/event_queue.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/qos/qos_manager.h"
#include "src/event_scheduler/backend_policy.h"

// #pragma GCC optimize("O0")

namespace pos
{
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Constructor
 *           Each object owns a thread affinited to set of cores
 *
 * @Param    eventCPUPoolInput
 * @Param    eventSchedulerInput
 */
/* --------------------------------------------------------------------------*/
EventWorker::EventWorker(cpu_set_t eventCPUPoolInput,
    EventScheduler* eventSchedulerInput, uint32_t id)
: eventQueue(new EventQueue),
  thread(nullptr),
  exit(false),
  eventCPUPool(eventCPUPoolInput),
  eventScheduler(eventSchedulerInput),
  id(id),
  running(false)
{
    thread = new std::thread(&EventWorker::Run, this);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Destructor
 */
/* --------------------------------------------------------------------------*/
EventWorker::~EventWorker(void)
{
    exit = true;
    thread->join();
    delete eventQueue;
    delete thread;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Enqueue event for worker
 *           SPSC queue (EventScheduler -> EventWorker)
 *
 * @Param    pobjInput
 */
/* --------------------------------------------------------------------------*/
void
EventWorker::EnqueueEvent(EventSmartPtr input)
{
    eventQueue->EnqueueEvent(input);
}

EventSmartPtr
EventWorker::DequeueEvent()
{
    return eventQueue->DequeueEvent();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Big loop
 *           Busy wait events
 *           Reschedule events to EventScheduler if needed
 */
/* --------------------------------------------------------------------------*/
void
EventWorker::Run(void)
{
    std::ostringstream name;
    name << "EventWorker" << id;
    pthread_setname_np(pthread_self(), name.str().c_str());
    sched_setaffinity(0, sizeof(eventCPUPool), &eventCPUPool);

    while (exit == false)
    {
        EventSmartPtr event = eventScheduler->PickWorkerEvent(this);
        if (nullptr == event)
        {
            usleep(1);
            continue;
        }
        running = true;
        bool done = event->Execute();
        eventScheduler->CheckAndSetQueueOccupancy(event->GetEventType());
        running = false;
        if (done == false)
        {
            eventScheduler->EnqueueEvent(event);
        }
    }
}

uint32_t
EventWorker::GetQueueSize(void)
{
    return eventQueue->GetQueueSize() + static_cast<uint32_t>(running);
}

} // namespace pos

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

#include "backend_event_minimum_policy.h"

#include <assert.h>
#include <unistd.h>

#include "src/event_scheduler/event.h"
#include "src/event_scheduler/event_queue.h"
#include "src/event_scheduler/event_worker.h"
#include "src/event_scheduler/scheduler_queue.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"
#include "src/qos/qos_manager.h"

namespace pos
{
BackendEventMinimumPolicy::BackendEventMinimumPolicy(
    QosManager* qosManager,
    std::vector<EventWorker*>* workerArray, uint32_t workerCount, uint32_t ioWorkerCount)
: BackendPolicy(qosManager, workerArray, workerCount, ioWorkerCount)
{
    cyclesElapsed = 0;
    for (unsigned int event = 0; (BackendEvent)event < BackendEvent_Count; event++)
    {
        queueOccupied[event] = false;
    }
}

BackendEventMinimumPolicy::~BackendEventMinimumPolicy()
{
}

void
BackendEventMinimumPolicy::EnqueueEvent(EventSmartPtr input)
{
    std::unique_lock<std::mutex> uniqueLock(queueLock[input->GetEventType()]);
    if (true == input->IsFrontEnd())
    {
        eventQueue[BackendEvent_FrontendIO]->EnqueueEvent(input);
    }
    else
    {
        switch (input->GetEventType())
        {
            case BackendEvent_Flush:
                eventQueue[BackendEvent_Flush]->EnqueueEvent(input);
                break;
            case BackendEvent_GC:
                eventQueue[BackendEvent_GC]->EnqueueEvent(input);
                break;
            case BackendEvent_UserdataRebuild:
                eventQueue[BackendEvent_UserdataRebuild]->EnqueueEvent(input);
                break;
            case BackendEvent_JournalIO:
                eventQueue[BackendEvent_JournalIO]->EnqueueEvent(input);
                break;
            case BackendEvent_MetaIO:
                eventQueue[BackendEvent_MetaIO]->EnqueueEvent(input);
                break;
            case BackendEvent_FrontendIO:
                eventQueue[BackendEvent_FrontendIO]->EnqueueEvent(input);
                break;
            case BackendEvent_Unknown:
                eventQueue[BackendEvent_Unknown]->EnqueueEvent(input);
                break;
            default:
                assert(0);
                break;
        }
    }
    CheckAndSetQueueOccupancy(input->GetEventType());
}

std::queue<EventSmartPtr>
BackendEventMinimumPolicy::DequeueEvents(void)
{
    EventSmartPtr event = nullptr;
    std::queue<EventSmartPtr> eventList;
    int64_t eventWeight = 0;
    for (uint32_t eventType = BackendEvent_Start; (BackendEvent)eventType < BackendEvent_Count; eventType++)
    {
        {
            std::unique_lock<std::mutex> uniqueLock(queueLock[(BackendEvent)eventType]);
            if ((BackendEvent)eventType == BackendEvent_FrontendIO)
            {
                do
                {
                    event = eventQueue[BackendEvent_FrontendIO]->DequeueEvent();
                    if (nullptr != event)
                    {
                        eventList.push(event);
                    }
                } while (nullptr != event);
                CheckAndSetQueueOccupancy(BackendEvent_FrontendIO);
            }
            else
            {
                eventWeight = _GetEventWeight((BackendEvent)eventType);
                if (eventWeight < 0)
                {
                    if (eventWeight != oldWeight[(BackendEvent)eventType])
                    {
                        runningWeight[(BackendEvent)eventType] = eventWeight;
                        oldWeight[(BackendEvent)eventType] = eventWeight;
                    }
                    runningWeight[(BackendEvent)eventType]++;
                    if (runningWeight[(BackendEvent)eventType] != 0)
                    {
                        continue;
                    }
                    else
                    {
                        eventWeight = 0;
                    }
                }
                else
                {
                    oldWeight[(BackendEvent)eventType] = eventWeight;
                }

                uint32_t currentWeight = 0;
                do
                {
                    event = eventQueue[eventType]->DequeueEvent();
                    if (nullptr != event)
                    {
                        eventList.push(event);
                        currentWeight++;
                    }
                } while ((nullptr != event) && (currentWeight < eventWeight));
                runningWeight[(BackendEvent)eventType] = oldWeight[(BackendEvent)eventType];
                CheckAndSetQueueOccupancy((BackendEvent)eventType);
            }
        }
    }
    return eventList;
}

uint32_t
BackendEventMinimumPolicy::_GetWorkerIDMinimumJobs(uint32_t numa)
{
    const uint32_t MAX_VALUE = UINT32_MAX;
    uint32_t minimumJobs = MAX_VALUE, minimumWorkerID = 0;
    std::vector<uint32_t>& workerIDVector = workerIDPerNumaVector[numa];

    if (!numaDedicatedSchedulingPolicy || workerIDPerNumaVector[numa].size() == 0)
    {
        workerIDVector = totalWorkerIDVector;
    }

    // In this case, we just try linear search.
    // The number of Event Worker is not so huge, so linear search does not have performance impact comparatively.
    // If performance Issue happens, we can change to "heap tree" or another effective algorithm.

    for (auto workerID : workerIDVector)
    {
        uint32_t size = (*workerArray)[workerID]->GetQueueSize();
        if (minimumJobs > size)
        {
            minimumJobs = size;
            minimumWorkerID = workerID;
        }
    }
    return minimumWorkerID;
}

bool
BackendEventMinimumPolicy::_CheckContention(BackendEvent eventId)
{
    bool frontendQueueOccupied = false;
    bool backendFlushQueueOccupied = false;
    bool contention = false;
    if (eventId == (BackendEvent)BackendEvent_UserdataRebuild)
    {
        frontendQueueOccupied = _GetQueueOccupancy(BackendEvent_FrontendIO);
        backendFlushQueueOccupied = _GetQueueOccupancy((BackendEvent)BackendEvent_Flush);
        contention = frontendQueueOccupied || backendFlushQueueOccupied;
    }
    return contention;
}

void
BackendEventMinimumPolicy::CheckAndSetQueueOccupancy(BackendEvent eventId)
{
    if (eventQueue[eventId]->GetQueueSize() == 0)
        queueOccupied[eventId] = false;
    else
        queueOccupied[eventId] = true;
}

bool
BackendEventMinimumPolicy::_NoContentionCycleDone(uint32_t cycles)
{
    bool cycleDone = false;

    // Check how many cycles have passed with no contention;
    if (cyclesElapsed / cycles >= 1)
    {
        cycleDone = true;
    }

    return cycleDone;
}

void
BackendEventMinimumPolicy::_IncrementCycles(void)
{
    cyclesElapsed++;
    uint32_t cycles = qosManager->GetNoContentionCycles();
    if (cyclesElapsed / cycles >= 1)
    {
        cyclesElapsed = cycles;
    }
}

int32_t
BackendEventMinimumPolicy::_GetEventWeight(BackendEvent eventId)
{
    int32_t eventWeight = qosManager->GetEventWeightWRR((BackendEvent)eventId);
    bool contention = _CheckContention(eventId);
    if (contention == true)
    {
        eventWeight = qosManager->GetEventWeightWRR((BackendEvent)eventId);
        cyclesElapsed = 0;
    }
    else
    {
        uint32_t noContentionCycles = qosManager->GetNoContentionCycles();
        if (_NoContentionCycleDone(noContentionCycles) == true)
        {
            eventWeight = qosManager->GetDefaultEventWeightWRR((BackendEvent)eventId);
        }
        _IncrementCycles();
    }
    return eventWeight;
}

int
BackendEventMinimumPolicy::Run(void)
{
    std::queue<EventSmartPtr> eventList = DequeueEvents();
    if (eventList.empty())
    {
        return 1;
    }

    uint32_t workerID = 0;
    EventSmartPtr event = nullptr;
    while (!eventList.empty())
    {
        event = eventList.front();
        workerID = _GetWorkerIDMinimumJobs(event->GetNumaId());
        if (unlikely(workerID >= workerCount))
        {
            POS_TRACE_WARN(EID(EVTSCHDLR_INVALID_WORKER_ID), "");
            return 1;
        }
        (*workerArray)[workerID]->EnqueueEvent(event);
        eventList.pop();
    }
    return 0;
}

EventSmartPtr
BackendEventMinimumPolicy::PickWorkerEvent(EventWorker* worker)
{
    return worker->DequeueEvent();
}

} // namespace pos

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

#include "backend_event_ratio_policy.h"

#include <air/Air.h>
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
BackendEventRatioPolicy::BackendEventRatioPolicy(
    QosManager* qosManager,
    std::vector<EventWorker*>* workerarray, uint32_t workerCount, uint32_t ioWorkerCount)
: BackendPolicy(qosManager, workerArray, workerCount, ioWorkerCount)
{
    totalEventCount = MAX_EVENTS_PER_EVENT_WORKER * workerCount;
    totalIOCount = MAX_IO_COUNT * ioWorkerCount;
    _InitLimitValues();
    outstandingFrontendIO = 0;
    for (unsigned int event = 0; (BackendEvent)event < BackendEvent_Count; event++)
    {
        queueOccupied[event] = false;
        cycleCount[event] = 0;
        currentEventCount[event] = 0;
        ioControl.currentIOCount[event] = 0;
    }
    for (uint32_t qId = 0; qId < FE_QUEUES; qId++)
    {
        feEventQueue[qId] = new SchedulerQueue{qosManager};
    }
}

BackendEventRatioPolicy::~BackendEventRatioPolicy()
{
    for (uint32_t qId = 0; qId < FE_QUEUES; qId++)
    {
        delete feEventQueue[qId];
        feEventQueue[qId] = nullptr;
    }
}

void
BackendEventRatioPolicy::EnqueueEvent(EventSmartPtr input)
{
    if (input->GetEventType() == BackendEvent_FrontendIO)
    {
        uint32_t queueId = rand() % FE_QUEUES;
        ++outstandingFrontendIO;
        std::unique_lock<std::mutex> uniqueLock(frontendQueueLock[queueId]);
        feEventQueue[queueId]->EnqueueEvent(input);
        CheckAndSetQueueOccupancy(input->GetEventType());
    }
    else
    {
        std::unique_lock<std::mutex> uniqueLock(queueLock[input->GetEventType()]);
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
            case BackendEvent_Unknown:
                eventQueue[BackendEvent_Unknown]->EnqueueEvent(input);
                break;
            default:
                assert(0);
                break;
        }
        CheckAndSetQueueOccupancy(input->GetEventType());
    }
    airlog("EventQueue_Push", "internal", input->GetEventType(), 1);
}

std::queue<EventSmartPtr>
BackendEventRatioPolicy::DequeueEvents(void)
{
    EventSmartPtr event = nullptr;
    std::queue<EventSmartPtr> eventList;
    for (uint32_t eventType = BackendEvent_Start; (BackendEvent)eventType < BackendEvent_Count; eventType++)
    {
        int32_t popCount = allowedEventCount[(BackendEvent)eventType] - currentEventCount[eventType];
        int32_t actualCount = 0;
        if ((BackendEvent)eventType == BackendEvent_FrontendIO)
        {
            uint32_t popCountReplenish = popCount;
            {
                do
                {
                    uint32_t qId = rand() % FE_QUEUES;
                    std::unique_lock<std::mutex> uniqueLock(frontendQueueLock[qId]);
                    event = feEventQueue[qId]->DequeueEvent();
                    if (nullptr != event)
                    {
                        outstandingFrontendIO--;
                        eventList.push(event);
                        popCountReplenish--;
                        actualCount++;
                    }
                } while (nullptr != event && popCountReplenish > 0);
            }
            currentEventCount[eventType] += std::min(actualCount, popCount);
        }
        else
        {
            std::unique_lock<std::mutex> uniqueLock(queueLock[(BackendEvent)eventType]);
            while (popCount > 0 && eventQueue[eventType]->GetQueueSize() > 0)
            {
                event = eventQueue[eventType]->DequeueEvent();
                if (nullptr != event)
                {
                    eventList.push(event);
                    popCount--;
                    actualCount++;
                }
            }
            currentEventCount[eventType] += actualCount;
        }
    }
    return eventList;
}

EventSmartPtr
BackendEventRatioPolicy::DequeueWorkerEvent(void)
{
    // Caller holds the lock
    uint32_t q_size = workerCommonQueue.size();
    airlog("Q_EventQueue", "base", 0, q_size);
    if (workerCommonQueue.empty())
    {
        return nullptr;
    }

    EventSmartPtr event = workerCommonQueue.front();
    workerCommonQueue.pop();
    airlog("WorkerCommonQueue_Pop", "internal", event->GetEventType(), 1);
    return event;
}

uint32_t
BackendEventRatioPolicy::GetWorkerQueueSize()
{
    // Caller holds the lock
    return workerCommonQueue.size();
}

void
BackendEventRatioPolicy::CheckAndSetQueueOccupancy(BackendEvent eventId)
{
    bool oldValue = queueOccupied[eventId];
    if (eventId == BackendEvent_FrontendIO || eventId == BackendEvent_Start)
    {
        queueOccupied[eventId] = (outstandingFrontendIO > 0);
    }
    else if (eventQueue[eventId]->GetQueueSize() == 0)
    {
        queueOccupied[eventId] = false;
    }
    else
    {
        queueOccupied[eventId] = true;
    }
    if (oldValue != queueOccupied[eventId])
    {
        _CheckAndSetAllowedLimit();
    }
}

void
BackendEventRatioPolicy::_InitLimitValues(void)
{
    for (uint32_t eventType = BackendEvent_Start; (BackendEvent)eventType < BackendEvent_Count; eventType++)
    {
        allowedEventCount[eventType] = totalEventCount / BackendEvent_Count;
        ioControl.allowedIOCount[eventType] = totalIOCount / BackendEvent_Count;
    }
}

void
BackendEventRatioPolicy::_CheckAndSetAllowedLimit(void)
{
    uint32_t totalLimit = 0;
    for (uint32_t eventType = BackendEvent_Start; (BackendEvent)eventType < BackendEvent_Count; eventType++)
    {
        bool tqueueOccupied = _GetQueueOccupancy((BackendEvent)eventType);

        if (tqueueOccupied == false)
        {
            cycleCount[eventType]++;
            if (cycleCount[eventType] > NO_CONTENTION_CYCLES)
            {
                // Wait for some cycles to switch the priority back to default;
                limit[eventType] = FALLBACK_WEIGHT;
                cycleCount[eventType] = 0;
            }
        }
        else
        {
            limit[eventType] = _GetEventLimit((BackendEvent)eventType);
            cycleCount[eventType] = 0;
        }
        totalLimit += limit[eventType];
    }
    if (totalLimit != 0)
    {
        for (uint32_t eventType = BackendEvent_Start; (BackendEvent)eventType < BackendEvent_Count; eventType++)
        {
            ioControl.allowedIOCount[eventType] = (totalIOCount * limit[eventType]) / totalLimit;
            allowedEventCount[eventType] = (totalEventCount * limit[eventType]) / totalLimit;
            if (allowedEventCount[eventType] == 0)
            {
                allowedEventCount[eventType] = 1;
            }
            if (ioControl.allowedIOCount[eventType] == 0)
            {
                ioControl.allowedIOCount[eventType] = 1;
            }
        }
    }
}

int32_t
BackendEventRatioPolicy::_GetEventLimit(BackendEvent eventId)
{
    return weightArr[qosManager->GetBackendPolicy(eventId).priorityImpact];
}

int
BackendEventRatioPolicy::Run(void)
{
    std::queue<EventSmartPtr> eventList = DequeueEvents();
    if (unlikely(eventList.empty()))
    {
        return QosReturnCode::FAILURE;
    }
    EventSmartPtr event = nullptr;
    workerQueueLock.lock();
    while (!eventList.empty())
    {
        event = eventList.front();
        eventList.pop();
        currentEventCount[event->GetEventType()]--;
        airlog("WorkerCommonQueue_Push", "internal", event->GetEventType(), 1);
        workerCommonQueue.push(event);
    }
    workerQueueLock.unlock();
    return QosReturnCode::SUCCESS;
}

EventSmartPtr
BackendEventRatioPolicy::PickWorkerEvent(EventWorker* worker)
{
    workerQueueLock.lock();
    EventSmartPtr event = DequeueWorkerEvent();
    workerQueueLock.unlock();
    return event;
}

} // namespace pos

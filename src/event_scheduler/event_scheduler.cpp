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

#include "src/event_scheduler/event_scheduler.h"

#include <assert.h>
#include <unistd.h>

#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/cpu_affinity/affinity_manager.h"
#include "src/logger/logger.h"
#include "src/event_scheduler/event.h"
#include "src/event_scheduler/event_queue.h"
#include "src/event_scheduler/event_worker.h"
#include "src/event_scheduler/minimum_job_policy.h"
#include "src/event_scheduler/scheduler_queue.h"

#if defined QOS_ENABLED_BE
#include "src/qos/qos_manager.h"
#endif

namespace pos
{
EventScheduler::EventScheduler(void)
: policy(nullptr),
  exit(false),
  workerCount(UINT32_MAX),
  schedulerThread(nullptr)
{
    CPU_ZERO(&schedulerCPUSet);
#if defined QOS_ENABLED_BE
    for (unsigned int event = 0; (BackendEvent)event < BackendEvent_Count;
            event++)
    {
        eventQueue[event] = new SchedulerQueue;
    }
#else
    eventQueue = new EventQueue;
#endif
}


uint32_t EventScheduler::GetWorkerIDMinimumJobs(void)
{
    const uint32_t MAX_VALUE = UINT32_MAX;
    uint32_t minimumJobs = MAX_VALUE, minimumWorkerID = 0;
    // In this case, we just trys linear search.
    // The number of Event Worker is not so huge, so linear search is comparatively well.
    // If Performance Issue is triggered, we can change to "heap tree" or another effective algorithm.
    for (uint32_t workerID = 0; workerID < workerCount; workerID++)
    {
        uint32_t size = workerArray[workerID]->GetQueueSize();
        if (minimumJobs > size)
        {
            minimumJobs = size;
            minimumWorkerID = workerID;
        }
    }
    return minimumWorkerID;
}

void
EventScheduler::Initialize(uint32_t workerCountInput,
        cpu_set_t schedulerCPUInput, cpu_set_t eventCPUSetInput)
{
    policy = new MinimumJobPolicy(workerCountInput);
    workerCount = workerCountInput;
    workerArray.resize(workerCountInput);
    schedulerCPUSet = schedulerCPUInput;
    _BuildCpuSet(eventCPUSetInput);

    for (unsigned int workerID = 0; workerID < workerCount; workerID++)
    {
        workerArray[workerID] =
            new EventWorker(cpuSetVector.at(workerID), this, workerID);
    }
    schedulerThread = new std::thread(&EventScheduler::Run, this);
}

EventScheduler::~EventScheduler(void)
{
    exit = true;
    if (nullptr != schedulerThread)
    {
        schedulerThread->join();
    }
#if defined QOS_ENABLED_BE
    for (unsigned int event = 0; (BackendEvent)event < BackendEvent_Count;
            event++)
    {
        delete eventQueue[event];
    }
#else
    delete eventQueue;
#endif
    if (nullptr != schedulerThread)
    {
        delete schedulerThread;
    }

    for (auto eventWorker : workerArray)
    {
        delete eventWorker;
    }

    if (nullptr != policy)
    {
        delete policy;
    }
}

void
EventScheduler::_BuildCpuSet(cpu_set_t& cpuSet)
{
    uint32_t totalCore = AffinityManagerSingleton::Instance()->GetTotalCore();

    uint32_t cpuIndex = 0;
    for (unsigned int workerID = 0; workerID < workerCount; workerID++)
    {
        while (!CPU_ISSET(cpuIndex, &cpuSet))
        {
            cpuIndex++;
            if (unlikely(cpuIndex >= totalCore))
            {
                POS_EVENT_ID eventId =
                    POS_EVENT_ID::EVENTSCHEDULER_NOT_MATCH_WORKER_COUNT;
                POS_TRACE_ERROR(static_cast<int>(eventId),
                    PosEventId::GetString(eventId));
                assert(0);
            }
        }

        cpu_set_t tempCpuSet;
        CPU_ZERO(&tempCpuSet);
        CPU_SET(cpuIndex, &tempCpuSet);
        cpuSetVector.push_back(tempCpuSet);
        cpuIndex = cpuIndex + 1;
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Big loop of EvntScheduler
 *           Get worker ID from policy and event form EventQueue
 *           Pass event pointer to event worker
 */
/* --------------------------------------------------------------------------*/
void
EventScheduler::Run(void)
{
    pthread_setname_np(pthread_self(), "EventScheduler");
    sched_setaffinity(0, sizeof(cpu_set_t), &schedulerCPUSet);

    // Scheduler Big Loop
    // Select thread according to SchedulerPolicy to execute event in Event Queue
    while (false == exit)
    {
#if defined QOS_ENABLED_BE
        std::queue<EventSmartPtr> eventList = DequeueEvents();
        if (eventList.empty())
        {
            usleep(1);
            continue;
        }
        uint32_t workerID = 0;
        EventSmartPtr event = nullptr;
        while (!eventList.empty())
        {
            event = eventList.front();
            workerID = policy->GetProperWorkerID();
            if (unlikely(workerID >= workerCount))
            {
                PosEventId::Print(POS_EVENT_ID::EVTSCHDLR_INVALID_WORKER_ID,
                    EventLevel::WARNING);
                usleep(1); // Added sleep for event worker to be available
                continue;
            }
            workerArray[workerID]->EnqueueEvent(event);
            eventList.pop();
        }
#else
        EventSmartPtr event = eventQueue->DequeueEvent();
        if (unlikely(nullptr == event))
        {
            usleep(1);
            continue;
        }

        uint32_t workerID = policy->GetProperWorkerID();
        if (unlikely(workerID >= workerCount))
        {
            PosEventId::Print(POS_EVENT_ID::EVTSCHDLR_INVALID_WORKER_ID,
                EventLevel::WARNING);
            EnqueueEvent(event);
            continue;
        }

        workerArray[workerID]->EnqueueEvent(event);
#endif
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Enqueue event for scheduling
 *           MPSC queue (Workers -> EventScheduler)
 *
 * @Param    input
 */
/* --------------------------------------------------------------------------*/
void
EventScheduler::EnqueueEvent(EventSmartPtr input)
{
#if defined QOS_ENABLED_BE
    if (input->GetEventType() == BackendEvent_Unknown)
    {
        input->SetEventType(BackendEvent_FrontendIO);
    }
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
            case BackendEvent_MetadataRebuild:
                eventQueue[BackendEvent_MetadataRebuild]->EnqueueEvent(input);
                break;
            case BackendEvent_MetaIO:
                eventQueue[BackendEvent_MetaIO]->EnqueueEvent(input);
                break;
            case BackendEvent_FrontendIO:
                eventQueue[BackendEvent_FrontendIO]->EnqueueEvent(input);
                break;
            case BackendEvent_Unknown:
                eventQueue[BackendEvent_FrontendIO]->EnqueueEvent(input);
                break;
            default:
                assert(0);
                break;
        }
    }

#else
    eventQueue->EnqueueEvent(input);
#endif
}
#if defined QOS_ENABLED_BE
/* --------------------------------------------------------------------------*/
/**
  * @Synopsis
  *
  * @Returns
  */
/* --------------------------------------------------------------------------*/
std::queue<EventSmartPtr>
EventScheduler::DequeueEvents(void)
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
            }
            else
            {
                eventWeight = QosManagerSingleton::Instance()->GetEventWeightWRR((BackendEvent)eventType);
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
            }
        }
    }
    return eventList;
}
#endif
} // namespace pos

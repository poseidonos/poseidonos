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

#include "src/event_scheduler/event_scheduler.h"

#include <assert.h>
#include <unistd.h>

#include <stdexcept>

#include "src/cpu_affinity/affinity_manager.h"
#include "src/event_scheduler/event.h"
#include "src/event_scheduler/event_queue.h"
#include "src/event_scheduler/event_worker.h"
#include "src/event_scheduler/minimum_job_policy.h"
#include "src/event_scheduler/scheduler_queue.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"
#include "src/master_context/config_manager.h"
#include "src/qos/qos_manager.h"

namespace pos
{
EventScheduler::EventScheduler(QosManager* qosManagerArg,
    ConfigManager* configManagerArg,
    AffinityManager* affinityManagerArg)
: policy(nullptr),
  exit(false),
  workerCount(UINT32_MAX),
  schedulerThread(nullptr),
  numaDedicatedSchedulingPolicy(false),
  qosManager(qosManagerArg),
  configManager(configManagerArg),
  affinityManager(affinityManagerArg)
{
    CPU_ZERO(&schedulerCPUSet);
    bool enable = false;

    if (nullptr == affinityManager)
    {
        affinityManager = AffinityManagerSingleton::Instance();
    }

    if (nullptr == configManager)
    {
        configManager = ConfigManagerSingleton::Instance();
    }

    // We fix the name of config as default
    int ret = configManager->GetValue("performance",
        "numa_dedicated", &enable, CONFIG_TYPE_BOOL);
    if (ret == EID(SUCCESS))
    {
        numaDedicatedSchedulingPolicy = enable;
    }

    if (nullptr == qosManager)
    {
        qosManager = QosManagerSingleton::Instance();
    }

    for (unsigned int event = 0; (BackendEvent)event < BackendEvent_Count; event++)
    {
        eventQueue[event] = new SchedulerQueue {qosManager};
        queueOccupied[event] = false;
    }
    for (uint32_t numa = 0; numa < MAX_NUMA; numa++)
    {
        workerIDPerNumaVector[numa].clear();
    }
    totalWorkerIDVector.clear();
    cyclesElapsed = 0;
}

EventScheduler::~EventScheduler(void)
{
    exit = true;
    if (nullptr != schedulerThread)
    {
        schedulerThread->join();
    }
    for (unsigned int event = 0; (BackendEvent)event < BackendEvent_Count; event++)
    {
        delete eventQueue[event];
    }
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
EventScheduler::Initialize(uint32_t workerCountInput,
    cpu_set_t schedulerCPUInput, cpu_set_t eventCPUSetInput)
{
    policy = new MinimumJobPolicy(workerCountInput);
    workerCount = workerCountInput;
    workerArray.resize(workerCountInput);
    schedulerCPUSet = schedulerCPUInput;

    try
    {
        _BuildCpuSet(eventCPUSetInput);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return;
    }

    for (unsigned int workerID = 0; workerID < workerCount; workerID++)
    {
        workerArray[workerID] =
            new EventWorker(cpuSetVector.at(workerID), this, workerID);
    }
    schedulerThread = new std::thread(&EventScheduler::Run, this);
}

uint32_t
EventScheduler::GetWorkerIDMinimumJobs(uint32_t numa)
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
EventScheduler::EnqueueEvent(EventSmartPtr input)
{
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
            default:
                assert(0);
                break;
        }
    }
    _CheckAndSetQueueOccupancy(input->GetEventType());
}

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
                _CheckAndSetQueueOccupancy(BackendEvent_FrontendIO);
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
                _CheckAndSetQueueOccupancy((BackendEvent)eventType);
            }
        }
    }
    return eventList;
}

void
EventScheduler::Run(void)
{
    pthread_setname_np(pthread_self(), "EventScheduler");
    sched_setaffinity(0, sizeof(cpu_set_t), &schedulerCPUSet);

    // Scheduler Big Loop
    // Select thread according to SchedulerPolicy to execute event in Event Queue
    while (false == exit)
    {
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
            workerID = policy->GetProperWorkerID(event->GetNumaId());
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
    }
}

void
EventScheduler::_BuildCpuSet(cpu_set_t& cpuSet)
{
    uint32_t totalCore = affinityManager->GetTotalCore();

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
                    "EventScheduler receives wrong worker count and cpu_set_t");
                throw std::runtime_error("cpuIndex is bigger than totalCore");
            }
        }

        cpu_set_t tempCpuSet;
        CPU_ZERO(&tempCpuSet);
        CPU_SET(cpuIndex, &tempCpuSet);
        cpuSetVector.push_back(tempCpuSet);

        uint32_t numa = affinityManager->GetNumaIdFromCoreId(cpuIndex);
        workerIDPerNumaVector[numa].push_back(workerID);
        totalWorkerIDVector.push_back(workerID);

        cpuIndex = cpuIndex + 1;
    }
}
bool
EventScheduler::_CheckContention(BackendEvent eventId)
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
EventScheduler::_CheckAndSetQueueOccupancy(BackendEvent eventId)
{
    if (eventQueue[eventId]->GetQueueSize() == 0)
        queueOccupied[eventId] = false;
    else
        queueOccupied[eventId] = true;
}

bool
EventScheduler::_GetQueueOccupancy(BackendEvent eventId)
{
    return queueOccupied[eventId];
}

bool
EventScheduler::_NoContentionCycleDone(uint32_t cycles)
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
EventScheduler::_IncrementCycles(void)
{
    cyclesElapsed++;
    uint32_t cycles = qosManager->GetNoContentionCycles();
    if (cyclesElapsed / cycles >= 1)
    {
        cyclesElapsed = cycles;
    }
}

int32_t
EventScheduler::_GetEventWeight(BackendEvent eventId)
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

} // namespace pos

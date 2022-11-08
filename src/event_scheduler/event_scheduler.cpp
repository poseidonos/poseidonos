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

#include <stdexcept>

#include "src/cpu_affinity/affinity_manager.h"
#include "src/device/i_io_dispatcher.h"
#include "src/event_scheduler/backend_event_minimum_policy.h"
#include "src/event_scheduler/backend_event_ratio_policy.h"
#include "src/event_scheduler/event.h"
#include "src/event_scheduler/event_queue.h"
#include "src/event_scheduler/event_worker.h"
#include "src/event_scheduler/scheduler_queue.h"
#include "src/event_scheduler/spdk_event_scheduler.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"
#include "src/master_context/config_manager.h"
#include "src/qos/qos_manager.h"
#include "src/spdk_wrapper/accel_engine_api.h"

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

    for (uint32_t numa = 0; numa < MAX_NUMA; numa++)
    {
        workerIDPerNumaVector[numa].clear();
    }

    workerArray.clear();
    totalWorkerIDVector.clear();
    ioDispatcher = nullptr;
    terminateStarted = false;
    ioReactorCount = 0;
    for (uint32_t coreIndex = 0; coreIndex < MAX_CORE; coreIndex++)
    {
        ioReactorCore[coreIndex] = 0;
    }
    for (uint32_t coreIndex = 0; coreIndex < MAX_CORE; coreIndex++)
    {
        if (AffinityManagerSingleton::Instance()->IsIoReactor(coreIndex))
        {
            ioReactorCore[ioReactorCount] = coreIndex;
            ioReactorCount++;
        }
    }
}

EventScheduler::~EventScheduler(void)
{
    exit = true;
    if (nullptr != schedulerThread)
    {
        schedulerThread->join();
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

    policy = new BackendEventRatioPolicy(qosManager, &workerArray, workerCountInput, affinityManager->GetCoreCount(CoreType::UDD_IO_WORKER));
    policy->Init(workerIDPerNumaVector, totalWorkerIDVector, numaDedicatedSchedulingPolicy);

    for (unsigned int workerID = 0; workerID < workerCount; workerID++)
    {
        workerArray[workerID] =
            new EventWorker(cpuSetVector.at(workerID), this, workerID);
    }
    schedulerThread = new std::thread(&EventScheduler::Run, this);
}

void
EventScheduler::InjectIODispatcher(IIODispatcher* input)
{
    ioDispatcher = input;
}

void
EventScheduler::EjectIODispatcher(void)
{
    ioDispatcher = nullptr;
}

void
EventScheduler::EnqueueEvent(EventSmartPtr input)
{
    if (!affinityManager->UseEventReactor())
    {
        policy->EnqueueEvent(input);
    }
    else
    {
        static std::atomic<uint32_t> lastReactorIndex;
        uint32_t type = static_cast<uint32_t>(input->GetEventType());
        if (type == BackendEvent_UserdataRebuild || type == BackendEvent_MetadataRebuild ||
            type == BackendEvent_FlushMap || type == BackendEvent_GC || type == BackendEvent_Unknown)
        {
            type = ReactorType_SpecialEvent;
        }
        else
        {
            type = ReactorType_IOEvent;
        }

        uint32_t coreIndex = lastReactorIndex;
        uint32_t targetCore = ioReactorCore[coreIndex], coreCount = ioReactorCount;
        bool ret = false;
        if (type == ReactorType_IOEvent)
        {
            ret = SpdkEventScheduler::SendSpdkEvent(targetCore, input);
            lastReactorIndex = (lastReactorIndex + 1) % coreCount;
        }
        else
        {
            ret = SpdkEventScheduler::SendSpdkEvent(input);
        }
        assert(ret);
    }
}

std::queue<EventSmartPtr>
EventScheduler::DequeueEvents(void)
{
    return policy->DequeueEvents();
}
int32_t
EventScheduler::GetAllowedIoCount(BackendEvent eventType)
{
    return policy->GetAllowedIoCount(eventType);
}

void
EventScheduler::IoEnqueued(BackendEvent type, uint64_t size)
{
    policy->IoEnqueued(type, size);
}

void
EventScheduler::CheckAndSetQueueOccupancy(BackendEvent eventId)
{
    policy->CheckAndSetQueueOccupancy(eventId);
}

EventSmartPtr
EventScheduler::PickWorkerEvent(EventWorker* worker)
{
    return policy->PickWorkerEvent(worker);
}

void
EventScheduler::IoDequeued(BackendEvent type, uint64_t size)
{
    policy->IoDequeued(type, size);
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
        if (likely(terminateStarted == false && ioDispatcher != nullptr))
        {
            ioDispatcher->ProcessQueues();
        }
        if (QosReturnCode::FAILURE == policy->Run())
        {
            usleep(1);
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
                    EID(EVENTSCHEDULER_NOT_MATCH_WORKER_COUNT);
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
} // namespace pos

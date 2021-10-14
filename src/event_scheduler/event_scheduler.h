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

#include <rte_config.h>
#include <sched.h>

#include <atomic>
#include <cstdint>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "src/include/backend_event.h"
#include "src/include/smart_ptr_type.h"
#include "src/lib/singleton.h"

namespace pos
{
class AffinityManager;
class ConfigManager;
class ISchedulerPolicy;
class EventQueue;
class EventWorker;
class SchedulerQueue;
class QosManager;
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Schedule events to Event worker
 *           Single core dedicated
 */
/* --------------------------------------------------------------------------*/
class EventScheduler
{
public:
    EventScheduler(QosManager* qosManager = nullptr,
        ConfigManager* configManager = nullptr,
        AffinityManager* affinityManager = nullptr);
    virtual ~EventScheduler(void);
    void Initialize(uint32_t workerCountInput, cpu_set_t schedulerCPUInput,
        cpu_set_t eventCPUSetInput);
    virtual uint32_t GetWorkerIDMinimumJobs(uint32_t numa);
    virtual void EnqueueEvent(EventSmartPtr input);
    std::queue<EventSmartPtr> DequeueEvents(void);
    void Run(void);
    std::mutex queueLock[BackendEvent_Count];

private:
    void _BuildCpuSet(cpu_set_t& cpuSet);
    bool _CheckContention(void);
    void _CheckAndSetQueueOccupancy(BackendEvent eventId);
    bool _GetQueueOccupancy(BackendEvent eventId);
    bool _NoContentionCycleDone(uint32_t cycles);
    void _IncrementCycles(void);
    int32_t _GetEventWeight(BackendEvent eventId);
    ISchedulerPolicy* policy;
    std::atomic<bool> exit;
    uint32_t workerCount;
    std::vector<EventWorker*> workerArray;
    std::thread* schedulerThread;
    cpu_set_t schedulerCPUSet;
    std::vector<cpu_set_t> cpuSetVector;
    static const uint32_t MAX_NUMA = RTE_MAX_NUMA_NODES;
    std::vector<uint32_t> workerIDPerNumaVector[MAX_NUMA];
    std::vector<uint32_t> totalWorkerIDVector;

    SchedulerQueue* eventQueue[BackendEvent_Count];
    int32_t oldWeight[BackendEvent_Count] = {0};
    int32_t runningWeight[BackendEvent_Count] = {0};
    bool queueOccupied[BackendEvent_Count] = {false};
    bool numaDedicatedSchedulingPolicy;
    uint32_t cyclesElapsed = 0;
    QosManager* qosManager;
    ConfigManager* configManager;
    AffinityManager* affinityManager;
};

using EventSchedulerSingleton = Singleton<EventScheduler>;

} // namespace pos

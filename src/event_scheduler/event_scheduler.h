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
class BackendPolicy;
class EventQueue;
class EventWorker;
class SchedulerQueue;
class QosManager;
class IIODispatcher;

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Schedule events to Event worker
 *           Single core dedicated
 */
/* --------------------------------------------------------------------------*/
enum ReactorType
{
    ReactorType_IOEvent = 0,
    ReactorType_SpecialEvent = 1,
    ReactorType_Count
};

class EventScheduler
{
public:
    EventScheduler(QosManager* qosManager = nullptr,
        ConfigManager* configManager = nullptr,
        AffinityManager* affinityManager = nullptr);
    virtual ~EventScheduler(void);
    void Initialize(uint32_t workerCountInput, cpu_set_t schedulerCPUInput,
        cpu_set_t eventCPUSetInput);
    void InjectIODispatcher(IIODispatcher* ioDispatcher);
    void EjectIODispatcher(void);
    virtual void EnqueueEvent(EventSmartPtr input);
    std::queue<EventSmartPtr> DequeueEvents(void);
    virtual int32_t GetAllowedIoCount(BackendEvent type);
    virtual void IoEnqueued(BackendEvent type, uint64_t size);
    virtual void IoDequeued(BackendEvent type, uint64_t size);
    virtual EventSmartPtr PickWorkerEvent(EventWorker* worker);
    virtual void CheckAndSetQueueOccupancy(BackendEvent eventId);
    void Run(void);
    BackendPolicy* policy;
    void SetTerminate(bool value)
    {
        terminateStarted = value;
    }

private:
    void _BuildCpuSet(cpu_set_t& cpuSet);
    std::atomic<bool> exit;
    uint32_t workerCount;
    std::vector<EventWorker*> workerArray;
    std::thread* schedulerThread;
    cpu_set_t schedulerCPUSet;
    std::vector<cpu_set_t> cpuSetVector;
    static const uint32_t MAX_NUMA = RTE_MAX_NUMA_NODES;
    std::vector<uint32_t> workerIDPerNumaVector[MAX_NUMA];
    std::vector<uint32_t> totalWorkerIDVector;
    bool numaDedicatedSchedulingPolicy;
    QosManager* qosManager;
    ConfigManager* configManager;
    AffinityManager* affinityManager;
    IIODispatcher* ioDispatcher;
    std::atomic<bool> terminateStarted;
    static const uint32_t MAX_CORE = 128;
    uint32_t ioReactorCore[MAX_CORE];
    uint32_t ioReactorCount = 0;
};

using EventSchedulerSingleton = Singleton<EventScheduler>;

} // namespace pos

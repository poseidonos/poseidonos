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

#include <atomic>
#include <mutex>
#include <queue>
#include <vector>

#include "src/bio/ubio.h"
#include "src/include/backend_event.h"

namespace pos
{
class EventWorker;
class EventQueue;
class SchedulerQueue;
class QosManager;

const uint32_t FE_QUEUES = 47;

class BackendPolicy
{
public:
    BackendPolicy(QosManager* qosManagerArg,
        std::vector<EventWorker*>* workerArrayInput, uint32_t workerCount, uint32_t ioWorkerCount);

    void Init(std::vector<uint32_t> iworkerIDPerNumaVector[RTE_MAX_NUMA_NODES],
        std::vector<uint32_t> itotalWorkerIDVector, bool inumaDedicatedSchedulingPolicy);
    virtual ~BackendPolicy();
    virtual void EnqueueEvent(EventSmartPtr input) = 0;
    virtual std::queue<EventSmartPtr> DequeueEvents(void) = 0;
    virtual int Run() = 0;
    virtual EventSmartPtr PickWorkerEvent(EventWorker*) = 0;
    virtual void CheckAndSetQueueOccupancy(BackendEvent eventId) = 0;

    inline void IoEnqueued(BackendEvent type, uint64_t size)
    {
        ioControl.currentIOCount[type] += size / Ubio::BYTES_PER_UNIT;
    }

    inline void IoDequeued(BackendEvent type, uint64_t size)
    {
        ioControl.currentIOCount[type] -= size / Ubio::BYTES_PER_UNIT;
    }

    inline int32_t GetAllowedIoCount(BackendEvent type)
    {
        return ioControl.allowedIOCount[type] - ioControl.currentIOCount[type];
    }

protected:
    bool _GetQueueOccupancy(BackendEvent eventId);
    static const uint32_t MAX_NUMA = RTE_MAX_NUMA_NODES;
    QosManager* qosManager;
    std::vector<EventWorker*>* workerArray;
    uint32_t workerCount;
    std::vector<uint32_t> workerIDPerNumaVector[MAX_NUMA];
    std::vector<uint32_t> totalWorkerIDVector;
    bool numaDedicatedSchedulingPolicy;

    std::atomic<uint64_t> outstandingFrontendIO;
    std::atomic<bool> queueOccupied[BackendEvent_Count];
    SchedulerQueue* eventQueue[BackendEvent_Count];
    std::mutex queueLock[BackendEvent_Count];
    struct IoControl
    {
        std::atomic<int32_t> currentIOCount[BackendEvent_Count];
        std::atomic<int32_t> allowedIOCount[BackendEvent_Count];
    } ioControl;
};

} // namespace pos

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

#include "backend_policy.h"

#include <atomic>
#include <mutex>
#include <queue>
#include <vector>

#include "src/event_scheduler/scheduler_queue.h"
#include "src/qos/qos_manager.h"

namespace pos
{
BackendPolicy::BackendPolicy(QosManager* qosManagerArg, std::vector<EventWorker*>* workerArrayInput, uint32_t workerCount, uint32_t ioWorkerCount)
: qosManager(qosManagerArg),
  workerArray(workerArrayInput),
  workerCount(workerCount)
{
    numaDedicatedSchedulingPolicy = false;
    workerArray = {nullptr};
    for (int type = BackendEvent_Start; type < BackendEvent_Count; type++)
    {
        queueOccupied[type] = false;
        eventQueue[type] = nullptr;
        ioControl.allowedIOCount[type] = 0;
        ioControl.currentIOCount[type] = 0;
    }
}

BackendPolicy::~BackendPolicy()
{
    qosManager = nullptr;
    for (unsigned int event = 0; (BackendEvent)event < BackendEvent_Count; event++)
    {
        delete eventQueue[event];
    }
}

void
BackendPolicy::Init(std::vector<uint32_t> iworkerIDPerNumaVector[RTE_MAX_NUMA_NODES],
    std::vector<uint32_t> itotalWorkerIDVector, bool inumaDedicatedSchedulingPolicy)
{
    for (unsigned int event = 0; (BackendEvent)event < BackendEvent_Count; event++)
    {
        eventQueue[event] = new SchedulerQueue {qosManager};
    }
    for (unsigned int numa = 0; numa < RTE_MAX_NUMA_NODES; ++numa)
    {
        workerIDPerNumaVector[numa] = iworkerIDPerNumaVector[numa];
    }
    totalWorkerIDVector = itotalWorkerIDVector;
    numaDedicatedSchedulingPolicy = inumaDedicatedSchedulingPolicy;
}

bool
BackendPolicy::_GetQueueOccupancy(BackendEvent eventId)
{
    return queueOccupied[eventId];
}

} // namespace pos

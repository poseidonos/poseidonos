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
#include <cstdint>
#include <queue>
#include <vector>

#include "backend_policy.h"

namespace pos
{
class BackendEventRatioPolicy : public BackendPolicy
{
public:
    BackendEventRatioPolicy(QosManager* qosManager, std::vector<EventWorker*>* workerarray, uint32_t workerCount, uint32_t ioWorkerCount = 1);
    ~BackendEventRatioPolicy();
    virtual void EnqueueEvent(EventSmartPtr input);
    virtual std::queue<EventSmartPtr> DequeueEvents(void);
    virtual EventSmartPtr DequeueWorkerEvent(void);
    virtual uint32_t GetWorkerQueueSize();
    inline virtual int Run();
    inline virtual EventSmartPtr PickWorkerEvent(EventWorker*);
    void CheckAndSetQueueOccupancy(BackendEvent eventId);

private:
    int32_t _GetEventLimit(BackendEvent eventId);
    void _CheckAndSetAllowedLimit(void);
    void _InitLimitValues(void);

    int32_t totalIOCount;
    uint32_t totalEventCount;
    std::atomic<int32_t> allowedEventCount[BackendEvent_Count];
    std::atomic<int32_t> currentEventCount[BackendEvent_Count];
    std::atomic<uint32_t> cycleCount[BackendEvent_Count];
    std::atomic<int32_t> limit[BackendEvent_Count];
    SchedulerQueue* feEventQueue[FE_QUEUES];
    std::mutex frontendQueueLock[FE_QUEUES];
    std::queue<EventSmartPtr> workerCommonQueue;
    std::mutex workerQueueLock;
};

} // namespace pos

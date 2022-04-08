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

#include <atomic>
#include <cstdint>
#include <thread>
#include <unordered_set>
#include <vector>

#include "io_worker_device_operation.h"
#include "io_worker_device_operation_queue.h"
#include "src/bio/ubio.h"


namespace pos
{
class IOQueue;
class Ubio;
class UBlockDevice;
class DeviceDetachTrigger;
class QosManager;
class EventScheduler;

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Handle bio, interact with devices
 *           Affinited to single core
 */
/* --------------------------------------------------------------------------*/
class IOWorker
{
public:
    IOWorker(cpu_set_t cpuSetInput, uint32_t id,
        DeviceDetachTrigger* detachTrigger = nullptr,
        QosManager* qosManager = nullptr, EventScheduler* eventScheduler = nullptr);
    virtual ~IOWorker(void);
    uint32_t GetWorkerId(void);
    virtual void DecreaseCurrentOutstandingIoCount(int count);

    virtual void EnqueueUbio(UbioSmartPtr ubio);
    int GetQueueSize(void);
    uint32_t AddDevice(UblockSharedPtr device);
    uint32_t AddDevices(std::vector<UblockSharedPtr>* inputList);
    virtual uint32_t RemoveDevice(UblockSharedPtr device);

    void Run(void);

private:
    void _SubmitAsyncIO(UbioSmartPtr ubio);
    void _SubmitPendingIO(void);
    void _CompleteCommand(void);
    void _DoPeriodicJob(void);
    void _HandleDeviceOperation(void);

    using DeviceSet = std::unordered_set<UblockSharedPtr>;
    using DeviceSetIter = DeviceSet::iterator;

    IoWorkerDeviceOperationQueue operationQueue;
    cpu_set_t cpuSet;
    IOQueue* ioQueue;
    std::thread* thread;
    uint32_t currentOutstandingIOCount;

    DeviceSet deviceList;
    std::atomic<bool> exit;
    uint32_t id;

    DeviceDetachTrigger* detachTrigger;
    bool productDetachTrigger {false};
    QosManager* qosManager;
    EventScheduler *eventScheduler;
};
} // namespace pos

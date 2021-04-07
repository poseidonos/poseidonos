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

#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_set>
#include <vector>

#include "src/io/general_io/ubio.h"

struct spdk_thread;

namespace ibofos
{
class EventScheduler;
class IOQueue;
class Ubio;
class UBlockDevice;

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Handle bio, interact with devices
 *           Affinited to single core
 */
/* --------------------------------------------------------------------------*/
class IOWorker
{
public:
    IOWorker(cpu_set_t cpuSetInput, uint32_t id);
    virtual ~IOWorker(void);

    void EnqueueUbio(UbioSmartPtr ubio);
    uint32_t AddDevice(UBlockDevice* device);
    uint32_t AddDevices(std::vector<UBlockDevice*>* inputList);
    uint32_t RemoveDevice(UBlockDevice* device);
    bool HasDevice(UBlockDevice* device);
    void StartThreadRegistration(void);
    void WaitRegistration(void);

    void Run(void);

#if defined QOS_ENABLED_BE
    void DecreaseCurrentOutstandingIoCount(int count);
    uint32_t GetWorkerId(void);
#endif

private:
    void _SubmitAsyncIO(UbioSmartPtr ubio);
    void _CompleteCommand(void);
    void _DoPeriodicJob(void);
    void _HandleDeviceOperation(void);

    enum DeviceOperationType
    {
        INSERT,
        REMOVE
    };

    class DeviceOperation
    {
    public:
        DeviceOperation(DeviceOperationType type, UBlockDevice* device);
        void WaitDone(void);
        void SetDone(void);
        UBlockDevice* GetDevice(void);
        DeviceOperationType GetCommand(void);

    private:
        DeviceOperationType command;
        UBlockDevice* device;
        std::atomic<bool> done;
    };

    class DeviceOperationQueue
    {
    public:
        void SubmitAndWait(DeviceOperationType type, UBlockDevice* device);
        DeviceOperation* Pop(void);

    private:
        std::queue<DeviceOperation*> queue;
        std::mutex queueLock;
    };

    using DeviceSet = std::unordered_set<UBlockDevice*>;
    using DeviceSetIter = DeviceSet::iterator;

    DeviceOperationQueue operationQueue;
    cpu_set_t cpuSet;
    IOQueue* ioQueue;
    std::thread* thread;
    pthread_rwlock_t deviceListLock;
    uint32_t currentOutstandingIOCount;
    spdk_thread* spdkThread;

    DeviceSet deviceList;
    std::atomic<bool> exit;
    uint32_t id;
};
} // namespace ibofos

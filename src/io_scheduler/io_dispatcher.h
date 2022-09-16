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

#include <memory>
#include <unordered_map>
#include <mutex>
#include <queue>
#include <vector>
#include <utility>

#include "src/device/i_io_dispatcher.h"
#include "src/include/backend_event.h"
#include "src/lib/singleton.h"

namespace pos
{

class IOWorker;
class EventFactory;
class EventFrameworkApi;
class EventScheduler;
class DispatcherPolicyI;

class IODispatcher : public IIODispatcher
{
public:
    explicit IODispatcher(EventFrameworkApi* eventFrameworkApi = nullptr,
        EventScheduler* eventScheduler = nullptr);
    ~IODispatcher(void);

    void AddIOWorker(cpu_set_t cpuSet) override;
    void RemoveIOWorker(cpu_set_t cpuSet) override;
    std::size_t SizeIOWorker(void);

    // These functions shall be guarrenteed to run itself at once by caller.
    void AddDeviceForReactor(UblockSharedPtr dev) override;
    void RemoveDeviceForReactor(UblockSharedPtr dev) override;

    void AddDeviceForIOWorker(UblockSharedPtr dev, cpu_set_t cpuSet) override;
    void RemoveDeviceForIOWorker(UblockSharedPtr dev) override;

    static void CompleteForThreadLocalDeviceList(void);
    int Submit(UbioSmartPtr ubio, bool sync = false, bool ioRecoveryNeeded = true) override;
    void ProcessQueues(void) override;

    static void RegisterRecoveryEventFactory(EventFactory* recoveryEventFactory);
    static void SetFrontendDone(bool value);

    std::queue<std::pair<IOWorker*, UbioSmartPtr>> ioQueue[BackendEvent_Count];
    std::mutex ioQueueLock[BackendEvent_Count];

private:
    uint32_t _GetLogicalCore(cpu_set_t cpuSet, uint32_t index);
    void _CallForFrontend(UblockSharedPtr device);
    void _SubmitRecovery(UbioSmartPtr ubio);
    static void _ProcessFrontend(void* ublockDevice);
    static void _AddDeviceToThreadLocalList(UblockSharedPtr device);
    static void _RemoveDeviceFromThreadLocalList(UblockSharedPtr device);
    static void _SubmitIOInReactor(void* ptr1, void* ptr2);

    using IOWorkerMap = std::unordered_map<uint32_t, IOWorker*>;
    using IOWorkerMapIter = IOWorkerMap::iterator;
    using IOWorkerPair = std::pair<uint32_t, IOWorker*>;

    uint32_t ioWorkerCount;
    pthread_rwlock_t ioWorkerMapLock;
    IOWorkerMap ioWorkerMap;
    uint32_t deviceAllocationTurn;
    EventScheduler* eventScheduler;
    std::mutex deviceLock;
    DispatcherPolicyI* dispPolicy;

    enum class DispatcherAction
    {
        OPEN,
        CLOSE,
    };
    static DispatcherAction frontendOperation;
    static EventFactory* recoveryEventFactory;
    static bool frontendDone;

    static EventFrameworkApi* eventFrameworkApi;
    static thread_local std::vector<UblockSharedPtr> threadLocalDeviceList;

    static const int DEVICE_FAILED = -1;
    static const int PARAM_FAILED = -2;
};

using IODispatcherSingleton = Singleton<IODispatcher>;

} // namespace pos

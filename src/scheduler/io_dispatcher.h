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

#include <memory>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ibofos
{
class IOWorker;
class UBlockDevice;

class Ubio;
using UbioSmartPtr = std::shared_ptr<Ubio>;
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Decide IOWorker for handling bio
 */
/* --------------------------------------------------------------------------*/
class IODispatcher
{
public:
    IODispatcher(void);
    ~IODispatcher(void);

    void AddIOWorker(cpu_set_t cpuSet);
    void RemoveIOWorker(cpu_set_t cpuSet);

    void StartIOWorkers(void);

    void CallForFrontend(UBlockDevice* device);

    // These functions shall be guarrenteed to run itself at once by caller.
    void AddDeviceForReactor(UBlockDevice* dev);
    void RemoveDeviceForReactor(UBlockDevice* dev);

    void AddDeviceForIOWorker(UBlockDevice* dev, cpu_set_t cpuSet);
    void RemoveDeviceForIOWorker(UBlockDevice* dev);

    static void ProcessFrontend(void* ublockDevice, void* arg2);

    int Submit(UbioSmartPtr ubio, bool sync = false);
    void CompleteForThreadLocalDeviceList(void);

private:
    using IOWorkerMap = std::unordered_map<uint32_t, IOWorker*>;
    using IOWorkerMapIter = IOWorkerMap::iterator;
    using IOWorkerPair = std::pair<uint32_t, IOWorker*>;

    IOWorkerMap ioWorkerMap;

    pthread_rwlock_t ioWorkerMapLock;

    uint32_t _GetLogicalCore(cpu_set_t cpuSet, uint32_t index);

    static bool frontendDone;
    uint32_t deviceAllocationTurn;
    enum class DispatcherAction
    {
        OPEN,
        CLOSE,
    };
    static DispatcherAction frontendOperation;
    std::mutex deviceLock;
    uint32_t ioWorkerCount;

    static void _AddDeviceToThreadLocalList(UBlockDevice* device);
    static void _RemoveDeviceFromThreadLocalList(UBlockDevice* device);

    static thread_local std::vector<UBlockDevice*> threadLocalDeviceList;
    static const int UBIO_CHECK_VALID_FAILED = -1;
    static const int PARAM_FAILED = -2;
};

} // namespace ibofos

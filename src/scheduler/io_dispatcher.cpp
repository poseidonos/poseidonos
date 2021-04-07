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

#include "io_dispatcher.h"

#include <assert.h>
#include <unistd.h>

#include <thread>

#include "event_framework_api.h"
#include "io_worker.h"
#include "src/include/branch_prediction.h"
#include "src/include/ibof_event_id.hpp"
#include "src/io/general_io/affinity_manager.h"
#include "src/logger/logger.h"
#include "ublock_device.h"

namespace ibofos
{
bool IODispatcher::frontendDone;
IODispatcher::DispatcherAction IODispatcher::frontendOperation;
thread_local std::vector<UBlockDevice*> IODispatcher::threadLocalDeviceList;

IODispatcher::IODispatcher(void)
: ioWorkerCount(0)
{
    pthread_rwlock_init(&ioWorkerMapLock, nullptr);
}

IODispatcher::~IODispatcher(void)
{
    IOWorkerMapIter it;
    for (it = ioWorkerMap.begin(); it != ioWorkerMap.end(); it++)
    {
        delete it->second;
    }
}

uint32_t
IODispatcher::_GetLogicalCore(cpu_set_t cpuSet, uint32_t index)
{
    uint32_t logicalCore = 0;
    uint32_t currentIndex = 0;
    uint32_t cpuCount = CPU_COUNT(&cpuSet);

    if (cpuCount <= index)
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::IODISPATCHER_INVALID_CPU_INDEX;
        IBOF_TRACE_ERROR(eventId, IbofEventId::GetString(eventId), index);
        return UINT32_MAX;
    }

    for (;; logicalCore++)
    {
        if (CPU_ISSET(logicalCore, &cpuSet))
        {
            if (currentIndex == index)
            {
                break;
            }

            currentIndex++;
        }
    }

    return logicalCore;
}

void
IODispatcher::AddIOWorker(cpu_set_t cpuSet)
{
    uint32_t cpuCount = CPU_COUNT(&cpuSet);

    pthread_rwlock_wrlock(&ioWorkerMapLock);
    for (uint32_t index = 0; index < cpuCount; index++)
    {
        uint32_t logicalCore = _GetLogicalCore(cpuSet, index);
        if (logicalCore == UINT32_MAX)
        {
            IBOF_EVENT_ID eventId =
                IBOF_EVENT_ID::IODISPATCHER_INVALID_CPU_INDEX;
            IBOF_TRACE_ERROR(eventId, IbofEventId::GetString(eventId), index);
            break;
        }

        IOWorkerMapIter it = ioWorkerMap.find(logicalCore);
        if (it == ioWorkerMap.end())
        {
            uint32_t ioWorkerIndex = ioWorkerCount;
            cpu_set_t targetCpuSet;
            CPU_ZERO(&targetCpuSet);
            CPU_SET(logicalCore, &targetCpuSet);
            IOWorker* newIOWorker = new IOWorker(targetCpuSet, ioWorkerIndex);
            ioWorkerCount++;
            ioWorkerMap.insert(IOWorkerPair(logicalCore, newIOWorker));
        }
    }
    pthread_rwlock_unlock(&ioWorkerMapLock);
}

void
IODispatcher::RemoveIOWorker(cpu_set_t cpuSet)
{
    uint32_t cpuCount = CPU_COUNT(&cpuSet);

    pthread_rwlock_wrlock(&ioWorkerMapLock);
    for (uint32_t index = 0; index < cpuCount; index++)
    {
        uint32_t logicalCore = _GetLogicalCore(cpuSet, index);

        IOWorkerMapIter it = ioWorkerMap.find(logicalCore);
        if (it != ioWorkerMap.end())
        {
            delete it->second;
            ioWorkerMap.erase(it);
        }
    }
    pthread_rwlock_unlock(&ioWorkerMapLock);
}

void
IODispatcher::CallForFrontend(UBlockDevice* dev)
{
    frontendDone = false;

    uint32_t firstReactorCore = EventFrameworkApi::GetFirstReactor();
    bool succeeded = true;

    if (firstReactorCore == EventFrameworkApi::GetCurrentReactor())
    {
        ProcessFrontend(dev, nullptr);
    }
    else
    {
        succeeded = EventFrameworkApi::SendSpdkEvent(firstReactorCore,
            ProcessFrontend, dev, nullptr);
    }
    if (unlikely(false == succeeded))
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::DEVICE_COMPLETION_FAILED;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId));
    }
    else
    {
        while (false == frontendDone)
        {
            usleep(1);
        }
    }
}

void
IODispatcher::AddDeviceForReactor(UBlockDevice* dev)
{
    std::lock_guard<std::mutex> lock(deviceLock);
    frontendOperation = DispatcherAction::OPEN;
    CallForFrontend(dev);
}

void
IODispatcher::RemoveDeviceForReactor(UBlockDevice* dev)
{
    std::lock_guard<std::mutex> lock(deviceLock);
    frontendOperation = DispatcherAction::CLOSE;
    CallForFrontend(dev);
}

// This function will be executed in thread level.
void
IODispatcher::ProcessFrontend(void* ublockDevice, void* arg2)
{
    UBlockDevice* dev = static_cast<UBlockDevice*>(ublockDevice);
    if (frontendOperation == DispatcherAction::OPEN)
    {
        _AddDeviceToThreadLocalList(dev);
        dev->Open();
    }
    else
    {
        dev->Close();
        _RemoveDeviceFromThreadLocalList(dev);
    }

    if (EventFrameworkApi::IsLastReactorNow())
    {
        frontendDone = true;
    }
    else
    {
        uint32_t nextCore = EventFrameworkApi::GetNextReactor();
        bool success = EventFrameworkApi::SendSpdkEvent(nextCore,
            ProcessFrontend, dev, NULL);
        if (unlikely(false == success))
        {
            IBOF_EVENT_ID eventId =
                IBOF_EVENT_ID::DEVICE_COMPLETION_FAILED;
            IBOF_TRACE_ERROR(static_cast<int>(eventId),
                IbofEventId::GetString(eventId), dev->GetName());

            frontendDone = true;
        }
    }
}

void
IODispatcher::AddDeviceForIOWorker(UBlockDevice* dev, cpu_set_t cpuSet)
{
    std::lock_guard<std::mutex> lock(deviceLock);
    uint32_t cpuCount = CPU_COUNT(&cpuSet);

    pthread_rwlock_rdlock(&ioWorkerMapLock);
    uint32_t index = deviceAllocationTurn % cpuCount;
    deviceAllocationTurn++;
    // ToDo :: This needs to be changed dev to IoworkerMap
    uint32_t logicalCore = _GetLogicalCore(cpuSet, index);

    IOWorkerMapIter it = ioWorkerMap.find(logicalCore);
    if (likely(it != ioWorkerMap.end()))
    {
        IOWorker* ioWorker = it->second;
        dev->SetDedicatedIOWorker(ioWorker);
        ioWorker->AddDevice(dev);
    }

    pthread_rwlock_unlock(&ioWorkerMapLock);
}

void
IODispatcher::RemoveDeviceForIOWorker(UBlockDevice* dev)
{
    std::lock_guard<std::mutex> lock(deviceLock);
    IOWorker* ioWorker = dev->GetDedicatedIOWorker();

    if (ioWorker != nullptr)
    {
        ioWorker->RemoveDevice(dev);
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Return IOWorker according to device
 *
 * @Param    device
 *
 * @Returns  The pointer of proper IOWorker
 */
/* --------------------------------------------------------------------------*/

// These Thread Local list can be used only by "FrontEnd"
// This list is similar as "IOWorker's" one,
// But, IO worker's API (example, SubmitAndWait or HasDevice) can be called by other thread.
// This ThreadLocalList API can be used only by its own thread.

void
IODispatcher::_AddDeviceToThreadLocalList(UBlockDevice* device)
{
    threadLocalDeviceList.push_back(device);
}

void
IODispatcher::_RemoveDeviceFromThreadLocalList(UBlockDevice* device)
{
    for (std::vector<UBlockDevice*>::iterator iter = threadLocalDeviceList.begin();
         iter != threadLocalDeviceList.end(); iter = iter + 1)
    {
        if (*iter == device)
        {
            threadLocalDeviceList.erase(iter);
            break;
        }
    }
}

int
IODispatcher::Submit(UbioSmartPtr ubio, bool sync)
{
    bool isReactor = EventFrameworkApi::IsReactorNow();

    if (unlikely(isReactor && sync))
    {
        IBOF_EVENT_ID eventId =
            IBOF_EVENT_ID::IODISPATCHER_INVALID_PARM;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId));
        ubio->CompleteWithoutRecovery(CallbackError::GENERIC_ERROR);
        return PARAM_FAILED;
    }

    if (unlikely(sync))
    {
        ubio->SetSyncMode();
    }

    if (ubio->CheckValid())
    {
        UBlockDevice* ublock = ubio->GetUBlock();
        if (isReactor)
        {
            return ublock->SubmitAsyncIO(ubio);
        }
        else
        {
            IOWorker* ioWorker = ublock->GetDedicatedIOWorker();
            if (likely(ioWorker != nullptr))
            {
                ioWorker->EnqueueUbio(ubio);
            }
        }
        if (unlikely(sync))
        {
            ubio->WaitDone();
        }
    }
    else
    {
        return UBIO_CHECK_VALID_FAILED;
    }
    return 0;
}

void
IODispatcher::CompleteForThreadLocalDeviceList(void)
{
    for (auto& iter : threadLocalDeviceList)
    {
        iter->CompleteIOs();
    }
}

} // namespace ibofos

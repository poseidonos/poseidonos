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

#include "src/spdk_wrapper/event_framework_api.h"
#include "src/io_scheduler/io_worker.h"
#include "src/event_scheduler/io_completer.h"
#include "src/include/i_array_device.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"
#include "src/device/base/ublock_device.h"
#include "src/event_scheduler/spdk_event_scheduler.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/event_scheduler/event_factory.h"

namespace pos
{
IODispatcher::DispatcherAction IODispatcher::frontendOperation;
bool IODispatcher::frontendDone;
thread_local std::vector<UblockSharedPtr> IODispatcher::threadLocalDeviceList;
EventFactory* IODispatcher::recoveryEventFactory = nullptr;

IODispatcher::IODispatcher(EventFrameworkApi* eventFrameworkApi_)
: ioWorkerCount(0),
  deviceAllocationTurn(0),
  eventFrameworkApi(eventFrameworkApi_)
{
    pthread_rwlock_init(&ioWorkerMapLock, nullptr);
    if (nullptr == eventFrameworkApi)
    {
        eventFrameworkApi = EventFrameworkApiSingleton::Instance();
    }
}

IODispatcher::~IODispatcher(void)
{
    IOWorkerMapIter it;
    for (it = ioWorkerMap.begin(); it != ioWorkerMap.end(); it++)
    {
        delete it->second;
    }
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
            POS_EVENT_ID eventId =
                POS_EVENT_ID::IODISPATCHER_INVALID_CPU_INDEX;
            POS_TRACE_ERROR(eventId, PosEventId::GetString(eventId), index);
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

std::size_t
IODispatcher::SizeIOWorker(void)
{
    std::size_t size = 0;
    pthread_rwlock_wrlock(&ioWorkerMapLock);
    size = ioWorkerMap.size();
    pthread_rwlock_unlock(&ioWorkerMapLock);
    return size;
}

void
IODispatcher::AddDeviceForReactor(UblockSharedPtr dev)
{
    std::lock_guard<std::mutex> lock(deviceLock);
    frontendOperation = DispatcherAction::OPEN;
    _CallForFrontend(dev);
}

void
IODispatcher::RemoveDeviceForReactor(UblockSharedPtr dev)
{
    std::lock_guard<std::mutex> lock(deviceLock);
    frontendOperation = DispatcherAction::CLOSE;
    _CallForFrontend(dev);
}

void
IODispatcher::AddDeviceForIOWorker(UblockSharedPtr dev, cpu_set_t cpuSet)
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
IODispatcher::RemoveDeviceForIOWorker(UblockSharedPtr dev)
{
    std::lock_guard<std::mutex> lock(deviceLock);
    IOWorker* ioWorker = dev->GetDedicatedIOWorker();

    if (ioWorker != nullptr)
    {
        ioWorker->RemoveDevice(dev);
    }
}

void
IODispatcher::CompleteForThreadLocalDeviceList(void)
{
    for (auto& iter : threadLocalDeviceList)
    {
        iter->CompleteIOs();
    }
}

void
IODispatcher::_SubmitRecovery(UbioSmartPtr ubio)
{
    ubio->SetError(IOErrorType::DEVICE_ERROR);
    EventSmartPtr failure = recoveryEventFactory->Create(ubio);
    EventSchedulerSingleton::Instance()->EnqueueEvent(failure);
}

int
IODispatcher::Submit(UbioSmartPtr ubio, bool sync, bool ioRecoveryNeeded)
{
    bool isReactor = eventFrameworkApi->IsReactorNow();

    // sync io for reactor is not allowed. (reactor should not be stuck in any point.)
    if (unlikely(isReactor && sync))
    {
        POS_EVENT_ID eventId =
            POS_EVENT_ID::IODISPATCHER_INVALID_PARM;
        POS_TRACE_ERROR(static_cast<int>(eventId),
            PosEventId::GetString(eventId));
        IoCompleter ioCompleter(ubio);
        ioCompleter.CompleteUbioWithoutRecovery(IOErrorType::GENERIC_ERROR, true);
        return PARAM_FAILED;
    }

    // sync operation does not support RAID5 recovery.
    // If caller assure that device given will not be failed in submit context,
    // It needs to skip device recovery (ioRecoveryNeeded)
    if (unlikely(sync))
    {
        ubio->SetSyncMode();
    }
    else if (ioRecoveryNeeded && ubio->NeedRecovery())
    {
        _SubmitRecovery(ubio);
        return DEVICE_FAILED;
    }

    UBlockDevice* ublock = nullptr;

    // ublock pointer can be obtained from either weak pointer or shared pointer.
    // It depends on given flag.
    if (!ioRecoveryNeeded)
    {
        ublock = ubio->GetArrayDev()->GetUblockPtr();
    }
    else
    {
        ublock = ubio->GetUBlock();
    }

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

    return 0;
}

uint32_t
IODispatcher::_GetLogicalCore(cpu_set_t cpuSet, uint32_t index)
{
    uint32_t logicalCore = 0;
    uint32_t currentIndex = 0;
    uint32_t cpuCount = CPU_COUNT(&cpuSet);

    if (cpuCount <= index)
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::IODISPATCHER_INVALID_CPU_INDEX;
        POS_TRACE_ERROR(eventId, PosEventId::GetString(eventId), index);
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
IODispatcher::_CallForFrontend(UblockSharedPtr dev)
{
    UblockSharedPtr* devArg = new UblockSharedPtr(dev);
    frontendDone = false;

    uint32_t firstReactorCore = eventFrameworkApi->GetFirstReactor();
    bool succeeded = true;

    if (firstReactorCore == eventFrameworkApi->GetCurrentReactor())
    {
        _ProcessFrontend(devArg);
    }
    else
    {
        succeeded = eventFrameworkApi->SendSpdkEvent(firstReactorCore,
            _ProcessFrontend, devArg);
    }
    if (unlikely(false == succeeded))
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::DEVICE_COMPLETION_FAILED;
        POS_TRACE_ERROR(static_cast<int>(eventId),
            PosEventId::GetString(eventId));
    }
    else
    {
        while (false == frontendDone)
        {
            usleep(1);
        }
    }
}

// This function will be executed in thread level.
void
IODispatcher::_ProcessFrontend(void* ublockDevice)
{
    UblockSharedPtr dev = *static_cast<UblockSharedPtr*>(ublockDevice);
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

    if (EventFrameworkApiSingleton::Instance()->IsLastReactorNow())
    {
        frontendDone = true;
    }
    else
    {
        uint32_t nextCore = EventFrameworkApiSingleton::Instance()->GetNextReactor();
        UblockSharedPtr* devArg = new UblockSharedPtr(dev);
        bool success = EventFrameworkApiSingleton::Instance()->SendSpdkEvent(nextCore,
            _ProcessFrontend, devArg);
        if (unlikely(false == success))
        {
            POS_EVENT_ID eventId =
                POS_EVENT_ID::DEVICE_COMPLETION_FAILED;
            POS_TRACE_ERROR(static_cast<int>(eventId),
                PosEventId::GetString(eventId), dev->GetName());

            frontendDone = true;
        }
    }
    delete static_cast<UblockSharedPtr*>(ublockDevice);
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
IODispatcher::_AddDeviceToThreadLocalList(UblockSharedPtr device)
{
    threadLocalDeviceList.push_back(device);
}

void
IODispatcher::_RemoveDeviceFromThreadLocalList(UblockSharedPtr device)
{
    for (std::vector<UblockSharedPtr>::iterator iter = threadLocalDeviceList.begin();
         iter != threadLocalDeviceList.end(); iter = iter + 1)
    {
        if (*iter == device)
        {
            threadLocalDeviceList.erase(iter);
            break;
        }
    }
}

void
IODispatcher::RegisterRecoveryEventFactory(EventFactory* recoveryEventFactory)
{
    IODispatcher::recoveryEventFactory = recoveryEventFactory;
}

} // namespace pos

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

#include "io_dispatcher.h"

#include <unistd.h>

#include "src/device/base/ublock_device.h"
#include "src/device/device_detach_trigger.h"
#include "src/event_scheduler/event_factory.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/event_scheduler/io_completer.h"
#include "src/event_scheduler/spdk_event_scheduler.h"
#include "src/include/branch_prediction.h"
#include "src/include/i_array_device.h"
#include "src/include/pos_event_id.hpp"
#include "src/io_scheduler/dispatcher_policy.h"
#include "src/io_scheduler/io_dispatcher_submission.h"
#include "src/io_scheduler/io_worker.h"
#include "src/logger/logger.h"
#include "src/spdk_wrapper/accel_engine_api.h"
#include "src/spdk_wrapper/event_framework_api.h"
namespace pos
{
IODispatcher::DispatcherAction IODispatcher::frontendOperation;
bool IODispatcher::frontendDone;
thread_local std::vector<UblockSharedPtr> IODispatcher::threadLocalDeviceList;
EventFactory* IODispatcher::recoveryEventFactory = nullptr;
EventFrameworkApi* IODispatcher::eventFrameworkApi = nullptr;

IODispatcher::IODispatcher(EventFrameworkApi* eventFrameworkApiArg,
    EventScheduler* eventSchedulerArg)
: ioWorkerCount(0),
  deviceAllocationTurn(0),
  eventScheduler(eventSchedulerArg),
  dispPolicy(nullptr)
{
    pthread_rwlock_init(&ioWorkerMapLock, nullptr);
    eventFrameworkApi = eventFrameworkApiArg;
    if (nullptr == eventFrameworkApi)
    {
        eventFrameworkApi = EventFrameworkApiSingleton::Instance();
    }
    if (nullptr == eventScheduler)
    {
        eventScheduler = EventSchedulerSingleton::Instance();
    }
    eventScheduler->InjectIODispatcher(this);
    dispPolicy = new DispatcherPolicyQos(this, eventScheduler);
}

IODispatcher::~IODispatcher(void)
{
    IOWorkerMapIter it;
    for (it = ioWorkerMap.begin(); it != ioWorkerMap.end(); it++)
    {
        delete it->second;
    }
    delete dispPolicy;
    eventScheduler->EjectIODispatcher();
}

void
IODispatcher::AddIOWorker(cpu_set_t cpuSet)
{
    uint32_t cpuCount = CPU_COUNT(&cpuSet);
    if (0 == cpuCount)
    {
        return;
    }

    pthread_rwlock_wrlock(&ioWorkerMapLock);
    for (uint32_t index = 0; index < cpuCount; index++)
    {
        uint32_t logicalCore = _GetLogicalCore(cpuSet, index);
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
    if (0 == cpuCount)
    {
        return;
    }

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
    uint32_t cpuCount = CPU_COUNT(&cpuSet);
    if (0 == cpuCount)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(deviceLock);
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

    if (nullptr != ioWorker)
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

int
IODispatcher::Submit(UbioSmartPtr ubio, bool sync, bool ioRecoveryNeeded)
{
    bool isReactor = eventFrameworkApi->IsReactorNow();
    uint32_t currentCore = EventFrameworkApiSingleton::Instance()->GetCurrentReactor();
    bool isEventReactor = false;
    if (isReactor)
    {
        isEventReactor = AffinityManagerSingleton::Instance()->IsEventReactor(currentCore);
    }
    // sync io for reactor is not allowed. (reactor should not be stuck in any point.)
    int ret = 0;
    if (unlikely(isReactor && (!isEventReactor) && sync))
    {
        POS_EVENT_ID eventId =
            EID(IODISPATCHER_INVALID_PARM);
        POS_TRACE_ERROR(static_cast<int>(eventId),
            "Invalid Param in submit IO");
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
    else if (ioRecoveryNeeded)
    {
        bool needRecovery = ubio->NeedRecovery();
        if (needRecovery)
        {
            _SubmitRecovery(ubio);
            return DEVICE_FAILED;
        }
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

    ret = IODispatcherSubmissionSingleton::Instance()->SubmitIO(ublock, ubio, dispPolicy);

    if (unlikely(sync))
    {
        ubio->WaitDone();
    }

    return ret;
}

void
IODispatcher::ProcessQueues(void)
{
    if (likely(dispPolicy != nullptr))
    {
        dispPolicy->Process();
    }
}

void
IODispatcher::RegisterRecoveryEventFactory(EventFactory* recoveryEventFactory)
{
    IODispatcher::recoveryEventFactory = recoveryEventFactory;
}

void
IODispatcher::SetFrontendDone(bool value)
{
    IODispatcher::frontendDone = value;
}

uint32_t
IODispatcher::_GetLogicalCore(cpu_set_t cpuSet, uint32_t index)
{
    uint32_t currentIndex = 0;
    long nproc = sysconf(_SC_NPROCESSORS_ONLN);
    long logicalCore;

    for (logicalCore = 0; logicalCore < nproc; logicalCore++)
    {
        int cpu_set = CPU_ISSET(logicalCore, &cpuSet);
        if (0 != cpu_set)
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
        POS_EVENT_ID eventId = EID(DEVICE_COMPLETION_FAILED);
        POS_TRACE_ERROR(static_cast<int>(eventId),
            "Error: occurred while getting IO completion events from device: {}",
            dev->GetName());
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
IODispatcher::_SubmitRecovery(UbioSmartPtr ubio)
{
    ubio->SetError(IOErrorType::DEVICE_ERROR);
    EventSmartPtr failure = recoveryEventFactory->Create(ubio);
    eventScheduler->EnqueueEvent(failure);
}

// This function will be executed in thread level.
void
IODispatcher::_ProcessFrontend(void* ublockDevice)
{
    UblockSharedPtr dev = *static_cast<UblockSharedPtr*>(ublockDevice);
    if (DispatcherAction::OPEN == frontendOperation)
    {
        bool devOpen = dev->Open();
        if (devOpen)
        {
            _AddDeviceToThreadLocalList(dev);
        }
        else
        {
            DeviceType deviceType = dev->GetType();
            if (DeviceType::SSD == deviceType)
            {
                DeviceDetachTrigger detachTrigger;
                detachTrigger.Run(dev);
            }
        }
    }
    else
    {
        dev->Close();
        _RemoveDeviceFromThreadLocalList(dev);
    }

    bool isLastReactorNow = eventFrameworkApi->IsLastReactorNow();
    if (isLastReactorNow)
    {
        frontendDone = true;
    }
    else
    {
        uint32_t nextCore = eventFrameworkApi->GetNextReactor();
        UblockSharedPtr* devArg = new UblockSharedPtr(dev);
        bool success = eventFrameworkApi->SendSpdkEvent(nextCore,
            _ProcessFrontend, devArg);
        if (unlikely(false == success))
        {
            POS_EVENT_ID eventId =
                EID(DEVICE_COMPLETION_FAILED);
            POS_TRACE_ERROR(static_cast<int>(eventId),
                "Error: occurred while getting IO completion events from device: {}",
                dev->GetName());

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

} // namespace pos

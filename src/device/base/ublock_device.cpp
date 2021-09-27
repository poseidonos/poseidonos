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

#include "src/device/base/ublock_device.h"

#include <sched.h>

#include "Air.h"
#include "src/cpu_affinity/affinity_manager.h"
#include "src/device/base/device_context.h"
#include "src/device/base/device_driver.h"
#include "src/event_scheduler/io_completer.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/io_scheduler/io_dispatcher.h"
#include "src/logger/logger.h"
#include "src/spdk_wrapper/event_framework_api.h"

namespace pos
{
thread_local uint32_t UBlockDevice::currentThreadVirtualId;
std::atomic<uint32_t> UBlockDevice::lastVirtualId;

UBlockDevice::UBlockDevice(std::string name, uint64_t size,
    DeviceDriver* driverToUse)
: driver(driverToUse),
  pendingErrorCount(0),
  completeErrorAsSuccess(false),
  deviceContextCount(0),
  detachedFunctionProcessing(false),
  dedicatedIOWorker(nullptr)
{
    property.name = name;
    property.size = size;
    property.cls = DeviceClass::SYSTEM;
    property.numa = UNKNOWN_NUMA_NODE;

    for (auto& iter : contextArray)
    {
        iter = nullptr;
    }
}

bool
UBlockDevice::_IsCurrentCoreRegistered(void)
{
    return (contextArray[currentThreadVirtualId] != nullptr);
}

bool
UBlockDevice::_IsCoreRegistered(uint32_t coreId)
{
    return (contextArray[coreId] != nullptr);
}

bool
UBlockDevice::_RegisterContextToCurrentCore(DeviceContext* devCtx)
{
    if (currentThreadVirtualId == ID_NOT_ALLOCATED)
    {
        currentThreadVirtualId = ++lastVirtualId;
        if (MAX_THREAD_COUNT <= currentThreadVirtualId)
        {
            POS_EVENT_ID eventId = POS_EVENT_ID::DEVICE_THREAD_REGISTERED_FAILED;
            POS_TRACE_ERROR(static_cast<int>(eventId),
                PosEventId::GetString(eventId));

            return false;
        }
    }

    if (contextArray[currentThreadVirtualId] == nullptr)
    {
        contextArray[currentThreadVirtualId] = devCtx;
        ++deviceContextCount;
        return true;
    }
    return false;
}

void
UBlockDevice::_UnRegisterContextToCurrentCore(void)
{
    if (contextArray[currentThreadVirtualId] != nullptr)
    {
        contextArray[currentThreadVirtualId] = nullptr;
        deviceContextCount--;
    }
}

UBlockDevice::~UBlockDevice(void)
{
}

bool
UBlockDevice::_RegisterThread(void)
{
    bool ret = _RegisterContextToCurrentCore(_AllocateDeviceContext());
    if (ret == false)
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::DEVICE_THREAD_REGISTERED_FAILED;
        POS_TRACE_ERROR(static_cast<int>(eventId),
            PosEventId::GetString(eventId), GetName());
        return false;
    }
    return true;
}

uint32_t
UBlockDevice::Close(void)
{
    uint32_t completionCount = 0;

    DeviceContext* devCtx = _GetDeviceContext();
    if (devCtx != nullptr)
    {
        completionCount = _Empty(devCtx);
        _CloseDeviceDriver(devCtx);
        _ReleaseDeviceContext(devCtx);
        _UnRegisterContextToCurrentCore();
    }

    return completionCount;
}

bool
UBlockDevice::IsAlive(void)
{
    // TODO check device is online or not
    return true;
}

DeviceContext*
UBlockDevice::_GetDeviceContext(void)
{
    return contextArray[currentThreadVirtualId];
}

bool
UBlockDevice::Open(void)
{
    bool returnValue = false;
    returnValue = _RegisterThread();
    if (returnValue == false)
    {
        return false;
    }
    DeviceContext* devCtx = _GetDeviceContext();
    returnValue = _OpenDeviceDriver(devCtx);
    if (returnValue == false)
    {
        _ReleaseDeviceContext(devCtx);
        _UnRegisterContextToCurrentCore();
        return false;
    }

    returnValue = _WrapupOpenDeviceSpecific(devCtx);

    return returnValue;
}

bool
UBlockDevice::_OpenDeviceDriver(DeviceContext* deviceContextToOpen)
{
    bool openSuccessful = driver->Open(deviceContextToOpen);
    return openSuccessful;
}

bool
UBlockDevice::_WrapupOpenDeviceSpecific(DeviceContext* devicecontext)
{
    return true;
}

bool
UBlockDevice::_CloseDeviceDriver(DeviceContext* deviceContextToClose)
{
    bool closeSuccessful = driver->Close(deviceContextToClose);
    return closeSuccessful;
}

int
UBlockDevice::SubmitAsyncIO(UbioSmartPtr bio)
{
    int completions = 0;
    DeviceContext* deviceContext = _GetDeviceContext();
    if (likely(deviceContext == nullptr))
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::DEVICE_CONTEXT_NOT_FOUND;
        POS_TRACE_DEBUG(eventId, PosEventId::GetString(eventId), GetName());
        IoCompleter ioCompleter(bio);
        ioCompleter.CompleteUbio(IOErrorType::DEVICE_ERROR, true);
        completions++;
        return completions;
    }
    completions = driver->SubmitAsyncIO(deviceContext, bio);
    ProfilePendingIoCount(deviceContext->GetPendingIOCount());
    return completions;
}

void*
UBlockDevice::GetByteAddress(void)
{
    return nullptr;
}

int
UBlockDevice::CompleteIOs(void)
{
    int completions = 0;
    DeviceContext* deviceContext = _GetDeviceContext();
    if (unlikely(deviceContext == nullptr))
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::DEVICE_CONTEXT_NOT_FOUND;
        POS_TRACE_ERROR(eventId, PosEventId::GetString(eventId), GetName());
        return 0;
    }
    if (0 < deviceContext->GetPendingIOCount())
    {
        completions = driver->CompleteIOs(deviceContext);
    }

    if (0 < deviceContext->GetPendingErrorCount())
    {
        completions += driver->CompleteErrors(deviceContext);
    }

    return completions;
}

uint32_t
UBlockDevice::_Empty(DeviceContext* deviceContext)
{
    uint32_t totalCompletions = 0;
    if (nullptr != deviceContext)
    {
        // Retry Logic decrease pendingError and increase PendingIOCount again.
        // So, we need to check both simultaneously
        while (0 < deviceContext->GetPendingIOCount() + deviceContext->GetPendingErrorCount())
        {
            totalCompletions += driver->CompleteIOs(deviceContext);
            totalCompletions += driver->CompleteErrors(deviceContext);
        }
    }
    return totalCompletions;
}

void
UBlockDevice::ProfilePendingIoCount(uint32_t pendingIOCount)
{
    if (GetType() == DeviceType::NVRAM)
    {
        if (EventFrameworkApiSingleton::Instance()->IsReactorNow() == true)
        {
            uint32_t reactor_id = EventFrameworkApiSingleton::Instance()->GetCurrentReactor();
            airlog("CNT_PendingIO", "AIR_NVRAM", reactor_id, pendingIOCount);
        }
        else
        {
            airlog("CNT_PendingIO", "AIR_NVRAM", IO_WORKER_AID, pendingIOCount);
        }
    }
    else if (GetType() == DeviceType::SSD)
    {
        if (EventFrameworkApiSingleton::Instance()->IsReactorNow() == true)
        {
            uint32_t reactor_id = EventFrameworkApiSingleton::Instance()->GetCurrentReactor();
            airlog("CNT_PendingIO", "AIR_SSD", reactor_id, pendingIOCount);
        }
        else
        {
            airlog("CNT_PendingIO", "AIR_SSD", IO_WORKER_AID, pendingIOCount);
        }
    }
}

void
UBlockDevice::AddPendingErrorCount(uint32_t errorsToAdd)
{
    uint32_t oldPendingErrorCount = pendingErrorCount.fetch_add(errorsToAdd);
    if (unlikely((UINT32_MAX - oldPendingErrorCount) < errorsToAdd))
    {
        POS_EVENT_ID eventId =
            POS_EVENT_ID::DEVICE_UNEXPECTED_PENDING_ERROR_COUNT;
        POS_TRACE_ERROR(static_cast<int>(eventId),
            PosEventId::GetString(eventId),
            "overflow!!", oldPendingErrorCount, errorsToAdd);
    }
}

void
UBlockDevice::SubtractPendingErrorCount(uint32_t errorsToSubtract)
{
    uint32_t oldPendingErrorCount = pendingErrorCount.fetch_sub(errorsToSubtract);
    if (unlikely(oldPendingErrorCount < errorsToSubtract))
    {
        POS_EVENT_ID eventId =
            POS_EVENT_ID::DEVICE_UNEXPECTED_PENDING_ERROR_COUNT;
        POS_TRACE_ERROR(static_cast<int>(eventId),
            PosEventId::GetString(eventId),
            "underflow!!", oldPendingErrorCount, errorsToSubtract);
    }
}

uint32_t
UBlockDevice::GetPendingErrorCount(void)
{
    return pendingErrorCount;
}

void
UBlockDevice::SetDedicatedIOWorker(IOWorker* ioWorker)
{
    if (unlikely(dedicatedIOWorker != nullptr))
    {
        POS_EVENT_ID eventId =
            POS_EVENT_ID::DEVICE_OVERLAPPED_SET_IOWORKER;
        POS_TRACE_WARN(static_cast<int>(eventId),
            PosEventId::GetString(eventId),
            GetName());
    }
    dedicatedIOWorker = ioWorker;
}

IOWorker*
UBlockDevice::GetDedicatedIOWorker(void)
{
    if (unlikely(dedicatedIOWorker == nullptr))
    {
        POS_EVENT_ID eventId =
            POS_EVENT_ID::DEVICE_NULLPTR_IOWORKER;
        POS_TRACE_WARN(static_cast<int>(eventId),
            PosEventId::GetString(eventId),
            GetName());
    }
    return dedicatedIOWorker;
}

} // namespace pos

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

#include "ublock_device.h"

#include <sched.h>

#include "Air.h"
#include "device_context.h"
#include "device_driver.h"
#include "src/device/event_framework_api.h"
#include "src/include/branch_prediction.h"
#include "src/include/ibof_event_id.hpp"
#include "src/io/general_io/affinity_manager.h"
#include "src/logger/logger.h"
#include "src/scheduler/event_argument.h"
#include "src/scheduler/io_dispatcher.h"

namespace ibofos
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

    for (auto& iter : contextArray)
    {
        iter = nullptr;
    }
}

bool
UBlockDevice::_IsCurrentCoreRegistered()
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
            IBOF_EVENT_ID eventId = IBOF_EVENT_ID::DEVICE_THREAD_REGISTERED_FAILED;
            IBOF_TRACE_ERROR(static_cast<int>(eventId),
                IbofEventId::GetString(eventId));

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
UBlockDevice::_UnRegisterContextToCurrentCore()
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
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::DEVICE_THREAD_REGISTERED_FAILED;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId), GetName());
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
    if (unlikely(contextArray[currentThreadVirtualId] == nullptr))
    {
        throw IBOF_EVENT_ID::DEVICE_CONTEXT_NOT_FOUND;
    }
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
    try
    {
        int completions = 0;
        DeviceContext* deviceContext = _GetDeviceContext();
        completions = driver->SubmitAsyncIO(deviceContext, bio);
        ProfilePendingIoCount(deviceContext->GetPendingIOCount());
        return completions;
    }
    catch (IBOF_EVENT_ID eventId)
    {
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId), GetName());
        return 0;
    }
}

int
UBlockDevice::CompleteIOs(void)
{
    try
    {
        int completions = 0;
        DeviceContext* deviceContext = _GetDeviceContext();
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
    catch (IBOF_EVENT_ID eventId)
    {
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId), GetName());
        return 0;
    }
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
        if (EventFrameworkApi::IsReactorNow() == true)
        {
            uint32_t aid = EventFrameworkApi::GetCurrentReactor();
            AIRLOG(Q_NVRAM, aid, pendingIOCount, pendingIOCount);
        }
        else
        {
            AIRLOG(Q_NVRAM, IO_WORKER_AID, pendingIOCount, pendingIOCount);
        }
    }
    else if (GetType() == DeviceType::SSD)
    {
        if (EventFrameworkApi::IsReactorNow() == true)
        {
            uint32_t aid = EventFrameworkApi::GetCurrentReactor();
            AIRLOG(Q_SSD, aid, pendingIOCount, pendingIOCount);
        }
        else
        {
            AIRLOG(Q_SSD, IO_WORKER_AID, pendingIOCount, pendingIOCount);
        }
    }
}

void
UBlockDevice::AddPendingErrorCount(uint32_t errorsToAdd)
{
    uint32_t oldPendingErrorCount = pendingErrorCount.fetch_add(errorsToAdd);
    if (unlikely((UINT32_MAX - oldPendingErrorCount) < errorsToAdd))
    {
        IBOF_EVENT_ID eventId =
            IBOF_EVENT_ID::DEVICE_UNEXPECTED_PENDING_ERROR_COUNT;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId),
            "overflow!!", oldPendingErrorCount, errorsToAdd);
    }
}

void
UBlockDevice::SubtractPendingErrorCount(uint32_t errorsToSubtract)
{
    uint32_t oldPendingErrorCount = pendingErrorCount.fetch_sub(errorsToSubtract);
    if (unlikely(oldPendingErrorCount < errorsToSubtract))
    {
        IBOF_EVENT_ID eventId =
            IBOF_EVENT_ID::DEVICE_UNEXPECTED_PENDING_ERROR_COUNT;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId),
            "underflow!!", oldPendingErrorCount, errorsToSubtract);
    }
}

uint32_t
UBlockDevice::GetPendingErrorCount(void)
{
    return pendingErrorCount;
}

void
UBlockDevice::SetErrorDisregard(bool errorAsSuccess)
{
    completeErrorAsSuccess = errorAsSuccess;
}

bool
UBlockDevice::GetErrorDisregard(void)
{
    return completeErrorAsSuccess;
}

void
UBlockDevice::SetDedicatedIOWorker(IOWorker* ioWorker)
{
    if (unlikely(dedicatedIOWorker != nullptr))
    {
        IBOF_EVENT_ID eventId =
            IBOF_EVENT_ID::DEVICE_OVERLAPPED_SET_IOWORKER;
        IBOF_TRACE_WARN(static_cast<int>(eventId),
            IbofEventId::GetString(eventId),
            GetName());
    }
    dedicatedIOWorker = ioWorker;
}

IOWorker*
UBlockDevice::GetDedicatedIOWorker(void)
{
    if (unlikely(dedicatedIOWorker == nullptr))
    {
        IBOF_EVENT_ID eventId =
            IBOF_EVENT_ID::DEVICE_NULLPTR_IOWORKER;
        IBOF_TRACE_WARN(static_cast<int>(eventId),
            IbofEventId::GetString(eventId),
            GetName());
    }
    return dedicatedIOWorker;
}

} // namespace ibofos

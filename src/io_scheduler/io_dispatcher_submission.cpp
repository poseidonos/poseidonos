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

#include "src/array_mgmt/array_manager.h"
#include "src/array_models/interface/i_array_info.h"
#include "src/cpu_affinity/affinity_manager.h"
#include "src/device/base/ublock_device.h"
#include "src/event_scheduler/spdk_event_scheduler.h"
#include "src/include/array_mgmt_policy.h"
#include "src/include/i_array_device.h"
#include "src/include/pos_event_id.hpp"
#include "src/io_scheduler/dispatcher_policy.h"
#include "src/io_scheduler/io_dispatcher_submission.h"
#include "src/io_scheduler/io_worker.h"
#include "src/logger/logger.h"
#include "src/spdk_wrapper/event_framework_api.h"

namespace pos
{

SchedulingInIODispatcher IODispatcherSubmission::scheduler[BackendEvent_Count];

IODispatcherSubmission::IODispatcherSubmission(void)
{
    ioReactorCount = 0;
    for (uint32_t coreIndex = 0; coreIndex < MAX_CORE; coreIndex++)
    {
        ioReactorCore[coreIndex] = 0;
    }
    for (uint32_t coreIndex = 0; coreIndex < MAX_CORE; coreIndex++)
    {
        if (AffinityManagerSingleton::Instance()->IsIoReactor(coreIndex))
        {
            ioReactorCore[ioReactorCount] = coreIndex;
            ioReactorCount++;
        }
    }
    for (uint32_t event = 0; event < BackendEvent_Count; event++)
    {
        scheduler[event].schedType = SchedulingType::NoMatter;
        scheduler[event].readMaxBw = NO_THROTTLING_VALUE;
        scheduler[event].writeMaxBw = NO_THROTTLING_VALUE;
        scheduler[event].remainingRead = scheduler[event].readMaxBw;
        scheduler[event].remainingWrite = scheduler[event].writeMaxBw;
        scheduler[event].reactorRatio = 0;
        scheduler[event].reactorStart = 0;
        scheduler[event].reactorCount = 0;
    }
    IODispatcherSubmission::_SetRebuildImpact(RebuildImpact::High);
    schedulingOn = false;
}

IODispatcherSubmission::~IODispatcherSubmission(void)
{
}

bool
IODispatcherSubmission::CheckThrottledAndConsumeRemaining(UbioSmartPtr ubio)
{
    uint32_t eventType = ubio->GetEventType();
    UbioDir dir = ubio->dir;
    if (scheduler[eventType].schedType == SchedulingType::ReactorRatioAndThrottling)
    {
        if (dir == UbioDir::Write)
        {
            if (scheduler[eventType].remainingWrite < 0)
            {
                return false;
            }
            scheduler[eventType].remainingWrite -= ubio->GetSize();
        }
        else if (dir == UbioDir::Read)
        {
            if (scheduler[eventType].remainingRead < 0)
            {
                return false;
            }
            scheduler[eventType].remainingRead -= ubio->GetSize();
        }
    }
    return true;
}

void
IODispatcherSubmission::_SubmitIOInReactor(void* ptr1, void* ptr2)
{
    UBlockDevice* ublock = (UBlockDevice*)ptr1;
    UbioSmartPtr* ubio = (UbioSmartPtr*)ptr2;
    bool ret = CheckThrottledAndConsumeRemaining(*ubio);
    if (ret)
    {
        ublock->SubmitAsyncIO(*ubio);
        delete ubio;
    }
    else
    {
        uint32_t targetReactorCore
            = EventFrameworkApiSingleton::Instance()->GetCurrentReactor();
        EventFrameworkApiSingleton::Instance()->SendSpdkEvent(
            targetReactorCore,
            _SubmitIOInReactor,
            ublock, (void*)ubio);
    }
}

void
IODispatcherSubmission::SetMaxBw(BackendEvent event, bool read, uint64_t max)
{
    if (read)
    {
        scheduler[event].readMaxBw = max;
    }
    else
    {
        scheduler[event].writeMaxBw = max;
    }
}

void
IODispatcherSubmission::SetReactorRatio(BackendEvent event, uint32_t ratio)
{
    scheduler[event].reactorRatio = ratio;
}

bool
IODispatcherSubmission::_IsReactorScheduledNeeded(SchedulingType schedType)
{
    return ((schedType == SchedulingType::ReactorRatio
        || schedType == SchedulingType::ReactorRatioAndThrottling) && schedulingOn == true);
}

int
IODispatcherSubmission::SubmitIOInReactor(UBlockDevice* ublock, UbioSmartPtr ubio)
{
    UbioSmartPtr* smartPtr = new UbioSmartPtr(ubio);
    uint32_t eventType = ubio->GetEventType();
    static std::atomic<uint32_t> lastIndexCommon;
    SchedulingType schedType = scheduler[eventType].schedType;
    uint32_t lastIndex = lastIndexCommon;
    uint64_t reactorStart = 0;
    uint64_t reactorCount = ioReactorCount;
    if (_IsReactorScheduledNeeded(schedType) == true)
    {
        lastIndex = scheduler[eventType].lastIndex;
        reactorStart = scheduler[eventType].reactorStart;
        reactorCount = scheduler[eventType].reactorCount;
    }

    uint32_t targetReactorCore = ioReactorCore[lastIndex];

    if (ubio->GetOriginCore() == INVALID_CORE)
    {
        ubio->SetOriginCore(targetReactorCore);
    }
    bool ret = true;
    scheduler[eventType].busyDetector++;
    ret = EventFrameworkApiSingleton::Instance()->SendSpdkEvent(targetReactorCore,
            _SubmitIOInReactor,
            ublock, (void*)smartPtr);
    uint32_t nextLastIndex = (lastIndex + 1) % reactorCount + reactorStart;

    if (_IsReactorScheduledNeeded(schedType) == true)
    {
        scheduler[eventType].lastIndex = nextLastIndex;
    }
    else
    {
        lastIndexCommon = nextLastIndex;
    }
    return ret;
}

void
IODispatcherSubmission::UpdateReactorRatio(void)
{
    uint32_t totalReactorRatio = 0;
    scheduler[BackendEvent_FrontendIO].schedType = SchedulingType::NoMatter;
    scheduler[BackendEvent_UserdataRebuild].schedType = SchedulingType::NoMatter;
    for (uint32_t event = 0; event < BackendEvent_Count; event++)
    {
        totalReactorRatio += scheduler[event].reactorRatio;
    }
    uint32_t currentStartReactor = 0;
    uint32_t lastEventWithRatioSet = 0;
    for (uint32_t event = 0; event < BackendEvent_Count; event++)
    {
        if (scheduler[event].reactorRatio != 0)
        {
            scheduler[event].reactorStart = currentStartReactor;
            uint32_t reactorCountForEvent = scheduler[event].reactorRatio * ioReactorCount / totalReactorRatio;
            scheduler[event].reactorCount = reactorCountForEvent;
            currentStartReactor += reactorCountForEvent;
            lastEventWithRatioSet = event;
        }
    }
    scheduler[lastEventWithRatioSet].reactorCount = ioReactorCount - scheduler[lastEventWithRatioSet].reactorStart;
    scheduler[BackendEvent_FrontendIO].schedType = SchedulingType::ReactorRatio;
    scheduler[BackendEvent_UserdataRebuild].schedType = SchedulingType::ReactorRatioAndThrottling;
}

void
IODispatcherSubmission::RefillRemaining(uint64_t scale)
{
    for (uint32_t event = 0; event < BackendEvent_Count; event++)
    {
        if (scheduler[event].schedType == SchedulingType::ReactorRatioAndThrottling)
        {
            scheduler[event].remainingRead = scheduler[event].readMaxBw / scale;
            scheduler[event].remainingWrite = scheduler[event].writeMaxBw / scale;
        }
    }
}

void
IODispatcherSubmission::CheckAndSetBusyMode(void)
{
    bool busyLocal = false;
    for (uint32_t arrayId = 0; arrayId < ArrayMgmtPolicy::MAX_ARRAY_CNT; arrayId++)
    {
        ComponentsInfo* compInfo = ArrayMgr()->GetInfo(arrayId);
        if (unlikely(compInfo == nullptr))
        {
            continue;
        }
        IArrayInfo* info = compInfo->arrayInfo;
        if (unlikely(info == nullptr))
        {
            continue;
        }
        IStateControl* stateControl = StateManagerSingleton::Instance()->GetStateControl(info->GetName());
        bool isBusyState = (stateControl->GetState()->ToStateType() == StateEnum::BUSY);
        busyLocal = (busyLocal || isBusyState);
    }
    schedulingOn = busyLocal;
    if (scheduler[BackendEvent_FrontendIO].busyDetector == 0)
    {
        scheduler[BackendEvent_FrontendIO].schedType = SchedulingType::NoMatter;
    }
    else
    {
        scheduler[BackendEvent_FrontendIO].schedType = SchedulingType::ReactorRatioAndThrottling;
    }
    scheduler[BackendEvent_FrontendIO].busyDetector = 0;
}

void
IODispatcherSubmission::SetRebuildImpact(RebuildImpact rebuildImpact)
{
    _SetRebuildImpact(rebuildImpact);
}

void
IODispatcherSubmission::_SetRebuildImpact(RebuildImpact rebuildImpact)
{
    switch(rebuildImpact)
    {
        case RebuildImpact::High:
        {
            SetReactorRatio(BackendEvent_FrontendIO, 15);
            SetReactorRatio(BackendEvent_UserdataRebuild, 35);
            SetMaxBw(BackendEvent_UserdataRebuild, false, SSD_REBUILD_WRITE_MAX_BW * 1024 * 1024);
            break;
        }
        case RebuildImpact::Medium:
        {
            SetReactorRatio(BackendEvent_FrontendIO, 25);
            SetReactorRatio(BackendEvent_UserdataRebuild, 25);
            SetMaxBw(BackendEvent_UserdataRebuild, false, SSD_REBUILD_WRITE_MAX_BW / 3 * 1024 * 1024);
            break;
        }
        default:
        {
            SetReactorRatio(BackendEvent_FrontendIO, 49);
            SetReactorRatio(BackendEvent_UserdataRebuild, 1);
            SetMaxBw(BackendEvent_UserdataRebuild, false, SSD_REBUILD_WRITE_MAX_BW / 35 * 1024 * 1024);
            break;
        }
    }
    UpdateReactorRatio();
}

int
IODispatcherSubmission::SubmitIO(UBlockDevice* ublock,
    UbioSmartPtr ubio, DispatcherPolicyI* dispatcherPolicy)
{
    int ret = 0;
    bool isReactor = EventFrameworkApiSingleton::Instance()->IsReactorNow();
    if (isReactor)
    {
        if ((AffinityManagerSingleton::Instance()->UseEventReactor()
            && ubio->GetOriginCore() == INVALID_CORE) || schedulingOn)
        {
            ret = SubmitIOInReactor(ublock, ubio);
        }
        else
        {
            ret = ublock->SubmitAsyncIO(ubio);
        }
    }
    else
    {
        IOWorker* ioWorker = nullptr;
        if (AffinityManagerSingleton::Instance()->UseEventReactor())
        {
            ret = SubmitIOInReactor(ublock, ubio);
        }
        else
        {
            ioWorker = ublock->GetDedicatedIOWorker();
        }
        if (likely(nullptr != ioWorker))
        {
            dispatcherPolicy->Submit(ioWorker, ubio);
        }
    }
    return ret;
}
} // namespace pos

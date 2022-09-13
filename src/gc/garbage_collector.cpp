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

#include "src/gc/garbage_collector.h"

#include <algorithm>
#include <future>
#include <memory>

#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/event_scheduler/event.h"
#include "src/event_scheduler/event_scheduler.h"

namespace pos
{
GarbageCollector::GarbageCollector(IArrayInfo* i, IStateControl* s)
: GarbageCollector(i, s, nullptr, nullptr, EventSchedulerSingleton::Instance())
{
    this->copierFactory = [](GcStatus* gcStatus, IArrayInfo* array, CopierSmartPtr inputEvent)
    {
        return std::make_shared<Copier>(UNMAP_SEGMENT, UNMAP_SEGMENT, gcStatus, array);
    };
}

GarbageCollector::GarbageCollector(IArrayInfo* i, IStateControl* s,
                                CopierSmartPtr inputEvent,
                                function<CopierSmartPtr(GcStatus*, IArrayInfo*, CopierSmartPtr)> copierFactory,
                                EventScheduler* inputEventScheduler)
: arrayInfo(i),
  state(s),
  gcStatus(),
  copierFactory(copierFactory),
  inputEvent(inputEvent),
  eventScheduler(inputEventScheduler)
{
}

int GarbageCollector::Init(void)
{
    state->Subscribe(this, typeid(*this).name());
    int returnVal = Start();
    if (unlikely(returnVal != EID(SUCCESS)))
    {
        state->Unsubscribe(this);
    }
    return returnVal;
}

void GarbageCollector::Dispose(void)
{
    End();
    state->Unsubscribe(this);
}

void GarbageCollector::Shutdown(void)
{
    Dispose();
}

void GarbageCollector::Flush(void)
{
    // no-op for IMountSequence
}

void GarbageCollector::Pause(void)
{
    if (nullptr != copierPtr)
    {
        copierPtr->Pause();
    }
    else
    {
        POS_TRACE_INFO(EID(GC_COPIER_NOT_EXIST), "gc copierPtr not exist");
    }
}

void GarbageCollector::Resume(void)
{
    if (nullptr != copierPtr)
    {
        copierPtr->Resume();
    }
    else
    {
        POS_TRACE_INFO(EID(GC_COPIER_NOT_EXIST), "gc copierPtr not exist");
    }
}

bool GarbageCollector::IsPaused(void)
{
    bool ret = true;
    if (nullptr != copierPtr)
    {
        ret = copierPtr->IsPaused();
    }
    else
    {
        POS_TRACE_INFO(EID(GC_COPIER_NOT_EXIST), "gc copierPtr not exist");
    }
    return ret;
}


void GarbageCollector::StateChanged(StateContext* prev, StateContext* next)
{
}

int
GarbageCollector::IsEnabled(void)
{
    int returnVal = EID(SUCCESS);

    StateEnum currState = state->GetState()->ToStateType();

    bool isEnabled = ((currState == StateEnum::NORMAL) ||
                       (currState == StateEnum::BUSY));

    if (false == isEnabled)
    {
        POS_TRACE_INFO(EID(GC_CANNOT_START), "cannot start gc");
        return EID(GC_CANNOT_START);
    }

    if (true == isRunning)
    {
        POS_TRACE_INFO(EID(GC_STARTED), "gc already running");
    }

    return returnVal;
}

int
GarbageCollector::Start(void)
{
    int returnVal = -1;

    if (false == isRunning)
    {
        POS_TRACE_INFO(EID(GC_STARTED), "gc started");
        returnVal = _DoGC();
        if (likely(returnVal == EID(SUCCESS)))
        {
            isRunning = true;
        }
    }
    else
    {
        POS_TRACE_INFO(EID(GC_STARTED), "gc already running");
        returnVal = EID(SUCCESS);
    }

    return returnVal;
}

int
GarbageCollector::DisableThresholdCheck(void)
{
    if (false == copierPtr->IsEnableThresholdCheck())
    {
        POS_TRACE_INFO(EID(GC_THRESHOLD_CHECK_DISABLE), "threshold check is already disabled");

        return -1;
    }

    copierPtr->DisableThresholdCheck();
    POS_TRACE_INFO(EID(GC_THRESHOLD_CHECK_DISABLE), "threshold check is disabled");
    return 0;
}

void
GarbageCollector::End(void)
{
    if (true == isRunning)
    {
        copierPtr->Stop();

        do
        {
            usleep(1);
        } while (true == copierPtr->IsStopped());
        isRunning = false;
        _GCdone();
    }
}

int
GarbageCollector::_DoGC(void)
{
    POS_TRACE_INFO(EID(GC_STARTED), "GC started");

    CopierSmartPtr event;
    event = copierFactory(&gcStatus, arrayInfo, inputEvent);

    if (unlikely(event == nullptr))
    {
        POS_TRACE_ERROR(EID(GC_CANNOT_CREATE_COPIER),
            "gc can not create copier event");
        return EID(GC_CANNOT_CREATE_COPIER);
    }
    eventScheduler->EnqueueEvent(event);
    copierPtr = event;
    return EID(SUCCESS);
}

void
GarbageCollector::_GCdone(void)
{
    copierPtr->ReadyToEnd();
    copierPtr = nullptr;
    POS_TRACE_INFO(EID(GC_DONE), "GC done");
}
} // namespace pos

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

#include "src/gc/garbage_collector.h"

#include <algorithm>
#include <future>

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/event_scheduler/event.h"
#include "src/event_scheduler/event_scheduler.h"

namespace pos
{
GarbageCollector::GarbageCollector(IArrayInfo* i, IStateControl* s)
: GarbageCollector(i, s, nullptr, EventSchedulerSingleton::Instance())
{
}

GarbageCollector::GarbageCollector(IArrayInfo* i, IStateControl* s,
                                CopierSmartPtr inputEvent, EventScheduler* inputEventScheduler)
: arrayInfo(i),
  state(s),
  gcStatus(),
  inputEvent(inputEvent),
  eventScheduler(inputEventScheduler)
{
}

int GarbageCollector::Init(void)
{
    state->Subscribe(this, typeid(*this).name());
    return Start();
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
        POS_TRACE_INFO(static_cast<int>(POS_EVENT_ID::GC_COPIER_NOT_EXIST), "gc copierPtr not exist");
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
        POS_TRACE_INFO(static_cast<int>(POS_EVENT_ID::GC_COPIER_NOT_EXIST), "gc copierPtr not exist");
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
        POS_TRACE_INFO(static_cast<int>(POS_EVENT_ID::GC_COPIER_NOT_EXIST), "gc copierPtr not exist");
    }
    return ret;
}


void GarbageCollector::StateChanged(StateContext* prev, StateContext* next)
{
}

int
GarbageCollector::IsEnabled(void)
{
    int returnVal = static_cast<int>(POS_EVENT_ID::SUCCESS);

    StateEnum currState = state->GetState()->ToStateType();

    bool isEnabled = ((currState == StateEnum::NORMAL) ||
                       (currState == StateEnum::BUSY));

    if (false == isEnabled)
    {
        POS_TRACE_INFO(static_cast<int>(POS_EVENT_ID::GC_CANNOT_START), "cannot start gc");
        return static_cast<int>(POS_EVENT_ID::GC_CANNOT_START);
    }

    if (true == isRunning)
    {
        POS_TRACE_INFO(static_cast<int>(POS_EVENT_ID::GC_STARTED), "gc already running");
    }

    return returnVal;
}

int
GarbageCollector::Start(void)
{
    int returnVal = -1;

    if (false == isRunning)
    {
        POS_TRACE_INFO(static_cast<int>(POS_EVENT_ID::GC_STARTED), "gc started");
        _DoGC();
        isRunning = true;
        returnVal = static_cast<int>(POS_EVENT_ID::SUCCESS);
    }
    else
    {
        POS_TRACE_INFO(static_cast<int>(POS_EVENT_ID::GC_STARTED), "gc already running");
        returnVal = static_cast<int>(POS_EVENT_ID::SUCCESS);
    }

    return returnVal;
}

int
GarbageCollector::DisableThresholdCheck(void)
{
    if (false == copierPtr->IsEnableThresholdCheck())
    {
        return -1;
    }

    copierPtr->DisableThresholdCheck();
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

void
GarbageCollector::_DoGC(void)
{
    POS_TRACE_INFO(static_cast<int>(POS_EVENT_ID::GC_STARTED), "GC started");

    CopierSmartPtr event;
    if (nullptr == inputEvent)
    {
        event = std::make_shared<Copier>(UNMAP_SEGMENT, UNMAP_SEGMENT, &gcStatus, arrayInfo);
    }
    else
    {
        event = inputEvent;
    }
    eventScheduler->EnqueueEvent(event);
    copierPtr = event;
}

void
GarbageCollector::_GCdone(void)
{
    copierPtr->ReadyToEnd();
    copierPtr = nullptr;
    POS_TRACE_INFO(static_cast<int>(POS_EVENT_ID::GC_DONE), "GC done");
}
} // namespace pos

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

#include "src/include/ibof_event_id.h"
#include "src/logger/logger.h"
#include "src/scheduler/event.h"
#include "src/scheduler/event_argument.h"

namespace ibofos
{
GarbageCollector::GarbageCollector(void)
: StateEvent("GarbageCollector"),
  gcStatus()
{
    StateManagerSingleton::Instance()->Subscribe(this);
}

int
GarbageCollector::IsGcPossible(void)
{
    int returnVal = static_cast<int>(IBOF_EVENT_ID::SUCCESS);

    State ibofState = currState.GetState();
    bool isPossible = ((ibofState == State::NORMAL) || (ibofState == State::BUSY));

    if (false == isPossible)
    {
        IBOF_TRACE_INFO(static_cast<int>(IBOF_EVENT_ID::GC_CANNOT_START), "cannot start gc");
        return static_cast<int>(IBOF_EVENT_ID::GC_CANNOT_START);
    }

    if (true == isRunning)
    {
        IBOF_TRACE_INFO(static_cast<int>(IBOF_EVENT_ID::GC_STARTED), "gc already running");
    }

    return returnVal;
}

int
GarbageCollector::Start(void)
{
    int returnVal = -1;

    if (false == isRunning)
    {
        IBOF_TRACE_INFO(static_cast<int>(IBOF_EVENT_ID::GC_STARTED), "gc started");
        _DoGC();
        isRunning = true;
        returnVal = static_cast<int>(IBOF_EVENT_ID::SUCCESS);
    }
    else
    {
        IBOF_TRACE_INFO(static_cast<int>(IBOF_EVENT_ID::GC_STARTED), "gc already running");
        returnVal = static_cast<int>(IBOF_EVENT_ID::SUCCESS);
    }

    return returnVal;
}

int
GarbageCollector::DisableThreshold(void)
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
    IBOF_TRACE_INFO(3401, "GC started");
    CopierSmartPtr event(new Copier(UNMAP_SEGMENT, UNMAP_SEGMENT, &gcStatus));
    EventArgument::GetEventScheduler()->EnqueueEvent(event);
    copierPtr = event;
}

void
GarbageCollector::_GCdone(void)
{
    copierPtr->ReadyToEnd();
    copierPtr = nullptr;
    IBOF_TRACE_INFO(static_cast<int>(IBOF_EVENT_ID::GC_DONE), "GC done");
}

void
GarbageCollector::StateChanged(StateContext prev, StateContext next)
{
    currState = next;
    if (currState.GetState() == State::STOP)
    {
        End();
    }
}

} // namespace ibofos

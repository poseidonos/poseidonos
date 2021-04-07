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

#include <vector>

#include "src/gc/copier.h"
#include "src/gc/gc_status.h"
#include "src/lib/singleton.h"
#include "src/state/state_manager.h"

using namespace std;

namespace ibofos
{
class Copier;
using CopierSmartPtr = std::shared_ptr<Copier>;

class GarbageCollector : StateEvent
{
public:
    GarbageCollector(void);
    ~GarbageCollector(void)
    {
        StateManagerSingleton::Instance()->Dispose(this);
    }

    int Start(void);
    void End(void);
    void
    Pause(void)
    {
        copierPtr->Pause();
    }
    void
    Resume(void)
    {
        copierPtr->Resume();
    }
    bool
    IsPause(void)
    {
        return copierPtr->IsPause();
    }

    int DisableThreshold(void);
    int IsGcPossible(void);

    bool
    GetGcRunning(void)
    {
        return gcStatus.GetGcRunning();
    }
    struct timeval
    GetStartTime(void)
    {
        return gcStatus.GetStartTime();
    }
    struct timeval
    GetEndTime(void)
    {
        return gcStatus.GetEndTime();
    }
    void StateChanged(StateContext prev, StateContext next);

private:
    void _DoGC(void);
    void _GCdone(void);
    StateContext currState{"garbage_collector"};
    bool isRunning = false;

    GcStatus gcStatus;
    CopierSmartPtr copierPtr;
};

using GarbageCollectorSingleton = Singleton<GarbageCollector>;
} // namespace ibofos

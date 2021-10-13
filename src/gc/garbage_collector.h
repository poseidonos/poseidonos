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

#pragma once

#include <memory>
#include <vector>

#include "src/gc/copier.h"
#include "src/gc/gc_status.h"
#include "src/gc/interface/i_gc_control.h"
#include "src/gc/interface/i_gc_info.h"
#include "src/array_models/interface/i_mount_sequence.h"
#include "src/state/interface/i_state_control.h"
#include "src/state/interface/i_state_observer.h"

using namespace std;
namespace pos
{
class Copier;
class EventScheduler;
using CopierSmartPtr = std::shared_ptr<Copier>;

class GarbageCollector : public IGCControl, public IGCInfo,
                         public IMountSequence, public IStateObserver
{
public:
    explicit GarbageCollector(IArrayInfo* i, IStateControl* s);
    GarbageCollector(IArrayInfo* i, IStateControl* s,
                    CopierSmartPtr inputEvent,
                    function<CopierSmartPtr(GcStatus*, IArrayInfo*, CopierSmartPtr)> CopierFactory,
                    EventScheduler* inputEventScheduler);
    virtual ~GarbageCollector(void) {}
    virtual int Start(void) override;
    virtual void End(void) override;

    virtual void StateChanged(StateContext* prev, StateContext* next) override;
    virtual int Init(void) override;
    virtual void Dispose(void) override;
    virtual void Shutdown(void) override;
    virtual void Flush(void) override;

    virtual void Pause(void);
    virtual void Resume(void);
    virtual bool IsPaused(void);

    virtual int DisableThresholdCheck(void);
    virtual int IsEnabled(void);

    virtual bool GetGcRunning(void) { return gcStatus.GetGcRunning(); }
    virtual struct timeval GetStartTime(void) { return gcStatus.GetStartTime(); }
    virtual struct timeval GetEndTime(void) { return gcStatus.GetEndTime(); }

private:
    int _DoGC(void);
    void _GCdone(void);
    bool isRunning = false;

    IArrayInfo* arrayInfo;
    IStateControl* state;
    GcStatus gcStatus;
    CopierSmartPtr copierPtr;

    function<CopierSmartPtr(GcStatus*, IArrayInfo*, CopierSmartPtr)> copierFactory = nullptr;
    CopierSmartPtr inputEvent;
    EventScheduler* eventScheduler;
};

} // namespace pos

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

#include "src/include/backend_event.h"
#include "src/state/interface/i_state_control.h"
#include "src/state/interface/i_state_observer.h"

class UbioSmartPtr;
class UBlockDevice;

namespace pos
{

enum class SchedulingType
{
    NoMatter,
    ReactorRatio,
    ReactorRatioAndThrottling
};

enum class RebuildImpact
{
    Low,
    Medium,
    High
};

struct SchedulingInIODispatcher
{
    SchedulingType schedType;
    SchedulingType schedTypeCLI;
    uint64_t readMaxBw;
    uint64_t writeMaxBw;
    std::atomic<int64_t> remainingRead;
    std::atomic<int64_t> remainingWrite;
    uint32_t reactorRatio;  // CLI Set
    uint32_t reactorStart; // internal used;
    uint32_t reactorCount; // internal used;
    uint64_t busyDetector;
    std::atomic<uint32_t> lastIndex;
};

class StateObserverForIO : public IStateObserver
{
public:
    explicit StateObserverForIO(IStateControl* state);
    virtual ~StateObserverForIO(void);
    void StateChanged(StateContext* prev, StateContext* next) override;
private:
    static std::atomic<int64_t> countOfRebuildingArrays;
    IStateControl* state;
};

class IODispatcherSubmission
{
public:
    explicit IODispatcherSubmission(void);
    virtual ~IODispatcherSubmission(void);

    void SetMaxBw(BackendEvent event, bool read, uint64_t max);
    void SetRebuildImpact(RebuildImpact rebuildImpact);
    void UpdateReactorRatio(void);
    void SetReactorRatio(BackendEvent event, uint32_t ratio);
    void RefillRemaining(uint64_t scale);
    int SubmitIO(UBlockDevice* ublock, UbioSmartPtr ubio, DispatcherPolicyI* dispatcherPolicy);
    void RebuildMode(void);
    void ChangeScheduleMode(bool enabled);
    void CheckAndSetBusyMode(void);

private:
    int SubmitIOInReactor(UBlockDevice* ublock, UbioSmartPtr ubio);
    bool _IsReactorScheduledNeeded(SchedulingType schedType);
    static bool _CheckThrottledAndConsumeRemaining(UbioSmartPtr ubio);
    static void _SubmitIOInReactor(void* ptr1, void* ptr2);
    void _SetRebuildImpact(RebuildImpact rebuildImpact);
    void _SetSchedType(BackendEvent event, SchedulingType schedType);
    static const uint32_t MAX_CORE = 128;
    uint32_t ioReactorCore[MAX_CORE], ioReactorCount = 0;
    static const uint64_t NO_THROTTLING_VALUE = 64 * 1024ULL * 1024ULL * 1024ULL; // 64G
    static const uint64_t SSD_REBUILD_WRITE_MAX_BW = 3500ULL * 1024ULL * 1024ULL;
    static SchedulingInIODispatcher scheduler[BackendEvent_Count];
    bool schedulingOn;
};

using IODispatcherSubmissionSingleton = Singleton<IODispatcherSubmission>;

} // namespace pos

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

#ifndef ARRAY_STATE_H_
#define ARRAY_STATE_H_

#include <condition_variable>
#include <mutex>
#include <string>

#include "src/state/state_manager.h"

namespace ibofos
{
enum class ArrayStateType
{
    NOT_EXIST,
    EXIST_NORMAL,
    EXIST_DEGRADED,
    EXIST_MISSING,
    EXIST_BROKEN,
    NORMAL,
    DEGRADED,
    REBUILD,
    BROKEN,
    TYPE_COUNT
};

class ArrayState : public StateEvent
{
public:
    ArrayState(void);
    virtual ~ArrayState(void);

    void SetLoad(uint32_t missingCnt, uint32_t brokenCnt);
    void SetCreate(void);
    void SetDelete(void);
    bool SetRebuild(void);
    void SetRebuildDone(bool isSuccess);
    int SetMount(void);
    int SetUnmount(void);
    void SetDegraded(void);

    int CanAddSpare(void);
    int CanRemoveSpare(void);
    int IsLoadable(void);
    int IsCreatable(void);
    int IsMountable(void);
    int IsUnmountable(void);
    int IsDeletable(void);
    bool IsRebuildable(void);
    bool IsRecoverable(void);

    void DataRemoved(bool isRebuildingDevice);

    bool Exists(void);
    bool IsMounted(void);
    bool IsBroken(void);

    std::string GetCurrentStateStr(void);
    void StateChanged(StateContext prev, StateContext next) override;

private:
    void _SetState(ArrayStateType newState);
    void _WaitSysStateFor(StateContext& goal, uint32_t sec);

    string sender = "array";
    StateContext currState{sender};
    StateContext degradedState{sender, Situation::DEGRADED};
    StateContext rebuildingState{sender, Situation::REBUILDING};
    StateContext stopState{sender, Situation::FAULT};

    std::mutex mtx;
    std::condition_variable cv;

    bool needForceMount = false;
    ArrayStateType state = ArrayStateType::NOT_EXIST;

    StateManager* sysStateMgr;
};

} // namespace ibofos

#endif // ARRAY_STATE_H_

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

#include "replay_state_changer.h"

namespace pos
{
ReplayStateChanger::ReplayStateChanger(void)
: recoverCtx(nullptr),
  state(nullptr)
{
}

ReplayStateChanger::ReplayStateChanger(IStateControl* iState)
: ReplayStateChanger(iState, new StateContext(typeid(this).name(), SituationEnum::JOURNAL_RECOVERY))
{
}

ReplayStateChanger::ReplayStateChanger(IStateControl* iState, StateContext* stateContext)
: recoverCtx(stateContext),
  state(iState)
{
    state->Subscribe(this, typeid(this).name());
}

ReplayStateChanger::~ReplayStateChanger(void)
{
    // TODO (huijeong.kim) nullptr check is temporal workaround for mocking
    if (state != nullptr)
    {
        state->Unsubscribe(this);
        delete recoverCtx;
    }
}

int
ReplayStateChanger::GetRecoverState(void)
{
    state->Invoke(recoverCtx);
    _WaitState(recoverCtx);

    return 0;
}

int
ReplayStateChanger::RemoveRecoverState(void)
{
    state->Remove(recoverCtx);
    return 0;
}

void
ReplayStateChanger::StateChanged(StateContext* prev, StateContext* next)
{
    std::unique_lock<std::mutex> lock(mtx);
    cv.notify_all();
}

void
ReplayStateChanger::_WaitState(StateContext* goal)
{
    std::unique_lock<std::mutex> lock(mtx);
    while (state->GetState() != goal)
    {
        cv.wait(lock);
    }
}

} // namespace pos

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

#include "state_manager.h"

#include <algorithm>

#include "src/include/ibof_event_id.h"
#include "src/logger/logger.h"

namespace ibofos
{
StateManager::StateManager()
{
    stateList = new StateList(bind(&StateManager::ListUpdated,
        this, placeholders::_1));
    stateList->Add(curr);
}

StateManager::~StateManager()
{
    delete stateList;
    stateList = nullptr;
}

void
StateManager::Subscribe(StateEvent* sub)
{
    publisher.RegisterSubscriber(sub);
}

void
StateManager::Dispose(StateEvent* sub)
{
    publisher.RemoveSubscriber(sub);
}

StateContext
StateManager::Invoke(string _sender, Situation s)
{
    StateContext ctx(_sender, s);
    async_future = async(launch::async, &StateList::Add, stateList, ctx);
    return ctx;
}

void
StateManager::Invoke(StateContext ctx)
{
    async_future = async(launch::async, &StateList::Add, stateList, ctx);
}

StateContext
StateManager::InvokeAndRemove(StateContext remove, string _sender, Situation s)
{
    StateContext add(_sender, s);
    async_future = async(launch::async, &StateList::AddandRemove, stateList, add, remove);
    return add;
}

void
StateManager::Remove(StateContext ctx)
{
    stateList->Remove(ctx);
}

bool
StateManager::Exist(StateContext ctx)
{
    return stateList->Exist(ctx);
}

bool
StateManager::ExistRebuild(void)
{
    return stateList->ExistRebuild();
}

void
StateManager::ListUpdated(StateContext front)
{
    _ChangeState(front);
}

void
StateManager::_ChangeState(StateContext next)
{
    if (curr != next)
    {
        IBOF_TRACE_INFO((int)IBOF_EVENT_ID::STATE_CHANGED,
            "STATE_CHANGED[{}] -> [{}]", curr.GetUuid(), next.GetUuid());

        StateContext prev = curr;
        curr = next;

        _NotifyState(prev, curr);
    }
}

void
StateManager::_NotifyState(StateContext& prev, StateContext& next)
{
    publisher.NotifyStateChanged(prev, next);
}

} // namespace ibofos

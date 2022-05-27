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

#include "state_control.h"
#include "state_list.h"
#include "state_publisher.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

#include <algorithm>

namespace pos
{
StateControl::StateControl(string array)
: StateControl(
    array,
    new StatePublisher(),
    new StateList(bind(&StateControl::_ListUpdated, this, placeholders::_1, placeholders::_2))
    )
{
    // delegated to other constructor
}

StateControl::StateControl(string array, StatePublisher* publisher, StateList* stateList)
: arrayName(array),
  publisher(publisher),
  stateList(stateList)
{
}

StateControl::~StateControl(void)
{
    delete publisher;
    delete stateList;
}

void
StateControl::Subscribe(IStateObserver* sub, string name)
{
    publisher->Add(sub, name);
}

void
StateControl::Unsubscribe(IStateObserver* sub)
{
    publisher->Remove(sub);
}

StateContext*
StateControl::GetState(void)
{
     return stateList->Current();
}

void
StateControl::Invoke(StateContext* ctx)
{
    async_future = async(launch::async, &StateList::Add, stateList, ctx);
}

void
StateControl::WaitOnInvokeFuture(void)
{
    // we don't expect any return value from this "future", so just "wait" should be enough.
    async_future.wait();
}

void
StateControl::Remove(StateContext* ctx)
{
    stateList->Remove(ctx);
}

bool
StateControl::Exists(SituationEnum situ)
{
    return stateList->Exists(situ);
}

void
StateControl::_ListUpdated(StateContext* prev, StateContext* next)
{
    if (prev != next)
    {
        string currSitu = prev->GetSituation().ToString();
        string nextSitu = next->GetSituation().ToString();
        POS_TRACE_TRACE(EID(POS_TRACE_ARRAY_STATE_CHANGED),
            "[{}] -> [{}], array_name:{}", currSitu, nextSitu, arrayName);
        _NotifyState(prev, next);
    }
}

void
StateControl::_NotifyState(StateContext* prev, StateContext* next)
{
    publisher->Notify(prev, next);
}

} // namespace pos

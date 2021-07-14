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

#include "state_list.h"

#include <algorithm>

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

using namespace std;

namespace pos
{
StateList::~StateList()
{
    contextList.clear();
}

void
StateList::Add(StateContext* ctx)
{
    unique_lock<mutex> lock(listMutex);
    if (Exists(ctx) == false)
    {
        contextList.push_back(ctx);
        POS_TRACE_DEBUG((int)POS_EVENT_ID::STATE_CONTEXT_UPDATED,
            "statecontext added - {}", ctx->GetSituation().ToString());
        sort(contextList.begin(), contextList.end(), _Compare);
        StateContext* next = contextList.front();
        listUpdated(next);
    }
    else
    {
        POS_TRACE_WARN((int)POS_EVENT_ID::STATE_CONTROL_DEBUG,
            "statecontext couldn't be added since it exists already - {}",
            ctx->GetSituation().ToString());
    }
}

void
StateList::Remove(StateContext* ctx)
{
    unique_lock<mutex> lock(listMutex);
    auto it = _Find(ctx);
    if (it != contextList.end())
    {
        POS_TRACE_DEBUG((int)POS_EVENT_ID::STATE_CONTEXT_UPDATED,
            "statecontext removed - {}", (*it)->GetSituation().ToString());
        contextList.erase(it);
        StateContext* next = contextList.front();
        listUpdated(next);
    }
    else
    {
        POS_TRACE_WARN((int)POS_EVENT_ID::STATE_CONTROL_DEBUG,
            "couldn't remove statecontext because it's already gone - {}",
            (ctx->GetSituation().ToString()));
    }
}

bool
StateList::Exists(StateContext* ctx)
{
    return _Find(ctx) != contextList.end();
}

bool
StateList::Exists(StateEnum state)
{
    for (auto it = contextList.begin(); it != contextList.end(); ++it)
    {
        if ((*it)->ToStateType() == state)
        {
            return true;
        }
    }
    return false;
}

bool StateList::Exists(SituationEnum situ)
{
    for (auto it = contextList.begin(); it != contextList.end(); ++it)
    {
        if ((*it)->GetSituation() == situ)
        {
            return true;
        }
    }
    return false;
}

const vector<StateContext*>&
StateList::GetContextList(void)
{
    return this->contextList;
}

void
StateList::SetContextList(const vector<StateContext*>& cList)
{
    this->contextList = cList; // copy to the internal member
}

vector<StateContext*>::iterator
StateList::_Find(StateContext* ctx)
{
    for (auto it = contextList.begin(); it != contextList.end(); ++it)
    {
        if (*it == ctx)
        {
            return it;
        }
    }
    return contextList.end();
}

bool
StateList::_Compare(StateContext* a, StateContext* b)
{
    return a->GetPriority() > b->GetPriority();
}

} // namespace pos

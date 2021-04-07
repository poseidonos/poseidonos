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

#include "src/include/ibof_event_id.h"
#include "src/logger/logger.h"
namespace ibofos
{
StateList::~StateList()
{
    contextList.clear();
}

void
StateList::Add(StateContext ctx)
{
    if (Exist(ctx) == false)
    {
        listMutex.lock();
        clock_gettime(CLOCK_REALTIME, &ctx.issued_time);
        contextList.push_back(ctx);
        IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::STATE_CONTEXT_UPDATED,
            "statecontext added - {}", ctx.GetUuid());
        sort(contextList.begin(), contextList.end());
        StateContext next = contextList.front();
        listMutex.unlock();
        listUpdated(next);
    }
}

void
StateList::Remove(StateContext ctx)
{
    auto it = Find(ctx);
    if (it != contextList.end())
    {
        listMutex.lock();
        IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::STATE_CONTEXT_UPDATED,
            "statecontext removed - {}", ctx.GetUuid());
        contextList.erase(it);
        StateContext next = contextList.front();
        listMutex.unlock();
        listUpdated(next);
    }
}

bool
StateList::Exist(StateContext ctx)
{
    return Find(ctx) != contextList.end();
}

bool
StateList::ExistRebuild(void)
{
    bool exist = false;

    listMutex.lock();
    for (StateContext ctx : contextList)
    {
        if (ctx.GetSituation() == Situation::REBUILDING)
        {
            exist = true;
            break;
        }
    }
    listMutex.unlock();
    return exist;
}

void
StateList::AddandRemove(StateContext add, StateContext remove)
{
    IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::STATE_CONTEXT_UPDATED,
        "statecontext add {} and remove {}",
        add.GetUuid(), remove.GetUuid());
    bool isChanged = false;
    listMutex.lock();
    if (Exist(add) == false)
    {
        clock_gettime(CLOCK_REALTIME, &add.issued_time);
        contextList.push_back(add);
        isChanged = true;
    }

    auto it = Find(remove);
    if (it != contextList.end())
    {
        contextList.erase(it);
        isChanged = true;
    }

    if (isChanged == true)
    {
        sort(contextList.begin(), contextList.end());
        StateContext next = contextList.front();
        listMutex.unlock();
        listUpdated(next);
    }
    else
    {
        listMutex.unlock();
    }
}

vector<StateContext>::iterator
StateList::Find(StateContext ctx)
{
    return find(contextList.begin(), contextList.end(), ctx);
}

} // namespace ibofos

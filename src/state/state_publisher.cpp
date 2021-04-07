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

#include "state_publisher.h"

#include "src/include/ibof_event_id.h"
#include "src/logger/logger.h"

namespace ibofos
{
StatePublisher::StatePublisher(void)
{
}

StatePublisher::~StatePublisher(void)
{
    subscribers.clear();
}

void
StatePublisher::RegisterSubscriber(StateEvent* subscriber)
{
    subscribers.push_back(subscriber);
}

void
StatePublisher::RemoveSubscriber(StateEvent* subscriber)
{
    for (auto it = subscribers.begin(); it != subscribers.end(); ++it)
    {
        if (*it == subscriber)
        {
            subscribers.erase(it);
            break;
        }
    }
}

void
StatePublisher::NotifyStateChanged(StateContext prev, StateContext next)
{
    StateEvent* requester = nullptr;
    for (auto it = subscribers.begin(); it != subscribers.end(); ++it)
    {
        if (next.Sender() == (*it)->Tag())
        {
            requester = *it;
            continue;
        }
        IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::STATE_EVENT,
            "NotifyStateChanged to {}", (*it)->Tag());
        (*it)->StateChanged(prev, next);
        IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::STATE_EVENT,
            "NotifyStateChanged to {} done", (*it)->Tag());
    }

    if (requester != nullptr)
    {
        IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::STATE_EVENT,
            "NotifyStateChanged to (requester) {}", requester->Tag());
        requester->StateChanged(prev, next);
        IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::STATE_EVENT,
            "NotifyStateChanged to (requester) {} done", requester->Tag());
    }
}

} // namespace ibofos

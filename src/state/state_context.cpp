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

#include "state_context.h"

#include "src/helper/time_helper.h"
#include "src/logger/logger.h"

namespace ibofos
{
StateContext::StateContext(string _sender)
{
    StateContext(_sender, Situation::DEFAULT);
}

StateContext::StateContext(string _sender, Situation _s)
{
    sender = _sender;
    situation = _s;
    clock_gettime(CLOCK_REALTIME, &ts);
    _MakeUuid();
}

void
StateContext::_MakeUuid(void)
{
    uuid = sender + "-" + StatePolicySingleton::Instance()->ToString(situation) + "-" + to_string(ts.tv_nsec);
}

State
StateContext::GetState(void)
{
    return StatePolicySingleton::Instance()->GetState(situation);
}

Situation
StateContext::GetSituation(void)
{
    return situation;
}

string
StateContext::GetStateStr(void)
{
    return StatePolicySingleton::Instance()->ToString(GetState());
}

string
StateContext::GetSituationStr(void)
{
    return StatePolicySingleton::Instance()->ToString(situation);
}

string
StateContext::GetUuid(void)
{
    return uuid;
}

bool
StateContext::IsOnline(void)
{
    return GetState() >= State::NORMAL;
}

int
StateContext::GetPriority(void) const
{
    return StatePolicySingleton::Instance()->GetPriority(situation);
}

string
StateContext::Sender(void)
{
    return sender;
}

bool
StateContext::operator==(const StateContext& ctx) const
{
    return (ctx.uuid == uuid);
}

bool
StateContext::operator!=(const StateContext& ctx) const
{
    return !operator==(ctx);
}

bool
StateContext::operator<=(const StateContext& ctx) const
{
    if (GetPriority() == ctx.GetPriority())
    {
        return issued_time > ctx.issued_time;
    }

    return GetPriority() <= ctx.GetPriority();
}

bool
StateContext::operator>=(const StateContext& ctx) const
{
    if (GetPriority() == ctx.GetPriority())
    {
        return issued_time < ctx.issued_time;
    }

    return GetPriority() >= ctx.GetPriority();
}

bool
StateContext::operator<(const StateContext& ctx) const
{
    return !operator<=(ctx);
}

bool
StateContext::operator>(const StateContext& ctx) const
{
    return !operator>=(ctx);
}

} // namespace ibofos

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

#include "array_state.h"

namespace ibofos
{
const char* ARRAY_STATE_STR[(int)ArrayStateType::TYPE_COUNT] = {
    "NOT_EXIST",
    "EXIST_NORMAL",
    "EXIST_DEGRADED",
    "EXIST_MISSING",
    "EXIST_BROKEN",
    "NORMAL",
    "DEGRADED",
    "REBUILD",
    "BROKEN"};

ArrayState::ArrayState(void)
: StateEvent("array"),
  sysStateMgr(StateManagerSingleton::Instance())
{
    sysStateMgr->Subscribe(this);
}

ArrayState::~ArrayState(void)
{
    sysStateMgr->Dispose(this);
}

std::string
ArrayState::GetCurrentStateStr(void)
{
    return ARRAY_STATE_STR[0];
}

void
ArrayState::SetLoad(uint32_t missingCnt, uint32_t brokenCnt)
{
}

void
ArrayState::SetCreate(void)
{
}

int
ArrayState::CanAddSpare(void)
{
    int eventId = 0;
    return eventId;
}

int
ArrayState::CanRemoveSpare(void)
{
    int eventId = 0;
    return eventId;
}

int
ArrayState::IsMountable(void)
{
    int eventId = 0;
    return eventId;
}

int
ArrayState::IsLoadable(void)
{
    int eventId = 0;
    return eventId;
}

int
ArrayState::IsCreatable(void)
{
    int eventId = 0;
    return eventId;
}

int
ArrayState::IsUnmountable(void)
{
    int eventId = 0;
    return eventId;
}

int
ArrayState::IsDeletable(void)
{
    int eventId = 0;
    return eventId;
}

bool
ArrayState::Exists(void)
{
    return true;
}

bool
ArrayState::IsMounted(void)
{
    return true;
}

int
ArrayState::SetMount(void)
{
    return 0;
}

int
ArrayState::SetUnmount(void)
{
    return 0;
}

void
ArrayState::DataRemoved(bool isRebuildingDevice)
{
}

void
ArrayState::SetDelete(void)
{
}

bool
ArrayState::IsRebuildable(void)
{
    return true;
}

bool
ArrayState::IsBroken(void)
{
    return false;
}

bool
ArrayState::SetRebuild(void)
{
    return true;
}

bool
ArrayState::IsRecoverable(void)
{
    return true;
}

void
ArrayState::SetRebuildDone(bool isSuccess)
{
}

void
ArrayState::_SetState(ArrayStateType newState)
{
}

void
ArrayState::_WaitSysStateFor(StateContext& goal, uint32_t sec)
{
}

void
ArrayState::StateChanged(StateContext prev, StateContext next)
{
}

} // namespace ibofos

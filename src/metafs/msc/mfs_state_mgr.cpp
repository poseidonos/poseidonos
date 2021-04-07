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

#include "mfs_state_mgr.h"

#include "mfs_log.h"
#include "mfs_ret_code.h"
#include "src/logger/logger.h"

extern const MetaFsSystemStateTransitionEntry NORMAL_STATE_MACHINE_TABLE[(uint32_t)MetaFsSystemState::Max];

MetaFsStateMgrClass::MetaFsStateMgrClass(void)
: currState(MetaFsSystemState::PowerOn),
  nextState(MetaFsSystemState::PowerOn),
  expState(MetaFsSystemState::PowerOn)
{
}

MetaFsStateMgrClass::~MetaFsStateMgrClass(void)
{
}

void
MetaFsStateMgrClass::RequestSystemStateChange(MetaFsSystemState next, MetaFsSystemState exp)
{
    nextState = next;
    expState = exp;
}

MetaFsSystemState
MetaFsStateMgrClass::GetCurrSystemState(void)
{
    return currState;
}

MetaFsSystemState
MetaFsStateMgrClass::GetNextSystemState(void)
{
    return nextState;
}

IBOF_EVENT_ID
MetaFsStateMgrClass::_ProcessNextSystemState(void)
{
    MetaFsSystemState state;
    IBOF_EVENT_ID sc;
    state = GetNextSystemState();
    sc = (&procDispatcher->*(procDispatcher.DispatchProcedure(state)))();

    if (sc == IBOF_EVENT_ID::SUCCESS)
    {
        currState = state;
    }
    return sc;
}

IBOF_EVENT_ID
MetaFsStateMgrClass::ExecuteStateTransition(void)
{
    IBOF_EVENT_ID sc = IBOF_EVENT_ID::SUCCESS;

    while (!_CheckExpStateReached())
    {
        sc = _ProcessNextSystemState();
        if (sc != IBOF_EVENT_ID::SUCCESS) // better to handle error case inside of processing routine depending on the failure reason
        {
            IBOF_TRACE_ERROR((int)sc,
                "Error occurred. Cannot proceed to transit to expected. Stuck in the state={}, nextState={}",
                (int)currState,
                (int)nextState);
            return sc;
        }
        _SetNextSystemState();

        MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
            "System state changed, currState={}, nextState={}, expState={}",
            (int)currState, (int)nextState, (int)expState);
    }
    return sc;
}

void
MetaFsStateMgrClass::_SetNextSystemState(void)
{
    nextState = NORMAL_STATE_MACHINE_TABLE[(uint32_t)currState].nextState;
}

bool
MetaFsStateMgrClass::_CheckExpStateReached(void)
{
    return (currState == expState) ? true : false;
}

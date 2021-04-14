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

#include "metafs_log.h"
#include "metafs_return_code.h"
#include "src/logger/logger.h"

namespace pos
{
extern const MetaFsSystemStateTransitionEntry NORMAL_STATE_MACHINE_TABLE[(uint32_t)MetaFsSystemState::Max];

MetaFsStateManager::MetaFsStateManager(void)
: currState(MetaFsSystemState::PowerOn),
  nextState(MetaFsSystemState::PowerOn),
  expState(MetaFsSystemState::PowerOn)
{
}

MetaFsStateManager::~MetaFsStateManager(void)
{
}

void
MetaFsStateManager::RequestSystemStateChange(MetaFsSystemState next, MetaFsSystemState exp)
{
    nextState = next;
    expState = exp;
}

MetaFsSystemState
MetaFsStateManager::GetCurrSystemState(void)
{
    return currState;
}

MetaFsSystemState
MetaFsStateManager::GetNextSystemState(void)
{
    return nextState;
}

POS_EVENT_ID
MetaFsStateManager::_ProcessNextSystemState(std::string& arrayName)
{
    MetaFsSystemState state;
    POS_EVENT_ID sc;
    state = GetNextSystemState();
    sc = (&procDispatcher->*(procDispatcher.DispatchProcedure(state)))(arrayName);

    if (sc == POS_EVENT_ID::SUCCESS)
    {
        currState = state;
    }
    return sc;
}

POS_EVENT_ID
MetaFsStateManager::ExecuteStateTransition(std::string& arrayName)
{
    POS_EVENT_ID sc = POS_EVENT_ID::SUCCESS;

    while (!_CheckExpStateReached())
    {
        sc = _ProcessNextSystemState(arrayName);
        if (sc != POS_EVENT_ID::SUCCESS) // better to handle error case inside of processing routine depending on the failure reason
        {
            POS_TRACE_ERROR((int)sc,
                "Error occurred. Cannot proceed to transit to expected. Stuck in the state={}, nextState={}",
                (int)currState,
                (int)nextState);
            return sc;
        }
        _SetNextSystemState();

        MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "System state changed, currState={}, nextState={}, expState={}",
            (int)currState, (int)nextState, (int)expState);
    }
    return sc;
}

void
MetaFsStateManager::_SetNextSystemState(void)
{
    nextState = NORMAL_STATE_MACHINE_TABLE[(uint32_t)currState].nextState;
}

bool
MetaFsStateManager::_CheckExpStateReached(void)
{
    return (currState == expState) ? true : false;
}
} // namespace pos

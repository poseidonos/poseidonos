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

#include "state_manager.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
StateManager::StateManager(void)
{
}

StateManager::~StateManager(void)
{
}

IStateControl* StateManager::CreateStateControl(string array)
{
    unique_lock<mutex> lock(stateMapMtx);
    StateControl* state = _Find(array);
    if (state == nullptr)
    {
        state = new StateControl(array);
        stateMap.emplace(array, state);
        POS_TRACE_INFO(EID(STATE_CONTROL_ADDED),
            "statecontrol of array:{} is added", array);
    }
    else
    {
        POS_TRACE_INFO(EID(STATE_CONTROL_DEBUG),
            "statecontrol of array:{} exists already. skipping the creation of statecontrol.", array);
    }
    return state;
}

IStateControl* StateManager::GetStateControl(string array)
{
    return _Find(array);
}

void StateManager::RemoveStateControl(string array)
{
    unique_lock<mutex> lock(stateMapMtx);
    if (array == "" && stateMap.size() == 1)
    {
        delete stateMap.begin()->second;
        stateMap.erase(stateMap.begin());
        POS_TRACE_INFO(EID(STATE_CONTROL_REMOVED),
                "statecontrol of array:'{}' is removed - empty array name", array);
    }
    else
    {
        auto it = stateMap.find(array);
        if (it != stateMap.end())
        {
            delete it->second;
            stateMap.erase(array);
            POS_TRACE_INFO(EID(STATE_CONTROL_REMOVED),
                "statecontrol of array:{} is removed", array);
        }
        else
        {
            POS_TRACE_WARN(EID(STATE_CONTROL_DEBUG),
                "couldn't remove statecontrol of array - {} : not existent", array);
        }
    }
}

StateControl* StateManager::_Find(string array)
{
    if (array == "" && stateMap.size() == 1)
    {
        return stateMap.begin()->second;
    }
    auto it = stateMap.find(array);
    if (it == stateMap.end())
    {
        POS_TRACE_INFO(EID(STATE_CONTROL_DEBUG),
        "statecontrol of array:{} is nullptr", array);
        return nullptr;
    }
    return it->second;
}

void
StateManager::SetStateMap(const map<string, StateControl*>& stateMap)
{
    this->stateMap = stateMap;  // copy to the internal member
}

const map<string, StateControl*>&
StateManager::GetStateMap(void)
{
    return stateMap;
}

} // namespace pos

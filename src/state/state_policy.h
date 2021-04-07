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

#pragma once

#include <time.h>

#include <map>
#include <string>

#include "src/lib/singleton.h"
using namespace std;

namespace ibofos
{
enum class State
{
    OFFLINE,
    DIAGNOSIS,
    NORMAL,
    BUSY,
    PAUSE,
    STOP,
};

enum class Situation
{
    DEFAULT,
    NORMAL,
    TRY_MOUNT,
    DEGRADED,
    TRY_UNMOUNT,
    JOURNAL_RECOVERY,
    REBUILDING,
    FAULT,
};

class StatePolicy
{
public:
    State
    GetState(Situation s)
    {
        return stateMap[s];
    }
    int
    GetPriority(Situation s)
    {
        return priorityLevel[s];
    }
    string
    ToString(State s)
    {
        return StateStr[static_cast<int>(s)];
    }
    string
    ToString(Situation s)
    {
        return SituationStr[static_cast<int>(s)];
    }

private:
    string StateStr[6] = {"OFFLINE", "DIAGNOSIS", "NORMAL",
        "BUSY", "PAUSE", "STOP"};

    string SituationStr[9] = {"DEFAULT", "NORMAL", "TRY_MOUNT", "DEGRADED",
        "TRY_UNMOUNT", "JOURNAL_RECOVERY", "REBUILDING", "FAULT"};

    map<Situation, State> stateMap = {
        {Situation::DEFAULT, State::OFFLINE},
        {Situation::NORMAL, State::NORMAL},
        {Situation::TRY_MOUNT, State::DIAGNOSIS},
        {Situation::DEGRADED, State::BUSY},
        {Situation::TRY_UNMOUNT, State::PAUSE},
        {Situation::JOURNAL_RECOVERY, State::PAUSE},
        {Situation::REBUILDING, State::BUSY},
        {Situation::FAULT, State::STOP},
    };

    map<Situation, int> priorityLevel = {
        {Situation::DEFAULT, 0},
        {Situation::NORMAL, 1},
        {Situation::TRY_MOUNT, 1},
        {Situation::DEGRADED, 10},
        {Situation::TRY_UNMOUNT, 20},
        {Situation::REBUILDING, 20},
        {Situation::JOURNAL_RECOVERY, 30},
        {Situation::FAULT, 99},
    };
};

using StatePolicySingleton = Singleton<StatePolicy>;
} // namespace ibofos

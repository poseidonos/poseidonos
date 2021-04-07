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

#include <string>

#include "src/include/ibof_event_id.h"
#include "src/logger/logger.h"
#include "src/state/state_manager.h"

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
    return ARRAY_STATE_STR[(int)state];
}

void
ArrayState::SetLoad(uint32_t missingCnt, uint32_t brokenCnt)
{
    ArrayStateType newState;
    uint32_t abnormalCnt = missingCnt + brokenCnt;

    switch (abnormalCnt)
    {
        case 0:
        {
            newState = ArrayStateType::EXIST_NORMAL;
            break;
        }
        case 1:
        {
            newState = ArrayStateType::EXIST_DEGRADED;
            if (missingCnt != 0)
            {
                needForceMount = true;
            }
            break;
        }
        default:
        {
            if (brokenCnt >= 2)
            {
                newState = ArrayStateType::EXIST_BROKEN;
            }
            else
            {
                newState = ArrayStateType::EXIST_MISSING;
            }
        }
    }
    _SetState(newState);
}

void
ArrayState::SetCreate(void)
{
    _SetState(ArrayStateType::EXIST_NORMAL);
}

int
ArrayState::CanAddSpare(void)
{
    int eventId = 0;
    if (!IsMounted())
    {
        eventId = (int)IBOF_EVENT_ID::ARRAY_NEED_MOUNT;
        IBOF_TRACE_WARN(eventId, "Failed to add spare. Spare cannot be added without Array mounted");
    }
    else if (IsBroken())
    {
        eventId = (int)IBOF_EVENT_ID::ARRAY_BROKEN_ERROR;
        IBOF_TRACE_WARN(eventId, "Failed to add spare. Array is broken");
    }
    return eventId;
}

int
ArrayState::CanRemoveSpare(void)
{
    int eventId = 0;
    if (!IsMounted())
    {
        eventId = (int)IBOF_EVENT_ID::ARRAY_NEED_MOUNT;
        IBOF_TRACE_WARN(eventId, "Failed to remove spare. Spare cannot be removed without Array mounted");
    }
    return eventId;
}

int
ArrayState::IsMountable(void)
{
    int eventId = 0;

    switch (state)
    {
        case ArrayStateType::NOT_EXIST:
        {
            eventId = (int)IBOF_EVENT_ID::ARRAY_STATE_NOT_EXIST;
            IBOF_TRACE_ERROR(eventId, "Failed to mount array. Array is not existed");
            break;
        }
        case ArrayStateType::EXIST_DEGRADED:
        {
            int degradedEvent = (int)IBOF_EVENT_ID::ARRAY_STATE_EXIST_DEGRADED;
            if (needForceMount)
            {
                IBOF_TRACE_WARN(degradedEvent, "Array force-mounted with degraded state");
            }
            else
            {
                IBOF_TRACE_WARN(degradedEvent, "Array mounted with degraded state");
            }
            break;
        }
        case ArrayStateType::EXIST_MISSING:
        {
            eventId = (int)IBOF_EVENT_ID::ARRAY_STATE_EXIST_MISSING;
            IBOF_TRACE_ERROR(eventId, "Failed to mount array. Some device is missing");
            break;
        }
        case ArrayStateType::EXIST_BROKEN:
        {
            eventId = (int)IBOF_EVENT_ID::ARRAY_STATE_EXIST_BROKEN;
            IBOF_TRACE_ERROR(eventId, "Failed to mount array. Array is broken");
            break;
        }
        default:
        {
        }
    }
    return eventId;
}

int
ArrayState::IsLoadable(void)
{
    int eventId = 0;
    if (IsMounted())
    {
        eventId = (int)IBOF_EVENT_ID::ARRAY_STATE_ONLINE;
        IBOF_TRACE_ERROR(eventId, "Failed to load array, already mounted");
    }
    else if (state == ArrayStateType::EXIST_BROKEN)
    {
        eventId = (int)IBOF_EVENT_ID::ARRAY_BROKEN_ERROR;
        IBOF_TRACE_ERROR(eventId, "Failed to load array, broken error");
    }

    return eventId;
}

int
ArrayState::IsCreatable(void)
{
    int eventId = 0;
    if (IsMounted())
    {
        eventId = (int)IBOF_EVENT_ID::ARRAY_STATE_ONLINE;
        IBOF_TRACE_ERROR(eventId, "Failed to load array, already mounted");
    }
    else if (Exists())
    {
        eventId = (int)IBOF_EVENT_ID::ARRAY_STATE_EXIST;
        IBOF_TRACE_ERROR(eventId, "Failed to create array, already existed");
    }

    return eventId;
}

int
ArrayState::IsUnmountable(void)
{
    int eventId = 0;
    if (IsMounted() == false)
    {
        eventId = (int)IBOF_EVENT_ID::ARRAY_STATE_OFFLINE;
        IBOF_TRACE_ERROR(eventId, "Failed to unmount array, not mounted");
    }
    else if (state == ArrayStateType::REBUILD)
    {
        eventId = (int)IBOF_EVENT_ID::ARRAY_STATE_REBUILDING;
        IBOF_TRACE_ERROR(eventId, "Failed to unmount array, array is being rebuilt");
    }

    return eventId;
}

int
ArrayState::IsDeletable(void)
{
    int eventId = 0;
    if (IsMounted())
    {
        eventId = (int)IBOF_EVENT_ID::ARRAY_STATE_ONLINE;
        IBOF_TRACE_ERROR(eventId, "Failed to delete array, already mounted");
    }
    else if (Exists() == false)
    {
        eventId = (int)IBOF_EVENT_ID::ARRAY_STATE_NOT_EXIST;
        IBOF_TRACE_ERROR(eventId, "Failed to delete array, already mounted");
    }
    return eventId;
}

bool
ArrayState::Exists(void)
{
    bool isExist = state > ArrayStateType::NOT_EXIST;
    return isExist;
}

bool
ArrayState::IsMounted(void)
{
    bool isMounted = state >= ArrayStateType::NORMAL;
    return isMounted;
}

int
ArrayState::SetMount(void)
{
    switch (state)
    {
        case ArrayStateType::EXIST_NORMAL:
        {
            _SetState(ArrayStateType::NORMAL);
            break;
        }
        case ArrayStateType::EXIST_DEGRADED:
        {
            _SetState(ArrayStateType::DEGRADED);
            break;
        }
        default:
        {
            IBOF_EVENT_ID id = IBOF_EVENT_ID::ARRAY_STATE_CHANGE_ERROR;
            IBOF_TRACE_WARN(id,
                "Failed to change array state, curret state is {}",
                GetCurrentStateStr());
            return (int)id;
        }
    }
    return 0;
}

int
ArrayState::SetUnmount(void)
{
    if (sysStateMgr->Exist(degradedState))
    {
        sysStateMgr->Remove(degradedState);
    }
    if (sysStateMgr->Exist(stopState))
    {
        sysStateMgr->Remove(stopState);
    }

    switch (state)
    {
        case ArrayStateType::NORMAL:
        {
            _SetState(ArrayStateType::EXIST_NORMAL);
            break;
        }
        case ArrayStateType::DEGRADED:
        {
            _SetState(ArrayStateType::EXIST_DEGRADED);
            break;
        }
        case ArrayStateType::BROKEN:
        {
            _SetState(ArrayStateType::EXIST_BROKEN);
            break;
        }
        default:
        {
            IBOF_EVENT_ID id = IBOF_EVENT_ID::ARRAY_STATE_CHANGE_ERROR;
            IBOF_TRACE_WARN(id,
                "Failed to change array state, curret state is {}",
                GetCurrentStateStr());
            return (int)id;
        }
    }
    return 0;
}

void
ArrayState::DataRemoved(bool isRebuildingDevice)
{
    switch (state)
    {
        case ArrayStateType::EXIST_NORMAL:
        {
            _SetState(ArrayStateType::EXIST_DEGRADED);
            break;
        }
        case ArrayStateType::EXIST_DEGRADED:
        {
            _SetState(ArrayStateType::EXIST_MISSING);
            break;
        }
        case ArrayStateType::NORMAL:
        {
            _SetState(ArrayStateType::DEGRADED);
            break;
        }
        case ArrayStateType::REBUILD:
        {
            if (isRebuildingDevice)
            {
                sysStateMgr->Remove(rebuildingState);
                _SetState(ArrayStateType::DEGRADED);
            }
            else
            {
                _SetState(ArrayStateType::BROKEN);
            }
            break;
        }
        case ArrayStateType::DEGRADED:
        {
            _SetState(ArrayStateType::BROKEN);
            break;
        }
        case ArrayStateType::NOT_EXIST:
        {
            assert(0);
        }
        default:
        {
        }
    }
}

void
ArrayState::SetDelete(void)
{
    _SetState(ArrayStateType::NOT_EXIST);
}

bool
ArrayState::IsRebuildable(void)
{
    if (state != ArrayStateType::DEGRADED ||
        currState.GetState() < State::NORMAL)
    {
        return false;
    }
    return true;
}

bool
ArrayState::IsBroken(void)
{
    if (state == ArrayStateType::BROKEN)
    {
        return true;
    }
    return false;
}

bool
ArrayState::SetRebuild(void)
{
    if (IsRebuildable() == false)
    {
        return false;
    }
    _SetState(ArrayStateType::REBUILD);

    uint32_t timeout = 2;
    _WaitSysStateFor(rebuildingState, timeout);
    if (currState != rebuildingState)
    {
        sysStateMgr->Remove(rebuildingState);
        _SetState(ArrayStateType::DEGRADED);
        return false;
    }
    return true;
}

void
ArrayState::SetDegraded(void)
{
    _SetState(ArrayStateType::DEGRADED);
}

bool
ArrayState::IsRecoverable(void)
{
    switch (state)
    {
        case ArrayStateType::DEGRADED:
        {
            return true;
        }
        // TODO if ibofos is not online, it will cause the error
        // case ArrayStateType::BROKEN:
        // {
        //     return false;
        // }
        default:
        {
            return false;
        }
    }
}

void
ArrayState::SetRebuildDone(bool isSuccess)
{
    sysStateMgr->Remove(rebuildingState);
    if (isSuccess)
    {
        if (state == ArrayStateType::REBUILD)
        {
            sysStateMgr->Remove(degradedState);
            _SetState(ArrayStateType::NORMAL);
        }
        else if (state != ArrayStateType::BROKEN)
        {
            IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::ARRAY_STATE_CHANGE_ERROR,
                "Unknown State Transition " + GetCurrentStateStr());
        }
        else
        {
            IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::ARRAY_STATE_EXIST_BROKEN,
                "Rebuild done on exist broken state");
        }
    }
}

void
ArrayState::_SetState(ArrayStateType newState)
{
    if (state != newState)
    {
        if (newState == ArrayStateType::DEGRADED)
        {
            sysStateMgr->Invoke(degradedState);
        }
        else if (newState == ArrayStateType::REBUILD)
        {
            sysStateMgr->Invoke(rebuildingState);
        }
        else if (newState == ArrayStateType::BROKEN)
        {
            sysStateMgr->Invoke(stopState);
        }

        IBOF_TRACE_INFO((int)IBOF_EVENT_ID::ARRAY_STATE_CHANGED,
            "Array state changed to {}",
            ARRAY_STATE_STR[(int)newState]);

        state = newState;
    }
}

void
ArrayState::_WaitSysStateFor(StateContext& goal, uint32_t sec)
{
    std::unique_lock<std::mutex> lock(mtx);
    while (currState != goal)
    {
        if (cv.wait_for(lock, chrono::seconds(sec)) == cv_status::timeout)
        {
            break;
        }
    }
}

void
ArrayState::StateChanged(StateContext prev, StateContext next)
{
    unique_lock<mutex> lock(mtx);
    currState = next;
    cv.notify_all();
}

} // namespace ibofos

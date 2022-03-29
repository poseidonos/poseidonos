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

#include "array_state.h"

#include <string>

#include "src/include/raid_state.h"
#include "src/include/pos_event_id.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"
#include "src/telemetry/telemetry_id.h"
#include "src/logger/logger.h"

using namespace pos;

ArrayState::ArrayState(IStateControl* iState, TelemetryClient* telemetryClient)
: iStateControl(iState),
  telemetryClient(telemetryClient)
{
    string sender = typeid(*this).name();
    degradedState = new StateContext(sender, SituationEnum::DEGRADED);
    rebuildingState = new StateContext(sender, SituationEnum::REBUILDING);
    stopState = new StateContext(sender, SituationEnum::FAULT);
    iStateControl->Subscribe(this, sender);
}

ArrayState::~ArrayState(void)
{
    iStateControl->Unsubscribe(this);
    delete stopState;
    delete rebuildingState;
    delete degradedState;
    if (publisher != nullptr)
    {
        delete publisher;
    }
}

ArrayStateType
ArrayState::GetState(void)
{
    return state;
}

StateContext*
ArrayState::GetSysState(void)
{
    return iStateControl->GetState();
}

void
ArrayState::SetState(ArrayStateEnum nextState)
{
    // Method for Unit Test Only
    state = nextState;
}

void
ArrayState::SetLoad(RaidState rs)
{
    ArrayStateType newState;

    switch (rs)
    {
        case RaidState::NORMAL:
        {
            newState = ArrayStateEnum::EXIST_NORMAL;
            break;
        }
        case RaidState::DEGRADED :
        {
            newState = ArrayStateEnum::EXIST_DEGRADED;
            break;
        }
        default:
        {
            newState = ArrayStateEnum::BROKEN;
        }
    }
    _SetState(newState);
}

void
ArrayState::SetCreate(void)
{
    _SetState(ArrayStateEnum::EXIST_NORMAL);
}

int
ArrayState::CanAddSpare(void)
{
    int eventId = 0;
    if (!IsMounted())
    {
        eventId = EID(ADD_SPARE_CAN_ONLY_BE_WHILE_ONLINE);
        POS_TRACE_WARN(eventId, "curr_state: {}", state.ToString());
    }
    return eventId;
}

int
ArrayState::CanRemoveSpare(void)
{
    int eventId = 0;
    if (!IsMounted())
    {
        eventId = EID(REMOVE_SPARE_CAN_ONLY_BE_WHILE_ONLINE);
        POS_TRACE_WARN(eventId, "curr_state: {}", state.ToString());
    }
    return eventId;
}

int
ArrayState::IsMountable(void)
{
    int ret = 0;
    switch (state)
    {
        case ArrayStateEnum::BROKEN:
        {
            ret = EID(MOUNT_ARRAY_BROKEN_ARRAY_CANNOT_BE_MOUNTED);
            POS_TRACE_ERROR(ret, "Failed to mount array. Array is broken");
            break;
        }
        case ArrayStateEnum::EXIST_NORMAL:
        case ArrayStateEnum::EXIST_DEGRADED:
        {
            ret = EID(SUCCESS);
            break;
        }
        default:
        {
            ret = EID(MOUNT_ARRAY_ALREADY_MOUNTED);
            break;
        }
    }
    return ret;
}

int
ArrayState::IsUnmountable(void)
{
    int eventId = 0;
    if (IsMounted() == false)
    {
        eventId = EID(UNMOUNT_ARRAY_ALREADY_UNMOUNTED);
        POS_TRACE_WARN(eventId, "curr_state: {}", state.ToString());
    }
    else if (state == ArrayStateEnum::REBUILD)
    {
        eventId = EID(UNMOUNT_ARRAY_REJECTED_DUE_TO_REBUILD_INPROGRESS);
        POS_TRACE_WARN(eventId, "curr_state: {}", state.ToString());
    }

    return eventId;
}

int
ArrayState::IsDeletable(void)
{
    int eventId = 0;
    if (IsMounted())
    {
        eventId = EID(DELETE_ARRAY_CAN_ONLY_BE_WHILE_OFFLINE);
        POS_TRACE_WARN(eventId, "curr_state: {}", state.ToString());
    }
    return eventId;
}

bool
ArrayState::Exists(void)
{
    bool isExist = state > ArrayStateEnum::NOT_EXIST;
    return isExist;
}

bool
ArrayState::IsMounted(void)
{
    bool isMounted = state >= ArrayStateEnum::NORMAL;
    return isMounted;
}

void
ArrayState::SetMount(void)
{
    switch (state)
    {
        case ArrayStateEnum::EXIST_NORMAL:
        {
            _SetState(ArrayStateEnum::NORMAL);
            break;
        }
        case ArrayStateEnum::EXIST_DEGRADED:
        {
            _SetState(ArrayStateEnum::DEGRADED);
            break;
        }
        default:
        {
            POS_TRACE_WARN(EID(ARRAY_EVENT_UNHANDLED_STATE_TRANSITION),
                "Failed to change array state, current state is {}",
                state.ToString());
        }
    }
}

void
ArrayState::SetUnmount(void)
{
    iStateControl->Remove(degradedState);
    checkShutdown = false;

    switch (state)
    {
        case ArrayStateEnum::NORMAL:
        {
            _SetState(ArrayStateEnum::EXIST_NORMAL);
            break;
        }
        case ArrayStateEnum::DEGRADED:
        {
            _SetState(ArrayStateEnum::EXIST_DEGRADED);
            break;
        }
        default:
        {
            POS_TRACE_WARN(EID(ARRAY_EVENT_UNHANDLED_STATE_TRANSITION),
                "Failed to change array state, curret state is {}",
                state.ToString());
        }
    }
}

void
ArrayState::RaidStateUpdated(RaidState rs)
{
    if (rs == RaidState::FAILURE)
    {
        _SetState(ArrayStateEnum::BROKEN);
        return;
    }

    bool isOffline = state < ArrayStateEnum::NORMAL;

    if (isOffline)
    {
        if (rs == RaidState::NORMAL)
        {
            _SetState(ArrayStateEnum::EXIST_NORMAL);
        }
        else if (rs == RaidState::DEGRADED)
        {
            _SetState(ArrayStateEnum::EXIST_DEGRADED);
        }
    }
    else
    {
        if (rs == RaidState::NORMAL)
        {
            _SetState(ArrayStateEnum::NORMAL);
        }
        else if (rs == RaidState::DEGRADED)
        {
            _SetState(ArrayStateEnum::DEGRADED);
        }
    }
}

void
ArrayState::SetDelete(void)
{
    _SetState(ArrayStateEnum::NOT_EXIST);
}

bool
ArrayState::IsRebuildable(void)
{
    return state == ArrayStateEnum::DEGRADED;
}

bool
ArrayState::IsBroken(void)
{
    return state == ArrayStateEnum::BROKEN;
}

int
ArrayState::WaitShutdownDone(void)
{
    int waitcount = 0;
    while (checkShutdown == true) // Broken State automatically triggers Shutdown to all array components
    {
        POS_TRACE_INFO(EID(DELETE_ARRAY_DEBUG_MSG), "Wait for shutdown done");
        usleep(100000);
        waitcount++;

        if (waitcount > 50)
        {
            int ret = EID(DELETE_ARRAY_TIMED_OUT);
            POS_TRACE_WARN(ret, "curr_state: {}", state.ToString());
            return ret;
        }
    }
    return 0;
}

bool
ArrayState::SetRebuild(void)
{
    POS_TRACE_DEBUG(EID(REBUILD_DEBUG_MSG), "SetRebuild, CurrState:{}", state.ToString());
    if (IsRebuildable() == false)
    {
        return false;
    }
    _SetState(ArrayStateEnum::REBUILD);
    bool res = _WaitState(rebuildingState);
    if (res == false)
    {
        iStateControl->Remove(rebuildingState);
        _SetState(ArrayStateEnum::DEGRADED);
        return false;
    }

    return true;
}

void
ArrayState::SetDegraded(void)
{
    _SetState(ArrayStateEnum::DEGRADED);
}

void
ArrayState::SetShutdown(void)
{
    checkShutdown = false;
}

bool
ArrayState::IsRecoverable(void)
{
    switch (state)
    {
        case ArrayStateEnum::EXIST_DEGRADED:
        case ArrayStateEnum::DEGRADED:
        {
            return true;
        }
        // TODO(srm) if poseidonos is not online, it will cause the error
        // case ArrayStateEnum::BROKEN:
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
    iStateControl->Remove(rebuildingState);
    if (state == ArrayStateEnum::REBUILD)
    {
        if (isSuccess)
        {
            iStateControl->Remove(degradedState);
            _SetState(ArrayStateEnum::NORMAL);
        }
        else
        {
            _SetState(ArrayStateEnum::DEGRADED);
        }
    }
}

void
ArrayState::StateChanged(StateContext* prev, StateContext* next)
{
    unique_lock<mutex> lock(mtx);
    cv.notify_all();
}

void
ArrayState::_SetState(ArrayStateEnum newState)
{
    POS_TRACE_DEBUG(EID(ARRAY_EVENT_STATE_CHANGED), "_SetState, CurrState:{}, NewState:{}", state.ToString(), newState);
    if (state != newState)
    {
        if (newState == ArrayStateEnum::DEGRADED)
        {
            iStateControl->Invoke(degradedState);
        }
        else if (newState == ArrayStateEnum::REBUILD)
        {
            iStateControl->Invoke(rebuildingState);
        }
        else if (newState == ArrayStateEnum::BROKEN)
        {
            if (state >= ArrayStateEnum::NORMAL)
            {
                checkShutdown = true;
            }
            iStateControl->Invoke(stopState);
        }

        state = newState;
        POS_TRACE_INFO(EID(ARRAY_EVENT_STATE_CHANGED),
            "Array state is changed to {}",
            state.ToString());
    }
    _PublishCurrentState();
}

bool
ArrayState::_WaitState(StateContext* goal)
{
    int timeout_sec = 3;
    std::unique_lock<std::mutex> lock(mtx);
    while (iStateControl->GetState() != goal)
    {
        if (cv.wait_for(lock, chrono::seconds(timeout_sec)) == cv_status::timeout)
        {
            return false;
        }
    }
    return true;
}

void
ArrayState::EnableStatePublisher(id_t arrayUniqueId)
{
    if (publisherEnabled == true)
    {
        POS_TRACE_WARN(EID(ARRAY_TELEMETRY_DEBUG_MSG),
            "Failed to enable array state publisher. Already enabled");
        return;
    }

    const string PUBLISHER_NAME = "ArrayStatePublisher";
    const string ARRAY_UID_LABEL = "array_unique_id";

    publisher = new TelemetryPublisher(PUBLISHER_NAME);
    publisher->AddDefaultLabel(ARRAY_UID_LABEL, to_string(arrayUniqueId));
    telemetryClient->RegisterPublisher(publisher);

    publisherEnabled = true;
}

void
ArrayState::_PublishCurrentState(void)
{
    if (publisherEnabled == false)
    {
        return;
    }

    POSMetricValue v;
    v.gauge = static_cast<uint64_t>(state.ToEnum());
    publisher->PublishData(TEL60001_ARRAY_STATUS, v, POSMetricTypes::MT_GAUGE);
}

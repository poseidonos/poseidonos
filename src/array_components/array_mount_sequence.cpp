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

#include "array_mount_sequence.h"

#include "src/array/interface/i_abr_control.h"
#include "src/array_components/mount_temp/mount_temp.h"
#include "src/array_models/interface/i_mount_sequence.h"
#include "src/include/pos_event_id.h"
#include "src/volume/volume_manager.h"

namespace pos
{
ArrayMountSequence::ArrayMountSequence(vector<IMountSequence*> seq,
    IAbrControl* abr, IStateControl* iState, string name)
: temp(abr, name),
  state(iState),
  arrayName(name)
{
    sequence.assign(seq.begin(), seq.end());
    string sender = typeid(*this).name();
    mountState = new StateContext(sender, SituationEnum::TRY_MOUNT);
    unmountState = new StateContext(sender, SituationEnum::TRY_UNMOUNT);
    normalState = new StateContext(sender, SituationEnum::NORMAL);
    state->Subscribe(this, sender);
}

ArrayMountSequence::~ArrayMountSequence(void)
{
    state->Unsubscribe(this);
    delete normalState;
    delete unmountState;
    delete mountState;
    sequence.clear();
}

int
ArrayMountSequence::Mount(void)
{
    auto it = sequence.begin();
    int ret = (int)POS_EVENT_ID::SUCCESS;

    StateContext* currState = state->GetState();
    if (currState->ToStateType() >= StateEnum::NORMAL)
    {
        ret = (int)POS_EVENT_ID::ARRAY_ALD_MOUNTED;
        return ret;
    }

    state->Invoke(mountState);
    bool res = _WaitState(mountState);
    if (res == false)
    {
        ret = (int)POS_EVENT_ID::ARRAY_MOUNT_PRIORITY_ERROR;
        goto error;
    }

    // mount array
    ret = (*it)->Init();
    if (ret != 0)
    {
        goto error;
    }

    // mount temp.mount1
    ret = temp.Mount1();
    if (ret != 0)
    {
        goto error;
    }

    // mount meta, gc
    it++;
    for (; it != sequence.end(); ++it)
    {
        ret = (*it)->Init();
        if (ret != (int)POS_EVENT_ID::SUCCESS)
        {
            break;
        }
    }

    if (ret != (int)POS_EVENT_ID::SUCCESS)
    {
        goto error;
    }
    state->Invoke(normalState);
    state->Remove(mountState);
    return ret;

error:
    for (; it == sequence.begin(); --it)
    {
        (*it)->Dispose();
    }
    state->Remove(mountState);
    return ret;
}

int
ArrayMountSequence::Unmount(void)
{
    StateContext* currState = state->GetState();
    if (currState->ToStateType() < StateEnum::NORMAL)
    {
        int eventId = (int)POS_EVENT_ID::ARRAY_ALD_UNMOUNTED;
        POS_TRACE_ERROR(eventId, "Failed to unmount system. Curr. state:{}",
            currState->ToStateType().ToString());
        return eventId;
    }

    state->Invoke(unmountState);
    bool res = _WaitState(unmountState);
    if (res == false)
    {
        state->Remove(unmountState);
        return (int)POS_EVENT_ID::ARRAY_UNMOUNT_PRIORITY_ERROR;
    }

    IVolumeManager* volMgr =
        VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);
    volMgr->DetachVolumes();
    for (auto it = sequence.rbegin(); it != sequence.rend(); ++it)
    {
        // do array->dispose after unmount2
        if (*it == sequence.front())
        {
            break;
        }

        (*it)->Dispose();
    }
    temp.Unmount2();
    // do array-dispose finally.
    sequence.front()->Dispose();
    state->Remove(normalState);
    state->Remove(unmountState);

    return (int)POS_EVENT_ID::SUCCESS;
}

void
ArrayMountSequence::Shutdown(void)
{
    IVolumeManager* volMgr =
        VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);
    volMgr->DetachVolumes();
    for (auto it = sequence.rbegin(); it != sequence.rend(); ++it)
    {
        (*it)->Shutdown();
    }

    temp.Shutdown();
}

void
ArrayMountSequence::StateChanged(StateContext* prev, StateContext* next)
{
    std::unique_lock<std::mutex> lock(mtx);
    cv.notify_all();

    if (next->ToStateType() == StateEnum::STOP)
    {
        Shutdown();
    }
}

bool
ArrayMountSequence::_WaitState(StateContext* goal)
{
    int timeout_sec = 3;
    std::unique_lock<std::mutex> lock(mtx);
    while (state->GetState() != goal)
    {
        if (cv.wait_for(lock, chrono::seconds(timeout_sec)) == cv_status::timeout)
        {
            return false;
        }
    }
    return true;
}
} // namespace pos

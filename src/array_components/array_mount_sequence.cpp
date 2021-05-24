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

#include "src/logger/logger.h"
#include "array_mount_sequence.h"

#include "src/array/interface/i_abr_control.h"
#include "src/array_components/mount_temp/mount_temp.h"
#include "src/array_models/interface/i_mount_sequence.h"
#include "src/include/pos_event_id.h"
#include "src/volume/volume_manager.h"

namespace pos
{
ArrayMountSequence::ArrayMountSequence(vector<IMountSequence*> seq,
    IAbrControl* abr, IStateControl* iState, string name, IVolumeManager* volMgr)
: ArrayMountSequence(seq, new MountTemp(abr, name), iState, name,
    new StateContext(typeid(*this).name(), SituationEnum::TRY_MOUNT),
    new StateContext(typeid(*this).name(), SituationEnum::TRY_UNMOUNT),
    new StateContext(typeid(*this).name(), SituationEnum::NORMAL),
    volMgr)
{
    // delegated to other constructor. The other constructor doesn't have IAbrControl in its
    // params because ArrayMountSequence uses IAbrControl just to instantiate MountTemp!

    // Please note that "VolumeServiceSingleton::Instance()->GetVolumeManager(name)" cannot be used in this context,
    // because VolumeManager may not have invoked "Init()" yet, leading to nullptr when we query against VolumeServiceSingleton.
}

ArrayMountSequence::ArrayMountSequence(vector<IMountSequence*> seq,
    MountTemp* mntTmp, IStateControl* iState, string name,
    StateContext* mountState, StateContext* unmountState, StateContext* normalState,
    IVolumeManager* volMgr)
: temp(mntTmp),
  state(iState),
  mountState(mountState),
  unmountState(unmountState),
  normalState(normalState),
  arrayName(name),
  volMgr(volMgr)
{
    sequence.assign(seq.begin(), seq.end());
    string sender = typeid(*this).name();
    state->Subscribe(this, sender);
}

ArrayMountSequence::~ArrayMountSequence(void)
{
    state->Unsubscribe(this);
    if (normalState != nullptr)
        delete normalState;
    if (unmountState != nullptr)
        delete unmountState;
    if (mountState != nullptr)
        delete mountState;
    if (temp != nullptr)
        delete temp;
    sequence.clear();
}

int
ArrayMountSequence::Mount(void)
{
    POS_TRACE_DEBUG(EID(ARRAY_MOUNTSEQ_DEBUG_MSG), "Entering ArrayMountSequence.Mount for {}", arrayName);
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
    POS_TRACE_DEBUG(EID(ARRAY_MOUNTSEQ_DEBUG_MSG), "Initializing the first mount sequence for {}", arrayName);
    ret = (*it)->Init();
    if (ret != 0)
    {
        goto error;
    }
    POS_TRACE_DEBUG(EID(ARRAY_MOUNTSEQ_DEBUG_MSG), "Initialized the first mount sequence for {}", arrayName);

    // mount temp.mount1
    POS_TRACE_DEBUG(EID(ARRAY_MOUNTSEQ_DEBUG_MSG), "Mounting MountTemp for {}", arrayName);
    ret = temp->Mount1();
    if (ret != 0)
    {
        goto error;
    }
    POS_TRACE_DEBUG(EID(ARRAY_MOUNTSEQ_DEBUG_MSG), "Mounted MountTemp for {}", arrayName);

    // mount meta, gc
    it++;
    for (; it != sequence.end(); ++it)
    {
        POS_TRACE_DEBUG(EID(ARRAY_MOUNTSEQ_DEBUG_MSG), "Initializing one of the remaining sequences for {}", arrayName);
        ret = (*it)->Init();
        if (ret != (int)POS_EVENT_ID::SUCCESS)
        {
            break;
        }
        POS_TRACE_DEBUG(EID(ARRAY_MOUNTSEQ_DEBUG_MSG), "Initialized the sequence for {}", arrayName);
    }

    if (ret != (int)POS_EVENT_ID::SUCCESS)
    {
        goto error;
    }
    state->Invoke(normalState);
    state->Remove(mountState);
    POS_TRACE_DEBUG(EID(ARRAY_MOUNTSEQ_DEBUG_MSG), "Returning from ArrayMountSequence.Mount for {}", arrayName);
    return ret;

error:
    POS_TRACE_WARN(ret, "Ran into an error while executing array mount sequence for {}", arrayName);
    while (true)
    {
        (*it)->Dispose();
        if (it == sequence.begin())
        {
            break;
        }
        --it;
    }
    state->Remove(mountState);
    return ret;
}

int
ArrayMountSequence::Unmount(void)
{
    POS_TRACE_DEBUG(EID(ARRAY_MOUNTSEQ_DEBUG_MSG), "Entering ArrayMountSequence.Unmount for {}", arrayName);
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

    POS_TRACE_DEBUG(EID(ARRAY_MOUNTSEQ_DEBUG_MSG), "Detaching volumes for {}", arrayName);
    volMgr->DetachVolumes();
    for (auto it = sequence.rbegin(); it != sequence.rend(); ++it)
    {
        // do array->dispose after unmount2
        if (*it == sequence.front())
        {
            break;
        }

        POS_TRACE_DEBUG(EID(ARRAY_MOUNTSEQ_DEBUG_MSG), "Disposing one of IMountSequence for {}", arrayName);
        (*it)->Dispose();
        POS_TRACE_DEBUG(EID(ARRAY_MOUNTSEQ_DEBUG_MSG), "Disposed the IMountSequence for {}", arrayName);
    }
    POS_TRACE_DEBUG(EID(ARRAY_MOUNTSEQ_DEBUG_MSG), "Unmounting MountTemp for {}", arrayName);
    temp->Unmount2();
    POS_TRACE_DEBUG(EID(ARRAY_MOUNTSEQ_DEBUG_MSG), "Unmounted MountTemp for {}", arrayName);
    // do array-dispose finally.
    sequence.front()->Dispose();
    state->Remove(normalState);
    state->Remove(unmountState);

    return (int)POS_EVENT_ID::SUCCESS;
}

void
ArrayMountSequence::Shutdown(void)
{
    volMgr->DetachVolumes();

    // unmount meta, gc
    auto it = sequence.rbegin();
    for (int cnt = 0; cnt < (int)sequence.size() - 1; cnt++, ++it)
    {
        (*it)->Shutdown();
    }

    // unmount metafs
    temp->Shutdown();

    // unmount array
    (*it)->Shutdown();
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

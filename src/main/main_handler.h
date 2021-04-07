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

#include <condition_variable>
#include <mutex>
#include <string>

#include "mfs.h"
#include "src/array/partition/partition.h"
#include "src/state/state_context.h"
#include "src/state/state_event.h"
#include "src/network/nvmf_volume.hpp"
#include "src/network/nvmf_volume_ibof.hpp"
#include "src/network/nvmf_target_event_subscriber.hpp"

namespace ibofos
{
class IbofosInfo
{
public:
    string state;
    string situation;
    uint32_t rebuildingProgress;
    uint64_t totalCapacity;
    uint64_t usedCapacity;
};

class MainHandler : public StateEvent
{
public:
    MainHandler(void);
    int Mount();
    int Unmount();
    int Exit();
    IbofosInfo GetInfo(void);
    void StateChanged(StateContext prev, StateContext next);

private:
    int _MountMetaFilesystem(void);
    MetaStorageMediaInfo _MakeMetaStorageMediaInfo(PartitionType partitionType);
    void _RegisterMediaInfoIfAvailable(PartitionType type,
        MetaStorageMediaInfoList& mediaList);
    void _WaitState(StateContext& goal);
    void _WaitStateFor(StateContext& goal, uint32_t sec);
    void _PrepareEventSubscription(void);
    void _ReleaseEventSubscription(void);
    void _InitNvmf(void);
    void _ResetNvmf(void);

    std::mutex mtx;
    std::condition_variable cv;
    std::string sender = "main_handler";
    StateContext mountCtx{sender};
    StateContext unmountCtx{sender};
    StateContext normalCtx{sender};
    StateContext currState{sender};
    bool ibofMounted = false;
    NvmfVolume* nvmfVolume;
    NvmfTargetEventSubscriber* nvmfTargetEventSubscriber;
};

} // namespace ibofos

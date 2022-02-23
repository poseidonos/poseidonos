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

#pragma once

#include <string>

#include "src/volume/i_volume_manager.h"

#include "mk/ibof_config.h"
#include "src/lib/singleton.h"
#include "src/array_models/interface/i_array_info.h"
#include "src/array_models/interface/i_mount_sequence.h"
#include "src/state/interface/i_state_control.h"
#include "src/state/interface/i_state_observer.h"
#include "src/qos/qos_common.h"
#include "src/state/state_manager.h"
#include "src/volume/volume_service.h"
#include "src/volume/volume_list.h"


#define VOLUME_UNIT_TEST 0
#define NO_QOS_LIMIT 0

namespace pos
{

class VolumeBase;

class VolumeManager : public IVolumeManager, public IMountSequence, public IStateObserver
{
public:
    VolumeManager(IArrayInfo* i, IStateControl* s);
    virtual ~VolumeManager(void);

    int Init(void) override;
    void Dispose(void) override;
    void Shutdown(void) override;
    void Flush(void) override;
    int Create(std::string name, uint64_t size, uint64_t maxiops, uint64_t maxbw) override;
    int Delete(std::string name) override;
    int Mount(std::string name, std::string subnqn) override;
    int Unmount(std::string name) override;
    int UpdateQoS(std::string name, uint64_t maxiops, uint64_t maxbw, uint64_t miniops, uint64_t minbw) override;
    int Rename(std::string oldname, std::string newname) override;

    void DetachVolumes(void) override;

    int VolumeName(int volId, std::string& volName) override;
    int VolumeID(std::string volName) override;
    int GetVolumeCount(void) override;
    int GetVolumeStatus(int volId) override;
    int CheckVolumeValidity(int volId) override;
    uint64_t EntireVolumeSize(void) override;
    int GetVolumeSize(int volId, uint64_t& volSize) override;
    VolumeList* GetVolumeList(void) override;
    std::string GetStatusStr(VolumeStatus status) override;

    int IncreasePendingIOCountIfNotZero(int volId, VolumeStatus mounted = VolumeStatus::Mounted, uint32_t ioCountToSubmit = 1) override;
    int DecreasePendingIOCount(int volId, VolumeStatus mounted = VolumeStatus::Mounted, uint32_t ioCountCompleted = 1) override;
    VolumeBase* GetVolume(int volId) override;
    void StateChanged(StateContext* prev, StateContext* next) override;
    std::string GetArrayName(void) override;

private:
    int _LoadVolumes(void);
    int _CheckPrerequisite(void);
    bool initialized = false;
    bool stopped = false;

    VolumeList volumes;
    const std::string VOLUME_STATUS_STR[2] = {
        "Unmounted",
        "Mounted"};

    IArrayInfo* arrayInfo;
    IStateControl* state;
};

} // namespace pos

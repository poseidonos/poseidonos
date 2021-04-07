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

#ifndef VOLUME_MANAGER_H_
#define VOLUME_MANAGER_H_

#include <string>

#include "src/lib/singleton.h"
#include "src/state/state_manager.h"
#include "volume.h"
#include "volume_list.h"
#include "volume_meta_intf.h"
#if defined QOS_ENABLED_FE
#include "src/qos/qos_common.h"
#endif

#define VOLUME_UNIT_TEST 0
#define NO_QOS_LIMIT 0

namespace ibofos
{
class VolumeManager : public StateEvent
{
public:
    VolumeManager(void)
    : StateEvent("VolumeManager")
    {
        StateManagerSingleton::Instance()->Subscribe(this);
    }

    ~VolumeManager(void) override
    {
        StateManagerSingleton::Instance()->Dispose(this);
    }
    void Initialize(void);
    void Dispose(void);
    int Create(string name, uint64_t size, string array, uint64_t maxiops, uint64_t maxbw);
    int Delete(string name, string array);
    int Mount(string name, string array, string subnqn);
    int Unmount(string name, string array);
    int UpdateQoS(string name, string array, uint64_t maxiops, uint64_t maxbw);
    int Rename(string oldname, string newname, string array);
    int Resize(string name, string array, uint64_t newsize);
#if defined QOS_ENABLED_FE
    int UpdateVolumePolicy(std::string volName, qos_vol_policy volPolicy);
    qos_vol_policy GetVolumePolicy(std::string volName);
#endif
    void DetachVolumes(void);
    int MountAll(void);
    int SaveVolumes(void);
    bool CheckVolumeIdle(int volId);
    void WaitUntilVolumeIdle(int volId);

    int VolumeName(int volId, string& volName);
    int VolumeID(string volName);
    int GetVolumeCount(void);
    int GetVolumeStatus(int volId);
    uint64_t EntireVolumeSize(void);
    int GetVolumeSize(int volId, uint64_t& volSize);
    VolumeList*
    GetVolumeList(void)
    {
        return &volumes;
    }
    string
    GetStatusStr(VolumeStatus status)
    {
        return VOLUME_STATUS_STR[status];
    }

    int IncreasePendingIOCount(int volId, uint32_t ioCountToSubmit = 1);
    int DecreasePendingIOCount(int volId, uint32_t ioCountCompleted = 1);
    VolumeBase* GetVolume(int volId);
    void StateChanged(StateContext prev, StateContext next) override;

private:
    int _LoadVolumes(string array);
    int _SetVolumeQoS(VolumeBase* vol, uint64_t maxiops, uint64_t maxbw);
    int _CheckVolumeSize(uint64_t volSize);
    bool _SubsystemExists(void);
    bool initialized = false;
    bool stopped = false;

    VolumeList volumes;
    const string VOLUME_STATUS_STR[2] = {
        "Unmounted",
        "Mounted"};
};

using VolumeManagerSingleton = Singleton<VolumeManager>;

} // namespace ibofos

#endif // VOLUME_MANAGER_H_

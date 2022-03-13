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

#include <list>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "src/qos/exit_handler.h"
#include "src/qos/qos_common.h"
#include "src/bio/volume_io.h"
#include "submission_adapter.h"
#include "submission_notifier.h"

namespace pos
{
class QosVolumeManager;
class QosContext;
class EventFrameworkApi;
class QosManager;
class QosArrayManager : public ExitQosHandler
{
public:
    QosArrayManager(uint32_t arrayIndex, QosContext* qosCtx,
        bool feQosEnabled,
        EventFrameworkApi* eventFrameworkApiArg,
        QosManager* qosManager);
    virtual ~QosArrayManager(void);
    void Initalize(void);
    int UpdateVolumePolicy(uint32_t volId, qos_vol_policy policy);
    qos_vol_policy GetVolumePolicy(uint32_t volId);
    uint32_t GetUsedStripeCnt(void);
    void IncreaseUsedStripeCnt(void);
    void DecreaseUsedStripeCnt(void);
    void HandlePosIoSubmission(IbofIoSubmissionAdapter* aioSubmission, VolumeIoSmartPtr io);
    void VolumeQosPoller(IbofIoSubmissionAdapter* aioSubmission, double offset);
    bool IsFeQosEnabled(void);
    qos_rebuild_policy GetRebuildPolicy(void);
    int UpdateRebuildPolicy(qos_rebuild_policy rebuildPolicy);
    void SetVolumeLimit(uint32_t volId, int64_t weight, bool iops);
    void GetMountedVolumes(std::list<uint32_t>& volumeList);
    int64_t GetVolumeLimit(uint32_t volId, bool iops);
    bool IsVolumePolicyUpdated(void);
    void SetGcFreeSegment(uint32_t count);
    uint32_t GetGcFreeSegment(void);
    void GetVolumePolicyMap(std::map<uint32_t, qos_vol_policy>& volumePolicyMapCopy);
    void UpdateSubsystemToVolumeMap(uint32_t nqnId, uint32_t volId);
    void DeleteVolumeFromSubsystemMap(uint32_t nqnId, uint32_t volId);
    std::vector<int> GetVolumeFromActiveSubsystem(uint32_t nqnId);
    void GetSubsystemVolumeMap(std::unordered_map<int32_t, std::vector<int>>& subsysVolMap);
    bool IsMinimumPolicyInEffect(void);
    void SetArrayName(std::string arrayName);
    void ResetVolumeThrottling(void);
    void SetMinimumVolume(uint32_t volId, uint64_t value, bool iops);
    uint64_t GetDynamicVolumeThrottling(uint32_t volId, bool iops);

private:
    void _Finalize(void);
    uint32_t arrayId;
    bool feQosEnabled;
    bool initialized;
    std::string arrayName;
    std::mutex policyUpdateLock;
    std::atomic<uint32_t> usedStripeCnt;
    std::vector<uint32_t> minGuaranteeVolume;
    std::atomic<bool> volMinPolicyInEffect;
    std::atomic<bool> minBwGuarantee;
    std::atomic<bool> volumePolicyUpdated;
    uint32_t gcFreeSegments;
    qos_vol_policy volPolicyCli[MAX_VOLUME_COUNT];
    qos_rebuild_policy rebuildPolicyCli;
    std::map<uint32_t, qos_vol_policy> volumePolicyMapCli;
    QosManager* qosManager;
    QosVolumeManager* qosVolumeManager;
};
} // namespace pos

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

#include "src/qos/qos_array_manager.h"

#include "src/master_context/config_manager.h"
#include "src/qos/qos_volume_manager.h"
#include "src/qos/qos_manager.h"

namespace pos
{
/* --------------------------------------------------------------------------*/
/**
  * @Synopsis
  *
  * @Returns
  */
/* --------------------------------------------------------------------------*/
QosArrayManager::QosArrayManager(uint32_t arrayIndex, QosContext* qosCtx,
        bool feQosEnabled, EventFrameworkApi* eventFrameworkApiArg,
        QosManager* qosManager)
    : arrayId(arrayIndex),
    feQosEnabled(feQosEnabled),
    qosManager(qosManager)
{
    initialized = false;
    volMinPolicyInEffect = false;
    gcFreeSegments = UPPER_GC_TH + 1;
    minBwGuarantee = false;
    volumePolicyUpdated = false;
    qosVolumeManager = new QosVolumeManager(qosCtx, feQosEnabled,
        arrayId, this, eventFrameworkApiArg, qosManager);

    minGuaranteedVolumeMaxCount = MIN_GUARANTEED_VOLUME_MAX_COUNT;
    int ret = 0;
    uint32_t value = 0;
    ret = ConfigManagerSingleton::Instance()->GetValue("fe_qos", "min_guaranteed_volume_max_count",
            static_cast<void*>(&value), CONFIG_TYPE_UINT32);
    if (ret == EID(SUCCESS))
    {
        minGuaranteedVolumeMaxCount = value;
    }
}
/* --------------------------------------------------------------------------*/
/**
  * @Synopsis
  *
  * @Returns
  */
/* --------------------------------------------------------------------------*/
QosArrayManager::~QosArrayManager(void)
{
    initialized = false;
    delete qosVolumeManager;
}
/* --------------------------------------------------------------------------*/
/**
  * @Synopsis
  *
  * @Returns
  */
/* --------------------------------------------------------------------------*/
qos_vol_policy
QosArrayManager::GetVolumePolicy(uint32_t volId)
{
    std::unique_lock<std::mutex> uniqueLock(policyUpdateLock);
    return volPolicyCli[volId];
}

void
QosArrayManager::GetMountedVolumes(std::list<uint32_t>& volumeList)
{
    qosVolumeManager->GetMountedVolumes(volumeList);
}

uint64_t
QosArrayManager::GetDynamicVolumeThrottling(uint32_t volId, bool iops)
{
    return qosVolumeManager->GetDynamicVolumeThrottling(volId, iops);
}

void
QosArrayManager::ResetVolumeThrottling(void)
{
    for (int volId = 0; volId < MAX_VOLUME_COUNT; volId++)
    {
        qosVolumeManager->ResetVolumeThrottling(volId, arrayId);
    }
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int
QosArrayManager::UpdateVolumePolicy(uint32_t volId, qos_vol_policy policy)
{
    bool minBwPolicy = minBwGuarantee;
    bool minPolicyInEffect = volMinPolicyInEffect;
    if (0 == policy.minBw)
    {
        policy.minBwGuarantee = false;
    }
    if (0 == policy.minIops)
    {
        policy.minIopsGuarantee = false;
    }
    qos_vol_policy cliPolicy = policy;

    bool minPolicyReceived = ((true == policy.minBwGuarantee) || (true == policy.minIopsGuarantee));
    bool existingPolicyChange = false;

    if (true == volMinPolicyInEffect)
    {
        if (true == minPolicyReceived)
        {
            if (minGuaranteeVolume.size() >= minGuaranteedVolumeMaxCount)
            {
                return QosReturnCode::EXCEED_MIN_GUARANTEED_VOLUME_MAX_CNT;
            }
            if ((minBwPolicy == true && policy.minIopsGuarantee) || (minBwPolicy == false && policy.minBwGuarantee))
            {
                return QosReturnCode::MIN_IOPS_OR_MIN_BW_ONLY_ONE;
            }

            // Check if change in min policy or new volume policy
            std::unique_lock<std::mutex> uniqueLock(policyUpdateLock);
            auto it = minGuaranteeVolume.begin();
            for (it = minGuaranteeVolume.begin(); it != minGuaranteeVolume.end(); ++it)
            {
                if (*it == volId)
                {
                    existingPolicyChange = true;
                }
            }
            if (!existingPolicyChange)
            {
                minGuaranteeVolume.push_back(volId);
            }
            minPolicyInEffect = true;
        }
        else
        {
            std::unique_lock<std::mutex> uniqueLock(policyUpdateLock);
            auto it = minGuaranteeVolume.begin();
            for (it = minGuaranteeVolume.begin(); it != minGuaranteeVolume.end(); ++it)
            {
                if (*it == volId)
                {
                    break;
                }
            }
            if (it != minGuaranteeVolume.end())
            {
                minGuaranteeVolume.erase(it);
            }
            if (minGuaranteeVolume.size() == 0)
            {
                minPolicyInEffect = false;
                minBwPolicy = false;
            }
        }
    }
    else
    {
        if (true == minPolicyReceived)
        {
            std::unique_lock<std::mutex> uniqueLock(policyUpdateLock);
            minGuaranteeVolume.push_back(volId);
            minPolicyInEffect = true;

            if (true == policy.minBwGuarantee)
            {
                minBwPolicy = true;
            }
            else
            {
                minBwPolicy = false;
            }
        }
    }
    if (0 == policy.maxBw)
    {
        policy.maxBw = DEFAULT_MAX_BW_IOPS;
    }
    else
    {
        policy.maxBw = policy.maxBw * (M_KBYTES * M_KBYTES / (PARAMETER_COLLECTION_INTERVAL));
        if (policy.maxBw == 0)
        {
            policy.maxBw = 1;
        }
    }
    qosVolumeManager->SetVolumeLimit(volId, policy.maxBw, false);

    if (0 == policy.maxIops)
    {
        policy.maxIops = DEFAULT_MAX_BW_IOPS;
    }
    else
    {
        // since value is taken in KIOPS, convert to actual number here
        policy.maxIops = policy.maxIops * KIOPS / PARAMETER_COLLECTION_INTERVAL;
        if (policy.maxIops == 0)
        {
            policy.maxIops = 1;
        }
    }
    qosVolumeManager->SetVolumeLimit(volId, policy.maxIops, true);

    if (0 == policy.minBw)
    {
        policy.minBw = DEFAULT_MIN_BW_MBPS;
    }
    policy.minBw = policy.minBw * (M_KBYTES * M_KBYTES / (PARAMETER_COLLECTION_INTERVAL));

    if (0 == policy.minIops)
    {
        policy.minIops = DEFAULT_MIN_IOPS;
    }
    else
    {
        // since value is taken in KIOPS, convert to actual number here
        policy.minIops = policy.minIops * KIOPS / PARAMETER_COLLECTION_INTERVAL;
    }
    {
        std::unique_lock<std::mutex> uniqueLock(policyUpdateLock);
        volPolicyCli[volId] = cliPolicy;
        volumePolicyMapCli[volId] = policy;
        minBwGuarantee = minBwPolicy;
        volMinPolicyInEffect = minPolicyInEffect;
        volumePolicyUpdated = true;
    }
    if (existingPolicyChange == true)
    {
        qosManager->ResetCorrection();
    }
    return QosReturnCode::SUCCESS;
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bool
QosArrayManager::IsVolumePolicyUpdated(void)
{
    return volumePolicyUpdated;
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bool
QosArrayManager::IsMinimumPolicyInEffect(void)
{
    return volMinPolicyInEffect;
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosArrayManager::GetVolumePolicyMap(std::map<uint32_t, qos_vol_policy>& volumePolicyMapCopy)
{
    std::unique_lock<std::mutex> uniqueLock(policyUpdateLock);
    volumePolicyMapCopy = volumePolicyMapCli;
    volumePolicyMapCli.clear();
    volumePolicyUpdated = false;
}
/* --------------------------------------------------------------------------*/
/**
  * @Synopsis
  *
  * @Returns
  */
/* --------------------------------------------------------------------------*/
void
QosArrayManager::HandlePosIoSubmission(IbofIoSubmissionAdapter* aioSubmission, VolumeIoSmartPtr volIo)
{
    qosVolumeManager->HandlePosIoSubmission(aioSubmission, volIo);
}

/* --------------------------------------------------------------------------*/
/**
  * @Synopsis
  *
  * @Returns
  */
/* --------------------------------------------------------------------------*/
void
QosArrayManager::IncreaseUsedStripeCnt(void)
{
    usedStripeCnt++;
}
/* --------------------------------------------------------------------------*/
/**
  * @Synopsis
  *
  * @Returns
  */
/* --------------------------------------------------------------------------*/
void
QosArrayManager::DecreaseUsedStripeCnt(void)
{
    usedStripeCnt--;
}
/* --------------------------------------------------------------------------*/
/**
  * @Synopsis
  *
  * @Returns
  */
/* --------------------------------------------------------------------------*/
uint32_t
QosArrayManager::GetUsedStripeCnt(void)
{
    return usedStripeCnt;
}
/* --------------------------------------------------------------------------*/
/**
  * @Synopsis
  *
  * @Returns
  */
/* --------------------------------------------------------------------------*/
qos_rebuild_policy
QosArrayManager::GetRebuildPolicy(void)
{
    return rebuildPolicyCli;
}
/* --------------------------------------------------------------------------*/
/**
  * @Synopsis
  *
  * @Returns
  */
/* --------------------------------------------------------------------------*/
int
QosArrayManager::UpdateRebuildPolicy(qos_rebuild_policy rebuildPolicy)
{
    rebuildPolicyCli = rebuildPolicy;
    return QosReturnCode::SUCCESS;
}
/* --------------------------------------------------------------------------*/
/**
  * @Synopsis
  *
  * @Returns
  */
/* --------------------------------------------------------------------------*/
void
QosArrayManager::SetVolumeLimit(uint32_t volId, int64_t weight, bool iops)
{
    qosVolumeManager->SetVolumeLimit(volId, weight, iops);
}
/* --------------------------------------------------------------------------*/
/**
  * @Synopsis
  *
  * @Returns
  */
/* --------------------------------------------------------------------------*/
int64_t
QosArrayManager::GetVolumeLimit(uint32_t volId, bool iops)
{
    return qosVolumeManager->GetVolumeLimit(volId, iops);
}
/* --------------------------------------------------------------------------*/
/**
  * @Synopsis
  *
  * @Returns
  */
/* --------------------------------------------------------------------------*/
void
QosArrayManager::SetGcFreeSegment(uint32_t freeSegments)
{
    gcFreeSegments = freeSegments;
}
/* --------------------------------------------------------------------------*/
/**
  * @Synopsis
  *
  * @Returns
  */
/* --------------------------------------------------------------------------*/
uint32_t
QosArrayManager::GetGcFreeSegment(void)
{
    return gcFreeSegments;
}
/* --------------------------------------------------------------------------*/
/**
  * @Synopsis
  *
  * @Returns
  */
/* --------------------------------------------------------------------------*/
std::vector<int>
QosArrayManager::GetVolumeFromActiveSubsystem(uint32_t nqnId)
{
    return qosVolumeManager->GetVolumeFromActiveSubsystem(nqnId);
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosArrayManager::UpdateSubsystemToVolumeMap(uint32_t nqnId, uint32_t volId)
{
    qosVolumeManager->UpdateSubsystemToVolumeMap(nqnId, volId);
}
/* --------------------------------------------------------------------------*/
/**
  * @Synopsis
  *
  * @Returns
  */
/* --------------------------------------------------------------------------*/
void
QosArrayManager::DeleteVolumeFromSubsystemMap(uint32_t nqnId, uint32_t volId)
{
    qosVolumeManager->DeleteVolumeFromSubsystemMap(nqnId, volId);
}
/* --------------------------------------------------------------------------*/
/**
  * @Synopsis
  *
  * @Returns
  */
/* --------------------------------------------------------------------------*/
void
QosArrayManager::VolumeQosPoller(IbofIoSubmissionAdapter* aioSubmission, double offset)
{
    qosVolumeManager->VolumeQosPoller(aioSubmission, offset);
}
/* --------------------------------------------------------------------------*/
/**
  * @Synopsis
  *
  * @Returns
  */
/* --------------------------------------------------------------------------*/
void
QosArrayManager::GetSubsystemVolumeMap(std::unordered_map<int32_t, std::vector<int>>& subsysVolMap)
{
    qosVolumeManager->GetSubsystemVolumeMap(subsysVolMap);
}
/* --------------------------------------------------------------------------*/
/**
  * @Synopsis
  *
  * @Returns
  */
/* --------------------------------------------------------------------------*/

void
QosArrayManager::SetArrayName(std::string name)
{
    arrayName = name;
    qosVolumeManager->SetArrayName(arrayName);
}

void
QosArrayManager::SetMinimumVolume(uint32_t volId, uint64_t value, bool iops)
{
    qosVolumeManager->SetMinimumVolume(volId, value, iops);
}

} // namespace pos

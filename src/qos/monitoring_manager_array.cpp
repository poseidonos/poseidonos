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

#include "src/qos/monitoring_manager_array.h"

#include "src/cpu_affinity/affinity_manager.h"
#include "src/include/pos_event_id.hpp"
#include "src/qos/monitoring_manager.h"
#include "src/qos/qos_context.h"
#include "src/qos/qos_manager.h"
#include "src/qos/resource.h"
#include "src/qos/resource_cpu.h"
#include "src/qos/user_policy_all_volumes.h"
#include "src/spdk_wrapper/connection_management.h"
#include "src/spdk_wrapper/event_framework_api.h"

#define VALID_ENTRY (1)
#define INVALID_ENTRY (0)
#define NVMF_CONNECT (0)
#define NVMF_DISCONNECT (1)

namespace pos
{
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

QosMonitoringManagerArray::QosMonitoringManagerArray(uint32_t arrayIndex, QosContext* qosCtx)
{
    arrayId = arrayIndex;
    qosContext = qosCtx;
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

QosMonitoringManagerArray::~QosMonitoringManagerArray(void)
{
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
i */
/* --------------------------------------------------------------------------*/
void
QosMonitoringManagerArray::_UpdateContextActiveVolumes(uint32_t volId)
{
    uint32_t globalVolId = arrayId * MAX_VOLUME_COUNT + volId;
    qosContext->InsertActiveVolume(globalVolId);
    _UpdateContextVolumeThrottle(volId);
    _UpdateContextVolumeParameter(volId);
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosMonitoringManagerArray::_UpdateContextActiveReactorVolumes(uint32_t reactor, uint32_t volId)
{
    uint32_t globalVolumeId = volId + arrayId * MAX_VOLUME_COUNT;
    qosContext->InsertActiveReactorVolume(reactor, globalVolumeId);
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosMonitoringManagerArray::UpdateContextUserVolumePolicy(void)
{
    QosManager* qosManager = QosManagerSingleton::Instance();
    std::map<uint32_t, qos_vol_policy> volumePolicyMap;
    bool volumePolicyUpdated = qosManager->IsVolumePolicyUpdated(arrayId);
    QosUserPolicy& userPolicy = qosContext->GetQosUserPolicy();
    AllVolumeUserPolicy& allVolumeUserPolicy = userPolicy.GetAllVolumeUserPolicy();
    allVolumeUserPolicy.SetMaxThrottlingChanged(false);
    std::pair<uint32_t, uint32_t> currentMinPolicyVolume = allVolumeUserPolicy.GetMinimumGuaranteeVolume();

    bool currentMinPolicyInEffect = allVolumeUserPolicy.IsMinPolicyInEffect();
    bool currentMinBwPolicy = allVolumeUserPolicy.IsMinBwPolicyInEffect();
    bool maxThrottlingChanged = false;

    if (false == volumePolicyUpdated)
    {
        return;
    }
    else
    {
        qosManager->GetVolumePolicyMap(arrayId, volumePolicyMap);
    }

    for (auto vol = volumePolicyMap.begin(); vol != volumePolicyMap.end(); vol++)
    {
        qos_vol_policy volumePolicy = vol->second;

        if (true == volumePolicy.maxValueChanged)
        {
            maxThrottlingChanged = true;
        }

        VolumeUserPolicy volUserPolicy;
        volUserPolicy.SetMaxBandwidth(volumePolicy.maxBw);
        volUserPolicy.SetMaxIops(volumePolicy.maxIops);
        volUserPolicy.SetMinBandwidth(volumePolicy.minBw);
        volUserPolicy.SetMinIops(volumePolicy.minIops);
        allVolumeUserPolicy.InsertVolumeUserPolicy(arrayId, vol->first, volUserPolicy);

        bool minBwPolicy = volumePolicy.minBwGuarantee;
        bool minIopsPolicy = volumePolicy.minIopsGuarantee;
        if (true == minBwPolicy || true == minIopsPolicy)
        {
            currentMinPolicyInEffect = true;
            if (true == minBwPolicy)
            {
                currentMinBwPolicy = true;
            }
            else
            {
                currentMinBwPolicy = false;
            }
            currentMinPolicyVolume.first = vol->first;
            currentMinPolicyVolume.second = arrayId;
        }
        else
        {
            if (currentMinPolicyVolume.first == vol->first && currentMinPolicyVolume.second == arrayId)
            {
                currentMinPolicyInEffect = false;
                currentMinBwPolicy = false;
                currentMinPolicyVolume.first = DEFAULT_MIN_VOL;
                currentMinPolicyVolume.second = MAX_ARRAY_COUNT + 1;
            }
        }
    }
    allVolumeUserPolicy.SetMinimumGuaranteeVolume(currentMinPolicyVolume.first, currentMinPolicyVolume.second);
    allVolumeUserPolicy.SetMinimumPolicyInEffect(currentMinPolicyInEffect);
    allVolumeUserPolicy.SetMinimumPolicyType(currentMinBwPolicy);
    if (true == maxThrottlingChanged)
    {
        allVolumeUserPolicy.SetMaxThrottlingChanged(true);
    }
    volumePolicyUpdated = false;
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

bool
QosMonitoringManagerArray::VolParamActivities(uint32_t volId, uint32_t reactor)
{
    QosManager* qosManager = QosManagerSingleton::Instance();
    volParams[volId] = qosManager->DequeueVolumeParams(reactor, volId, arrayId);
    if (volParams[volId].valid == M_VALID_ENTRY)
    {
        _UpdateContextActiveVolumes(volId);
        _UpdateContextActiveReactorVolumes(reactor, volId);
        volParams[volId].valid = M_INVALID_ENTRY;
        _UpdateVolumeReactorParameter(volId, reactor);
        return true;
    }
    return false;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosMonitoringManagerArray::UpdateVolumeParameter(uint32_t volId)
{
    QosParameters& qosParam = qosContext->GetQosParameters();
    AllVolumeParameter& allVolParam = qosParam.GetAllVolumeParameter();
    VolumeParameter& volParam = allVolParam.GetVolumeParameter(arrayId, volId);
    map<uint32_t, ReactorParameter> reactorParamMap = volParam.GetReactorParameterMap();
    for (map<uint32_t, ReactorParameter>::iterator it = reactorParamMap.begin(); it != reactorParamMap.end(); it++)
    {
        ReactorParameter& reactorParam = volParam.GetReactorParameter(it->first);
        volParam.IncreaseBandwidth(reactorParam.GetBandwidth());
        volParam.IncreaseIops(reactorParam.GetIops());
    }
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosMonitoringManagerArray::UpdateContextResourceDetails(void)
{
    QosManager* qosManager = QosManagerSingleton::Instance();
    QosResource& resourceDetails = qosContext->GetQosResource();
    ResourceNvramStripes& resourceNvramStripes = resourceDetails.GetResourceNvramStripes(arrayId);
    ResourceArray& resourceArray = resourceDetails.GetResourceArray(arrayId);
    uint32_t usedStripeCnt = qosManager->GetUsedStripeCnt(arrayId);
    resourceNvramStripes.SetNvramStripesUsedCount(usedStripeCnt);
    resourceArray.SetGcFreeSegment(qosManager->GetGcFreeSegment(arrayId));
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosMonitoringManagerArray::_UpdateContextVolumeThrottle(uint32_t volId)
{
    QosCorrection& qosCorrection = qosContext->GetQosCorrection();
    AllVolumeThrottle& allVolumeThrottle = qosCorrection.GetVolumeThrottlePolicy();

    VolumeThrottle volThottle;
    volThottle.Reset();
    allVolumeThrottle.InsertVolumeThrottle(arrayId, volId, volThottle);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosMonitoringManagerArray::_UpdateContextVolumeParameter(uint32_t volId)
{
    QosParameters& qosParameters = qosContext->GetQosParameters();
    AllVolumeParameter& allVolumeParameter = qosParameters.GetAllVolumeParameter();

    VolumeParameter volParam;
    volParam.Reset();
    allVolumeParameter.InsertVolumeParameter(arrayId, volId, volParam);
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosMonitoringManagerArray::_UpdateVolumeReactorParameter(uint32_t volId, uint32_t reactor)
{
    QosParameters& qosParam = qosContext->GetQosParameters();
    AllVolumeParameter& allVolParam = qosParam.GetAllVolumeParameter();
    {
        VolumeParameter volParam;
        bool volFound = allVolParam.VolumeExists(arrayId, volId);
        if (volFound == false)
        {
            allVolParam.InsertVolumeParameter(arrayId, volId, volParam);
        }
    }

    VolumeParameter& volParam = allVolParam.GetVolumeParameter(arrayId, volId);
    {
        ReactorParameter reactorParam;
        if (false == volParam.IsReactorExists(reactor))
        {
            volParam.InsertReactorParameter(reactor, reactorParam);
        }
    }
    ReactorParameter& reactorParam = volParam.GetReactorParameter(reactor);
    reactorParam.IncreaseBandwidth(volParams[volId].currentBW);
    reactorParam.IncreaseIops(volParams[volId].currentIOs);
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosMonitoringManagerArray::UpdateContextUserRebuildPolicy(void)
{
    QosManager* qosManager = QosManagerSingleton::Instance();
    string arrName = qosManager->GetArrayNameFromMap(arrayId);
    QosUserPolicy& userPolicy = qosContext->GetQosUserPolicy();
    RebuildUserPolicy& rebuildUserPolicy = userPolicy.GetRebuildUserPolicy();
    qos_rebuild_policy rebuildPolicy = qosManager->GetRebuildPolicy(arrName);

    if (rebuildUserPolicy.GetRebuildImpact() != rebuildPolicy.rebuildImpact)
    {
        rebuildUserPolicy.SetRebuildImpact(rebuildPolicy.rebuildImpact);
        rebuildUserPolicy.SetPolicyChange(true);
    }
}

} // namespace pos

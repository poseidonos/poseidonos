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

#include "src/qos/volume_policy.h"

namespace pos
{
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
VolumePolicy::VolumePolicy(QosContext* qosCtx)
{
    qosContext = qosCtx;
    lastAllVolumePolicy.Reset();
    lastAllVolumeParam.Reset();
    resetBwThrottling = false;
    changeCorrection = false;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
VolumePolicy::~VolumePolicy(void)
{
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
VolumePolicy::HandlePolicy(void)
{
    _HandleInternalPolicy();
    _StoreContext();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
VolumePolicy::_HandleInternalPolicy(void)
{
    bool retVal = false;
    QosCorrection& qosCorrection = qosContext->GetQosCorrection();
    uint32_t minVolId = DEFAULT_MIN_VOL;

    retVal = _MinimumPolicyDisabledChanged(minVolId);
    if (true == retVal)
    {
        return;
    }

    if (minVolId == DEFAULT_MIN_VOL)
    {
        return;
    }

    if (false == qosContext->IsCorrectionCycleOver())
    {
        qosContext->SetApplyCorrection(false);
        qosCorrection.SetCorrectionType(QosCorrection_VolumeThrottle, true);
        return;
    }
    _HandleBandwidthIopsUpdate();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bool
VolumePolicy::_MinimumPolicyDisabledChanged(uint32_t& minVolId)
{
    QosUserPolicy& currUserPolicy = qosContext->GetQosUserPolicy();
    QosCorrection& qosCorrection = qosContext->GetQosCorrection();
    AllVolumeUserPolicy& currAllVolumePolicy = currUserPolicy.GetAllVolumeUserPolicy();
    uint32_t currMinVolId = currAllVolumePolicy.GetMinimumGuaranteeVolume();
    uint32_t lastMinVolId = lastAllVolumePolicy.GetMinimumGuaranteeVolume();
    minVolId = currMinVolId;

    if ((false == currAllVolumePolicy.IsMinPolicyInEffect()) && (true == lastAllVolumePolicy.IsMinPolicyInEffect()))
    {
        // Minimum Policy Disabled
        qosCorrection.SetCorrectionType(QosCorrection_VolumeThrottle, true);
        qosContext->SetApplyCorrection(true);
        return true;
    }

    if (currMinVolId != lastMinVolId)
    {
        // Minimum Valume Changed
        qosCorrection.SetCorrectionType(QosCorrection_VolumeThrottle, true);
        qosContext->SetApplyCorrection(true);
        return true;
    }

    if (true == currAllVolumePolicy.IsMaxThrottlingChanged())
    {
        qosCorrection.SetCorrectionType(QosCorrection_VolumeThrottle, true);
        qosContext->SetApplyCorrection(true);
        return true;
    }
    if (lastVolReactorMap == qosContext->GetActiveVolumeReactors())
    {
        return false;
    }
    else
    {
        qosCorrection.SetCorrectionType(QosCorrection_VolumeThrottle, true);
        qosContext->SetApplyCorrection(true);
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
VolumePolicy::_HandleBandwidthIopsUpdate(void)
{
    QosParameters& parameters = qosContext->GetQosParameters();
    AllVolumeParameter& allVolumeParam = parameters.GetAllVolumeParameter();
    QosUserPolicy& currUserPolicy = qosContext->GetQosUserPolicy();
    AllVolumeUserPolicy& currAllVolumePolicy = currUserPolicy.GetAllVolumeUserPolicy();
    QosCorrection& qosCorrection = qosContext->GetQosCorrection();
    std::map<uint32_t, uint32_t> activeVolumeMap = qosContext->GetActiveVolumes();
    uint32_t currMinVolId = currAllVolumePolicy.GetMinimumGuaranteeVolume();
    bool minBwPolicy = currAllVolumePolicy.IsMinBwPolicyInEffect();

    VolumeUserPolicy* volPolicy = currAllVolumePolicy.GetVolumeUserPolicy(currMinVolId);
    if (volPolicy == nullptr)
    {
        return;
    }

    if (false == allVolumeParam.VolumeExists(currMinVolId))
    {
        return;
    }

    uint64_t minVolAverageIops = 0;
    uint64_t minVolAverageBw = 0;
    for (map<uint32_t, uint32_t>::iterator it = activeVolumeMap.begin(); it != activeVolumeMap.end(); it++)
    {
        if (false == allVolumeParam.VolumeExists(it->first))
        {
            return;
        }
        VolumeParameter& volParam = allVolumeParam.GetVolumeParameter(it->first);
        uint64_t averageIops = volParam.GetAvgIops();
        uint64_t averageBandwidth = volParam.GetAvgBandwidth();

        if ((0 == averageIops) || (0 == averageBandwidth))
        {
            return;
        }
        if (currMinVolId == it->first)
        {
            minVolAverageIops = averageIops;
            minVolAverageBw = averageBandwidth;
        }
    }

    uint64_t minIops = volPolicy->GetMinIops();
    uint64_t minBandwidth = volPolicy->GetMinBandwidth();

    if (true == minBwPolicy)
    {
        if (minVolAverageBw < minBandwidth)
        {
            _MarkAllVolumesThrottling(true, QosCorrectionDir_Increase);
            qosContext->SetApplyCorrection(true);
        }
        else if (minVolAverageBw > ((UPPER_THRESHOLD_PERCT_VALUE * minBandwidth) / PERCENTAGE_VALUE))
        {
            _MarkAllVolumesThrottling(true, QosCorrectionDir_Decrease);
            qosContext->SetApplyCorrection(true);
        }
        else
        {
            _MarkAllVolumesThrottling(true, QosCorrectionDir_NoChange);
            qosContext->SetApplyCorrection(false);
        }
    }
    else
    {
        if (minVolAverageIops < minIops)
        {
            _MarkAllVolumesThrottling(true, QosCorrectionDir_Increase);
            qosContext->SetApplyCorrection(true);
        }
        else if (minVolAverageIops > ((UPPER_THRESHOLD_PERCT_VALUE * minIops) / PERCENTAGE_VALUE))
        {
            _MarkAllVolumesThrottling(true, QosCorrectionDir_Decrease);
            qosContext->SetApplyCorrection(true);
        }
        else
        {
            _MarkAllVolumesThrottling(true, QosCorrectionDir_NoChange);
            qosContext->SetApplyCorrection(false);
        }
    }
    qosCorrection.SetCorrectionType(QosCorrection_VolumeThrottle, true);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
VolumePolicy::_MarkAllVolumesThrottling(bool skipMinVol, uint32_t change)
{
    QosCorrection& qosCorrection = qosContext->GetQosCorrection();
    AllVolumeThrottle& allVolumeThrottle = qosCorrection.GetVolumeThrottlePolicy();
    QosUserPolicy& currUserPolicy = qosContext->GetQosUserPolicy();
    AllVolumeUserPolicy& currAllVolumePolicy = currUserPolicy.GetAllVolumeUserPolicy();
    uint32_t currMinVolId = currAllVolumePolicy.GetMinimumGuaranteeVolume();

    std::map<uint32_t, VolumeThrottle>& volumeThrottleMap = allVolumeThrottle.GetVolumeThrottleMap();
    for (auto it = volumeThrottleMap.begin(); it != volumeThrottleMap.end(); it++)
    {
        if ((currMinVolId == it->first) && (true == skipMinVol))
        {
            continue;
        }
        VolumeThrottle& volThrottle = it->second;
        volThrottle.SetResetFlag(true);
        volThrottle.SetCorrectionType(change);
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
VolumePolicy::_StoreContext(void)
{
    QosUserPolicy& userPolicy = qosContext->GetQosUserPolicy();
    AllVolumeUserPolicy& allVolumeUserPolicy = userPolicy.GetAllVolumeUserPolicy();
    QosParameters& qosParameter = qosContext->GetQosParameters();
    AllVolumeParameter& allVolumeParameter = qosParameter.GetAllVolumeParameter();

    lastAllVolumePolicy = allVolumeUserPolicy;
    lastAllVolumeParam = allVolumeParameter;
    lastActiveVolMap = qosContext->GetActiveVolumes();
    lastVolReactorMap = qosContext->GetActiveVolumeReactors();
}
} // namespace pos

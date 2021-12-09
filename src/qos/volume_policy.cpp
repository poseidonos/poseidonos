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

#include "src/qos/volume_policy.h"
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
    std::pair<uint32_t, uint32_t> minVolId;
    minVolId.first = DEFAULT_MIN_VOL;
    minVolId.second = DEFAULT_MIN_ARRAY;

    retVal = _MinimumPolicyDisabledChanged(minVolId);
    if (true == retVal)
    {
        return;
    }

    QosUserPolicy& currUserPolicy = qosContext->GetQosUserPolicy();
    AllVolumeUserPolicy& currAllVolumePolicy = currUserPolicy.GetAllVolumeUserPolicy();
    std::vector<std::pair<uint32_t, uint32_t>> currMinVolId = currAllVolumePolicy.GetMinimumGuaranteeVolume();

    if (currMinVolId.size() == 0)
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
VolumePolicy::_MinimumPolicyDisabledChanged(std::pair<uint32_t, uint32_t>& minVolId)
{
    QosUserPolicy& currUserPolicy = qosContext->GetQosUserPolicy();
    QosCorrection& qosCorrection = qosContext->GetQosCorrection();
    AllVolumeUserPolicy& currAllVolumePolicy = currUserPolicy.GetAllVolumeUserPolicy();
    std::vector<std::pair<uint32_t, uint32_t>> currMinVolId = currAllVolumePolicy.GetMinimumGuaranteeVolume();
    std::vector<std::pair<uint32_t, uint32_t>> lastMinVolId = lastAllVolumePolicy.GetMinimumGuaranteeVolume();

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
        return false;
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
    std::vector<std::pair<uint32_t, uint32_t>> currMinVolId = currAllVolumePolicy.GetMinimumGuaranteeVolume();

    bool allNotMetMinPolicy = true;
    bool allExceedThreshold = true;
    bool allMetMinPolicy = true;

    for (map<uint32_t, uint32_t>::iterator it = activeVolumeMap.begin(); it != activeVolumeMap.end(); it++)
    {
        uint32_t arrayId = it->first / MAX_VOLUME_COUNT;
        uint32_t volId = it->first % MAX_VOLUME_COUNT;
        if (false == allVolumeParam.VolumeExists(arrayId, volId))
        {
            return;
        }
        VolumeParameter& volParam = allVolumeParam.GetVolumeParameter(arrayId, volId);
        uint64_t averageIops = volParam.GetAvgIops();
        uint64_t averageBandwidth = volParam.GetAvgBandwidth();

        if ((0 == averageIops) || (0 == averageBandwidth))
        {
            return;
        }

        VolumeUserPolicy* volPolicy = currAllVolumePolicy.GetVolumeUserPolicy(arrayId, volId);
        if (volPolicy == nullptr)
        {
            return;
        }


        if (volPolicy->IsMinimumVolume() == true)
        {
            if (volPolicy->IsBwPolicySet() ==true)
            {
                uint64_t minBandwidth = volPolicy->GetMinBandwidth();
                if (averageBandwidth < minBandwidth)
                {
                    allExceedThreshold = false;
                    allMetMinPolicy = false;
                }
                else if (averageBandwidth > ((UPPER_THRESHOLD_PERCT_VALUE * minBandwidth) / PERCENTAGE_VALUE))
                {
                    allNotMetMinPolicy = false;
                    allMetMinPolicy = false;
                }
                else
                {
                    allNotMetMinPolicy = false;
                    allExceedThreshold = false;
                }
            }
            else
            {
                uint64_t minIops = volPolicy->GetMinIops();
                if (averageIops < minIops)
                {
                    allExceedThreshold = false;
                    allMetMinPolicy = false;
                }
                else if (averageIops > ((UPPER_THRESHOLD_PERCT_VALUE * minIops) / PERCENTAGE_VALUE))
                {
                    allNotMetMinPolicy = false;
                    allMetMinPolicy = false;
                }
                else
                {
                    allNotMetMinPolicy = false;
                    allExceedThreshold = false;
                }
            }
        }
    }


    if (allNotMetMinPolicy == true && allExceedThreshold == true && allMetMinPolicy == true)
    {
        qosContext->SetApplyCorrection(true);
        qosCorrection.SetCorrectionType(QosCorrection_VolumeThrottle, true);
        return;
    }
    else if (allNotMetMinPolicy == true)
    {
        _MarkAllVolumesThrottling(true, QosCorrectionDir_Increase);
        qosContext->SetApplyCorrection(true);
    }
    else if (allExceedThreshold == true)
    {
        _MarkAllVolumesThrottling(true, QosCorrectionDir_Decrease);
        qosContext->SetApplyCorrection(true);
    }
    else if (allMetMinPolicy == true)
    {
        _MarkAllVolumesThrottling(true, QosCorrectionDir_NoChange);
        qosContext->SetApplyCorrection(true);
    }
    else
    {
        _MarkAllVolumesThrottling(true, QosCorrectionDir_SetMaxLimit);
        qosContext->SetApplyCorrection(true);
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
    std::vector<std::pair<uint32_t, uint32_t>> currMinVolId = currAllVolumePolicy.GetMinimumGuaranteeVolume();

    std::map<std::pair<uint32_t, uint32_t>, VolumeThrottle>& volumeThrottleMap = allVolumeThrottle.GetVolumeThrottleMap();
    for (auto it = volumeThrottleMap.begin(); it != volumeThrottleMap.end(); it++)
    {
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

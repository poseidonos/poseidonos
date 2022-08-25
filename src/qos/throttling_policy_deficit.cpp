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

#include "src/qos/throttling_policy_deficit.h"

#include <cstdint>

#include "src/logger/logger.h"
#include "src/qos/qos_common.h"
#include "src/qos/qos_manager.h"
#include "src/qos/throttle_volume.h"

const int GUARANTEE_CYCLES = 20;
const int WAIT_CYCLES = 20;
const int STEP_BW_DECREASE_VAL = 5 * (M_KBYTES * M_KBYTES / (PARAMETER_COLLECTION_INTERVAL));
const int MIN_CORRECTION = (M_KBYTES * M_KBYTES / (PARAMETER_COLLECTION_INTERVAL));

namespace pos
{
ThrottlingPolicyDeficit::ThrottlingPolicyDeficit(QosContext* qosCtx, QosManager* qosManager)
    : qosContext(qosCtx),
    qosManager(qosManager)
{
    resetFlag = true;
    totalDeficit = 0;
    beginAgain = true;
    noOfCycles = 0;
    prevDeficit = -1;
    storeCorrection = 0;
    totalCycleIter = 0;
}

bool
ThrottlingPolicyDeficit::GetCorrectionType(uint32_t volId, uint32_t arrayId)
{
    return correctionType[std::make_pair(volId, arrayId)];
}

void
ThrottlingPolicyDeficit::Reset(void)
{
    resetFlag = true;
    beginAgain = true;
    noOfCycles = 0;
    totalCycleIter = 0;
}

AllVolumeParameter&
ThrottlingPolicyDeficit::_GetAllVolumeParameters(void)
{
    QosParameters& parameters = qosContext->GetQosParameters();
    AllVolumeParameter& allVolumeParam = parameters.GetAllVolumeParameter();
    return allVolumeParam;
}

AllVolumeUserPolicy&
ThrottlingPolicyDeficit::_GetUserPolicy(void)
{
    QosUserPolicy& currUserPolicy = qosContext->GetQosUserPolicy();
    AllVolumeUserPolicy& currAllVolumePolicy = currUserPolicy.GetAllVolumeUserPolicy();
    return currAllVolumePolicy;
}

void
ThrottlingPolicyDeficit::IncrementCycleCount(void)
{
    noOfCycles++;
}

void
ThrottlingPolicyDeficit::_GetBwIopsCorrection(int64_t& bwCorrection, int64_t& iopsCorrection, uint32_t volId, uint32_t arrayId, VolumeThrottle* volThrottle)
{
    std::map<uint32_t, uint32_t> activeVolumeMap = qosContext->GetActiveVolumes();

    if (noOfCycles <= WAIT_CYCLES && beginAgain == true)
    {
        // Waiting for ios to stabalize
        bwCorrection = 0;
        if (noOfCycles == (GUARANTEE_CYCLES - 1))
        {
            if (_CheckValidInput() == false)
            {
                noOfCycles = 0;
                beginAgain = true;
                POS_EVENT_ID eventId = EID(QOS_MINIMUM_NOT_MET);
                POS_TRACE_ERROR(static_cast<int>(eventId), "Minimum Policy cannot be met, set valid values");
                return;
            }
        }
        if (noOfCycles == WAIT_CYCLES)
        {
            noOfCycles = 0;
            beginAgain = false;
            return;
        }
        return;
    }
    else if (noOfCycles <= GUARANTEE_CYCLES)
    {
        switch (volThrottle->CorrectionType())
        {
            case QosCorrectionDir_Increase:
            case QosCorrectionDir_SetMaxLimit:
                if (resetFlag == true)
                {
                    _CalculateDeficit(bwCorrection, iopsCorrection);
                    bwCorrection = (totalDeficit / GUARANTEE_CYCLES);
                }
                else
                {
                    bwCorrection = storeCorrection;
                }
                bwCorrection = bwCorrection * -1;
                iopsCorrection = iopsCorrection * -1;
                break;
            case QosCorrectionDir_Decrease:
                bwCorrection = STEP_BW_DECREASE_VAL;
                break;
            case QosCorrectionDir_NoChange:
                bwCorrection = 0;
                iopsCorrection = 0;
                break;
            default:
                break;
        }
    }
    else
    {
        totalCycleIter++;
        if (totalCycleIter == 1 && volThrottle->CorrectionType() != QosCorrectionDir_NoChange)
        {
            // Minimum not met in first GUARANTEE_CYCLES
            POS_EVENT_ID eventId = EID(QOS_MINIMUM_NOT_MET);
            POS_TRACE_ERROR(static_cast<int>(eventId), "Minimum Policy not met in Guarantee Cycles");
        }
        noOfCycles = 0;
        resetFlag = true;
        bwCorrection = 0;
        iopsCorrection = 0;
    }
}

unsigned int
ThrottlingPolicyDeficit::GetNewWeight(uint32_t volId, uint32_t arrayId, VolumeThrottle* volumeThrottle)
{
    std::map<uint32_t, map<uint32_t, uint32_t>> volReactorMap = qosContext->GetActiveVolumeReactors();
    std::pair<uint32_t, uint32_t> volArray = std::make_pair(volId, arrayId);
    correctionType[volArray] = true;
    AllVolumeUserPolicy& allVolUserPolicy = _GetUserPolicy();
    AllVolumeParameter& allVolumeParameters = _GetAllVolumeParameters();
    VolumeParameter& volumeParam = allVolumeParameters.GetVolumeParameter(arrayId, volId);
    VolumeUserPolicy* volumeUserPolicy = allVolUserPolicy.GetVolumeUserPolicy(arrayId, volId);
    if (volumeUserPolicy == nullptr)
    {
        return -1;
    }

    if (volumeParam.GetAvgBandwidth() == 0 || volumeParam.GetAvgIops() == 0)
    {
        return 0;
    }

    if (volumeUserPolicy->IsMinimumVolume() == true)
    {
        if (volumeUserPolicy->IsBwPolicySet() == true)
        {
            return  _MinimumVolumeCorrection(volumeThrottle, volumeUserPolicy);
        }
        else
        {
            correctionType[volArray] = false;
            return  _MinimumVolumeCorrection(volumeThrottle, volumeUserPolicy);
        }
    }
    uint32_t globalVolId = MAX_VOLUME_COUNT * arrayId + volId;
    uint64_t currentBwWeight = 0;
    uint64_t currentIopsWeight = 0;

    currentBwWeight += qosManager->GetVolumeLimit(volId, false, arrayId);
    currentIopsWeight += qosManager->GetVolumeLimit(volId, true, arrayId);

    int64_t bwCorrection = 0;
    int64_t iopsCorrection = 0;
    uint64_t newWeight = 0;

    _GetBwIopsCorrection(bwCorrection, iopsCorrection, volId, arrayId, volumeThrottle);

    if ((bwCorrection == 0 || iopsCorrection == 0)&& volumeThrottle->CorrectionType() == QosCorrectionDir_NoChange)
    {
        return 0;
    }

    for (map<uint32_t, uint32_t>::iterator it = volReactorMap[globalVolId].begin(); it != volReactorMap[globalVolId].end(); ++it)
    {
        VolumeParameter& volParameter = allVolumeParameters.GetVolumeParameter(arrayId, volId);
        newWeight = _InitialValueCheck(currentBwWeight, false, volParameter, *volumeUserPolicy);
        uint64_t totalBwArray = allVolumeParameters.GetTotalBw(arrayId);
        uint64_t volBw = volParameter.GetAvgBandwidth();

        uint64_t throttlingFactor = (totalBwArray - allVolumeParameters.GetMinVolBw(arrayId)) / volBw;

        if (bwCorrection < 0)
        {
            if ((bwCorrection * -1) < MIN_CORRECTION)
            {
                bwCorrection = MIN_CORRECTION;
                bwCorrection *= -1;
            }
        }
        if (bwCorrection < 0)
        {
            if (newWeight > (uint64_t)((int)(bwCorrection / (int)throttlingFactor) * -1))
            {
                newWeight = newWeight + (int)(bwCorrection / (int)throttlingFactor);
            }
            else
            {
                newWeight = -1 * MIN_CORRECTION;
            }
        }
        return newWeight;
    }
    return 0;
}

uint64_t
ThrottlingPolicyDeficit::_MinimumVolumeCorrection(VolumeThrottle* volumeThrottle, VolumeUserPolicy* volumeUserPolicy)
{
    switch (volumeThrottle->CorrectionType())
    {
        case QosCorrectionDir_Increase:
        {
            if (volumeUserPolicy->IsBwPolicySet() == true)
            {
                uint64_t userSetMaxBw = volumeUserPolicy->GetMaxBandwidth();
                return userSetMaxBw;
            }
            else
            {
                uint64_t userSetMaxIops = volumeUserPolicy->GetMaxIops();
                return userSetMaxIops;
            }
        }
        break;
        case QosCorrectionDir_Decrease:
        case QosCorrectionDir_SetMaxLimit:
        {
            if (volumeUserPolicy->IsBwPolicySet() == true)
            {
                uint64_t minBandwidth = volumeUserPolicy->GetMinBandwidth();
                uint64_t userSetMaxBw = volumeUserPolicy->GetMaxBandwidth();
                uint64_t threshold = ((UPPER_THRESHOLD_PERCT_VALUE * minBandwidth) / PERCENTAGE_VALUE);
                uint64_t val = threshold > userSetMaxBw ? userSetMaxBw : threshold;
                return val;
            }
            else
            {
                uint64_t minIops = volumeUserPolicy->GetMinIops();
                uint64_t userSetMaxIops = volumeUserPolicy->GetMaxIops();
                uint64_t threshold = ((UPPER_THRESHOLD_PERCT_VALUE * minIops) / PERCENTAGE_VALUE);
                uint64_t val = threshold > userSetMaxIops ? userSetMaxIops : threshold;
                return val;
            }
        }
        break;
        case QosCorrectionDir_NoChange:
        {
            return 0;
        }
        break;
        default:
        return 0;
    }
}

uint64_t
ThrottlingPolicyDeficit::_InitialValueCheck(uint64_t value, bool iops, VolumeParameter& volParameter, VolumeUserPolicy& volUserPolicy)
{
    uint64_t userSetMaxBw = volUserPolicy.GetMaxBandwidth();
    uint64_t userSetMaxIops = volUserPolicy.GetMaxIops();
    if (true == iops)
    {
        return ((value >= userSetMaxIops) ? volParameter.GetAvgIops() : value);
    }
    else
    {
        return ((value >= userSetMaxBw) ? volParameter.GetAvgBandwidth() : value);
    }
    return 0;
}

bool
ThrottlingPolicyDeficit::_CheckValidInput(void)
{
    bool possible = false;
    uint64_t totalUserMinReqd = 0;
    uint64_t sumOfAll = 0;
    uint64_t noOfMinVolumes = 0;
    std::map<uint32_t, uint32_t> activeVolumeMap = qosContext->GetActiveVolumes();
    AllVolumeParameter& allVolumeParam = _GetAllVolumeParameters();
    AllVolumeUserPolicy& currAllVolumePolicy = _GetUserPolicy();
    for (map<uint32_t, uint32_t>::iterator it = activeVolumeMap.begin(); it != activeVolumeMap.end(); it++)
    {
        uint32_t volumeId = it->first % MAX_VOLUME_COUNT;
        uint32_t arrayId = it->first / MAX_VOLUME_COUNT;
        VolumeUserPolicy* volumeUserPolicy = currAllVolumePolicy.GetVolumeUserPolicy(arrayId, volumeId);
        VolumeParameter& volParam = allVolumeParam.GetVolumeParameter(arrayId, volumeId);

        sumOfAll += volParam.GetAvgBandwidth();
        if (volumeUserPolicy->IsMinimumVolume() == true)
        {
            noOfMinVolumes++;
            totalUserMinReqd += volumeUserPolicy->GetMinBandwidth();
        }
    }

    if (totalUserMinReqd < (sumOfAll - (noOfMinVolumes * DEFAULT_MIN_BW_MBPS)))
    {
        possible = true;
    }
    return possible;
}

void
ThrottlingPolicyDeficit::_CalculateDeficit(int64_t& bwCorrection, int64_t& iopsCorrection)
{
    prevDeficit = totalDeficit;
    totalDeficit = 0;
    noOfCycles = 1;

    std::map<uint32_t, uint32_t> activeVolumeMap = qosContext->GetActiveVolumes();

    AllVolumeParameter& allVolumeParam = _GetAllVolumeParameters();
    AllVolumeUserPolicy& currAllVolumePolicy = _GetUserPolicy();
    for (map<uint32_t, uint32_t>::iterator it = activeVolumeMap.begin(); it != activeVolumeMap.end(); it++)
    {
        uint32_t volumeId = it->first % MAX_VOLUME_COUNT;
        uint32_t arrayId = it->first / MAX_VOLUME_COUNT;
        VolumeUserPolicy* volumeUserPolicy = currAllVolumePolicy.GetVolumeUserPolicy(arrayId, volumeId);

        if (volumeUserPolicy->IsMinimumVolume() == true)
        {
            if (volumeUserPolicy->IsBwPolicySet() == true)
            {
                VolumeParameter& volParam = allVolumeParam.GetVolumeParameter(arrayId, volumeId);
                uint64_t avgBw = volParam.GetAvgBandwidth();
                uint64_t minBandwidth = volumeUserPolicy->GetMinBandwidth();

                if (avgBw > minBandwidth)   // Coverity Fix
                {
                    continue;
                }
                totalDeficit += (minBandwidth - avgBw);
            }
            else
            {
                VolumeParameter& volParam = allVolumeParam.GetVolumeParameter(arrayId, volumeId);
                uint64_t avgBw = volParam.GetAvgBandwidth();
                uint64_t blockSize = volParam.GetBlockSize();
                uint64_t minIops = volumeUserPolicy->GetMinIops();
                uint64_t minBwEq = minIops * blockSize;

                if (minBwEq - avgBw < 0)
                {
                    continue;
                }
                totalDeficit += (minBwEq - avgBw);
            }
        }
    }
    if (totalDeficit < 0)
    {
        bwCorrection = 0;
        iopsCorrection = 0;
        return;
    }

    if (prevDeficit != 0)
    {
        // should be more than 20 %
        if (prevDeficit - totalDeficit <= (20 * prevDeficit) / 100)
        {
            bwCorrection = 0;
            iopsCorrection = 0;
            return;
        }
    }
    storeCorrection = (totalDeficit / GUARANTEE_CYCLES);
    resetFlag = false;
    beginAgain = false;
}
} // namespace pos

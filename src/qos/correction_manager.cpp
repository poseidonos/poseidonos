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

#include "src/qos/correction_manager.h"

#include "src/include/pos_event_id.hpp"
#include "src/qos/qos_context.h"
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
QosCorrectionManager::QosCorrectionManager(QosContext* qosCtx)
{
    qosContext = qosCtx;
    nextManagerType = QosInternalManager_Unknown;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosCorrectionManager::~QosCorrectionManager(void)
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
QosCorrectionManager::_HandleWrrCorrection(void)
{
    QosManager* qosManager = QosManagerSingleton::Instance();
    QosCorrection qosCorrection = qosContext->GetQosCorrection();
    QosEventWrrWeight eventWrrPolicy = qosCorrection.GetEventWrrWeightPolicy();
    for (uint32_t event = BackendEvent_Start; event < BackendEvent_Count; event++)
    {
        int32_t weight = qosManager->GetEventWeightWRR((BackendEvent)event);
        if (true == eventWrrPolicy.IsReset(event))
        {
            qosManager->SetEventWeightWRR((BackendEvent)event, M_DEFAULT_WEIGHT);
            eventWrrPolicy.SetEventWrrWeight((BackendEvent)event, M_DEFAULT_WEIGHT);
        }
        switch (eventWrrPolicy.CorrectionType(event))
        {
            case QosCorrectionDir_NoChange:
                continue;
                break;
            case QosCorrectionDir_Increase:
                weight -= 3 * M_WEIGHT_CHANGE_INDEX;
                break;
            case QosCorrectionDir_Increase2X:
                weight -= 5 * M_WEIGHT_CHANGE_INDEX;
                break;
            case QosCorrectionDir_Increase4X:
                weight -= 10 * M_WEIGHT_CHANGE_INDEX;
                break;
            case QosCorrectionDir_Decrease:
                weight += M_WEIGHT_CHANGE_INDEX;
                break;
            case QosCorrectionDir_Decrease2X:
                weight += 2 * M_WEIGHT_CHANGE_INDEX;
                break;
            case QosCorrectionDir_Decrease4X:
                weight += 4 * M_WEIGHT_CHANGE_INDEX;
                break;
            case QosCorrectionDir_PriorityHighest:
                weight = PRIO_WT_HIGHEST;
                break;
            case QosCorrectionDir_PriorityHigh:
                weight = PRIO_WT_HIGH;
                break;
            case QosCorrectionDir_PriorityMedium:
                weight = PRIO_WT_MEDIUM;
                break;
            case QosCorrectionDir_PriorityLow:
                weight = PRIO_WT_LOW;
                break;
            case QosCorrectionDir_PriorityLowest:
                weight = PRIO_WT_LOWEST;
                break;
            case QosCorrectionDir_Reset:
                weight = PRIO_CRIT_WT_1;
            default:
                break;
        }
        if (weight < M_MAX_NEGATIVE_WEIGHT)
        {
            weight = M_MAX_NEGATIVE_WEIGHT;
        }
        if (weight >= M_DEFAULT_WEIGHT)
        {
            weight = M_DEFAULT_WEIGHT;
        }
        qosManager->SetEventWeightWRR((BackendEvent)event, weight);
        eventWrrPolicy.SetEventWrrWeight((BackendEvent)event, weight);
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

uint64_t
QosCorrectionManager::_InitialValueCheck(uint64_t value, bool iops, VolumeParameter& volParameter, VolumeUserPolicy& volUserPolicy)
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

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosCorrectionManager::_HandleMaxThrottling(void)
{
    QosManager* qosManager = QosManagerSingleton::Instance();
    std::map<uint32_t, uint32_t> activeVolumeMap = qosContext->GetActiveVolumes();
    std::map<uint32_t, map<uint32_t, uint32_t>> volReactorMap = qosContext->GetActiveVolumeReactors();
    QosUserPolicy& qosUserPolicy = qosContext->GetQosUserPolicy();
    AllVolumeUserPolicy& allVolUserPolicy = qosUserPolicy.GetAllVolumeUserPolicy();
    std::pair<uint32_t, uint32_t> minVolId = allVolUserPolicy.GetMinimumGuaranteeVolume();
    QosCorrection qosCorrection = qosContext->GetQosCorrection();

    QosParameters qosParameters = qosContext->GetQosParameters();
    for (map<uint32_t, uint32_t>::iterator it = activeVolumeMap.begin(); it != activeVolumeMap.end(); it++)
    {
        uint64_t currentBwWeight = 0;
        uint64_t currentIopsWeight = 0;
        uint32_t volId = it->first;
        uint32_t totalConnection = qosContext->GetTotalConnection(volId);

        uint32_t arrayId = volId / MAX_VOLUME_COUNT;
        uint32_t volumeId = volId % MAX_VOLUME_COUNT;
        VolumeUserPolicy* volumeUserPolicy = allVolUserPolicy.GetVolumeUserPolicy(arrayId, volumeId);
        if (volumeUserPolicy == nullptr)
        {
            continue;
        }
        for (map<uint32_t, uint32_t>::iterator it = volReactorMap[volId].begin(); it != volReactorMap[volId].end(); ++it)
        {
            currentBwWeight += qosManager->GetVolumeLimit(it->first, volumeId, false, arrayId);
            currentIopsWeight += qosManager->GetVolumeLimit(it->first, volumeId, true, arrayId);
        }

        uint64_t userSetBwWeight = volumeUserPolicy->GetMaxBandwidth();
        uint64_t userSetIopsWeight = volumeUserPolicy->GetMaxIops();

        for (map<uint32_t, uint32_t>::iterator it = volReactorMap[volId].begin(); it != volReactorMap[volId].end(); ++it)
        {
            if (minVolId.first != DEFAULT_MIN_VOL)
            {
                currentBwWeight = currentBwWeight >= userSetBwWeight ? userSetBwWeight : currentBwWeight;
                currentIopsWeight = currentIopsWeight >= userSetIopsWeight ? userSetIopsWeight : currentIopsWeight;
            }
            else
            {
                currentBwWeight = userSetBwWeight;
                currentIopsWeight = userSetIopsWeight;
            }
            qosManager->SetVolumeLimit(it->first, volumeId, (currentBwWeight * (it->second)) / (totalConnection), false, arrayId);
            int64_t weight = (currentIopsWeight * it->second) / totalConnection;
            if (0 == weight)
            {
                weight = 1;
            }
            qosManager->SetVolumeLimit(it->first, volumeId, weight, true, arrayId);
        }
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
QosCorrectionManager::_HandleVolumeCorrection(void)
{
    QosUserPolicy& qosUserPolicy = qosContext->GetQosUserPolicy();
    AllVolumeUserPolicy& allVolUserPolicy = qosUserPolicy.GetAllVolumeUserPolicy();
    std::pair<uint32_t, uint32_t> minVolId = allVolUserPolicy.GetMinimumGuaranteeVolume();
    if (minVolId.first != DEFAULT_MIN_VOL)
    {
        // _HandleMinThrottling(minVolId);
        // Min Throttling not supported in multi array
    }
    _HandleMaxThrottling();
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosCorrectionManager::Execute(void)
{
    QosCorrection& qosCorrection = qosContext->GetQosCorrection();
    qos_correction_type qosCorrectionType = qosCorrection.GetCorrectionType();
    if (true == qosCorrectionType.volumeThrottle)
    {
        _HandleVolumeCorrection();
    }
    if (true == qosCorrectionType.eventWrr)
    {
        _HandleWrrCorrection();
    }
    _SetNextManagerType();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosCorrectionManager::_SetNextManagerType(void)
{
    nextManagerType = QosInternalManager_Monitor;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosInternalManagerType
QosCorrectionManager::GetNextManagerType(void)
{
    return nextManagerType;
}

} // namespace pos

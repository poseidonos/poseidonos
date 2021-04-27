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
            case QosCorrectionDir_PriorityHigh:
                weight = PRIO_WT_H2;
                break;
            case QosCorrectionDir_PriorityLow:
                weight = PRIO_WT_L2;
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
void
QosCorrectionManager::_HandleMinThrottling(uint32_t minVolId)
{
    QosManager* qosManager = QosManagerSingleton::Instance();
    QosCorrection& qosCorrection = qosContext->GetQosCorrection();
    AllVolumeThrottle& allVolumeThrottle = qosCorrection.GetVolumeThrottlePolicy();
    std::map<uint32_t, map<uint32_t, uint32_t>> volReactorMap = qosContext->GetActiveVolumeReactors();
    QosParameters qosParameters = qosContext->GetQosParameters();
    QosUserPolicy& qosUserPolicy = qosContext->GetQosUserPolicy();
    AllVolumeUserPolicy& allVolUserPolicy = qosUserPolicy.GetAllVolumeUserPolicy();
    AllVolumeParameter& allVolumeParameters = qosParameters.GetAllVolumeParameter();
    std::map<uint32_t, VolumeThrottle>& volumeThrottleMap = allVolumeThrottle.GetVolumeThrottleMap();
    bool minBwPolicy = allVolUserPolicy.IsMinBwPolicyInEffect();

    for (auto it = volumeThrottleMap.begin(); it != volumeThrottleMap.end(); it++)
    {
        VolumeThrottle& volThrottle = it->second;
        uint32_t volId = it->first;
        int64_t bwCorrection = 0;
        int64_t iopsCorrection = 0;
        uint64_t currentBwWeight = 0;
        uint64_t currentIopsWeight = 0;
        uint64_t newBwWeight = 0;
        uint64_t newIopsWeight = 0;
        uint32_t totalConnection = qosContext->GetTotalConnection(volId);
        VolumeUserPolicy& volumeUserPolicy = allVolUserPolicy.GetVolumeUserPolicy(volId);
        uint64_t userSetMaxBw = volumeUserPolicy.GetMaxBandwidth();
        uint64_t userSetMaxIops = volumeUserPolicy.GetMaxIops();

        if ((minVolId == it->first))
        {
            qosManager->SetVolumeLimit(it->first, volId, userSetMaxBw, false);
            qosManager->SetVolumeLimit(it->first, volId, userSetMaxIops, true);
            continue;
        }
        switch (volThrottle.CorrectionType())
        {
            case QosCorrectionDir_Increase:
                bwCorrection = volThrottle.GetCorrectionValue(false);
                iopsCorrection = volThrottle.GetCorrectionValue(true);
                bwCorrection = bwCorrection * -1;
                iopsCorrection = iopsCorrection * -1;
                break;
            case QosCorrectionDir_Decrease:
                bwCorrection = volThrottle.GetCorrectionValue(false);
                iopsCorrection = volThrottle.GetCorrectionValue(true);
                break;
            case QosCorrectionDir_NoChange:
                continue;
                break;
            default:
                break;
        }
        for (map<uint32_t, uint32_t>::iterator it = volReactorMap[volId].begin(); it != volReactorMap[volId].end(); ++it)
        {
            currentBwWeight += qosManager->GetVolumeLimit(it->first, volId, false);
            currentIopsWeight += qosManager->GetVolumeLimit(it->first, volId, true);
        }
        for (map<uint32_t, uint32_t>::iterator it = volReactorMap[volId].begin(); it != volReactorMap[volId].end(); ++it)
        {
            VolumeParameter& volParameter = allVolumeParameters.GetVolumeParameter(volId);
            newBwWeight = _InitialValueCheck(currentBwWeight, false, volParameter, volumeUserPolicy);
            newBwWeight = newBwWeight + bwCorrection;
            if (newBwWeight > userSetMaxBw)
            {
                newBwWeight = userSetMaxBw;
            }
            newIopsWeight = _InitialValueCheck(currentIopsWeight, true, volParameter, volumeUserPolicy);
            newIopsWeight = newIopsWeight + iopsCorrection;
            if (newIopsWeight > userSetMaxIops)
            {
                newIopsWeight = userSetMaxIops;
            }
            if (true == minBwPolicy)
            {
                _ApplyCorrection(newBwWeight, false, volId, it->first, it->second, totalConnection);
            }
            else
            {
                _ApplyCorrection(newIopsWeight, true, volId, it->first, it->second, totalConnection);
            }
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
QosCorrectionManager::_ApplyCorrection(uint64_t value, bool iops, uint64_t volId, uint64_t reactor, uint64_t count, uint64_t totalConnection)
{
    QosManager* qosManager = QosManagerSingleton::Instance();
    if (true == iops)
    {
        value = (value <= DEFAULT_MIN_IO_PCS) ? DEFAULT_MIN_IO_PCS : value;
        int64_t weight = (value * count) / totalConnection;
        if (0 == weight)
        {
            weight = 1;
        }
        qosManager->SetVolumeLimit(reactor, volId, weight, true);
    }
    else
    {
        value = (value <= DEFAULT_MIN_BW_PCS) ? DEFAULT_MIN_BW_PCS : value;
        qosManager->SetVolumeLimit(reactor, volId, (int64_t)(value * (count)) / (totalConnection), false);
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
    QosCorrection qosCorrection = qosContext->GetQosCorrection();
    QosParameters qosParameters = qosContext->GetQosParameters();

    for (map<uint32_t, uint32_t>::iterator it = activeVolumeMap.begin(); it != activeVolumeMap.end(); it++)
    {
        uint64_t currentBwWeight = 0;
        uint64_t currentIopsWeight = 0;
        uint32_t volId = it->first;
        uint32_t totalConnection = qosContext->GetTotalConnection(volId);
        VolumeUserPolicy& volumeUserPolicy = allVolUserPolicy.GetVolumeUserPolicy(volId);
        for (map<uint32_t, uint32_t>::iterator it = volReactorMap[volId].begin(); it != volReactorMap[volId].end(); ++it)
        {
            currentBwWeight += qosManager->GetVolumeLimit(it->first, volId, false);
            currentIopsWeight += qosManager->GetVolumeLimit(it->first, volId, true);
        }

        uint64_t userSetBwWeight = volumeUserPolicy.GetMaxBandwidth();
        uint64_t userSetIopsWeight = volumeUserPolicy.GetMaxIops();
        for (map<uint32_t, uint32_t>::iterator it = volReactorMap[volId].begin(); it != volReactorMap[volId].end(); ++it)
        {
            currentBwWeight = currentBwWeight >= userSetBwWeight ? userSetBwWeight : currentBwWeight;
            qosManager->SetVolumeLimit(it->first, volId, (currentBwWeight * (it->second)) / (totalConnection), false);
            currentIopsWeight = currentIopsWeight >= userSetIopsWeight ? userSetIopsWeight : currentIopsWeight;
            int64_t weight = (currentIopsWeight * it->second) / totalConnection;
            if (0 == weight)
            {
                weight = 1;
            }
            qosManager->SetVolumeLimit(it->first, volId, weight, true);
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
    uint32_t minVolId = allVolUserPolicy.GetMinimumGuaranteeVolume();
    if (minVolId != DEFAULT_MIN_VOL)
    {
        _HandleMinThrottling(minVolId);
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

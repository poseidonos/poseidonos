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

#include "src/qos/correction_manager.h"

#include "src/include/pos_event_id.hpp"
#include "src/qos/qos_context.h"
#include "src/qos/qos_manager.h"
#include "src/qos/reactor_heap.h"
#include "src/qos/throttling_policy_deficit.h"

namespace pos
{
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosCorrectionManager::QosCorrectionManager(QosContext* qosCtx, QosManager* qosManager)
: qosContext(qosCtx),
  qosManager(qosManager)
{
    reactorMinHeap = new ReactorHeap();
    nextManagerType = QosInternalManager_Unknown;
    throttlingLogic = new ThrottlingPolicyDeficit(qosCtx, qosManager);
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
    delete reactorMinHeap;
    delete throttlingLogic;
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
            case QosCorrectionDir_PriorityHigher:
                weight = PRIO_WT_HIGHER;
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
            case QosCorrectionDir_PriorityLower:
                weight = PRIO_WT_LOWER;
                break;
            case QosCorrectionDir_PriorityLowest:
                weight = PRIO_WT_LOWEST;
                break;
            case QosCorrectionDir_Reset:
                weight = M_DEFAULT_WEIGHT;
            default:
                break;
        }
        if (weight <= M_MAX_NEGATIVE_WEIGHT)
        {
            weight = M_MAX_NEGATIVE_WEIGHT;
        }
        if (weight >= M_MAX_POSITIVE_WEIGHT)
        {
            weight = M_MAX_POSITIVE_WEIGHT;
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
QosCorrectionManager::Reset(void)
{
    throttlingLogic->Reset();
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosCorrectionManager::_HandleMinThrottling(std::vector<std::pair<uint32_t, uint32_t>> minVolId)
{
    QosCorrection& qosCorrection = qosContext->GetQosCorrection();
    AllVolumeThrottle& allVolumeThrottle = qosCorrection.GetVolumeThrottlePolicy();
    std::map<uint32_t, map<uint32_t, uint32_t>> volReactorMap = qosContext->GetActiveVolumeReactors();
    QosParameters qosParameters = qosContext->GetQosParameters();
    QosUserPolicy& qosUserPolicy = qosContext->GetQosUserPolicy();
    AllVolumeUserPolicy& allVolUserPolicy = qosUserPolicy.GetAllVolumeUserPolicy();

    std::map<std::pair<uint32_t, uint32_t>, VolumeThrottle>& volumeThrottleMap = allVolumeThrottle.GetVolumeThrottleMap();
    throttlingLogic->IncrementCycleCount();
    for (auto it = volumeThrottleMap.begin(); it != volumeThrottleMap.end(); it++)
    {
        VolumeThrottle& volThrottle = it->second;

        uint32_t arrayId = it->first.first;
        uint32_t volId = it->first.second;
        uint32_t globalVolId = arrayId * MAX_VOLUME_COUNT + volId;
        uint64_t newWeight = 0;
        uint32_t totalConnection = qosContext->GetTotalConnection(globalVolId);

        VolumeUserPolicy* volumeUserPolicy = allVolUserPolicy.GetVolumeUserPolicy(arrayId, volId);
        if (volumeUserPolicy == nullptr)
        {
            continue;
        }

        newWeight = throttlingLogic->GetNewWeight(volId, arrayId, &volThrottle);

        if (newWeight == 0)
        {
            continue;
        }

        for (map<uint32_t, uint32_t>::iterator it = volReactorMap[globalVolId].begin(); it != volReactorMap[globalVolId].end(); ++it)
        {
            bool correctionTypeBw = throttlingLogic->GetCorrectionType(volId, arrayId);
            if (true == correctionTypeBw)
            {
                _ApplyCorrection(newWeight, false, globalVolId, it->first, it->second, totalConnection);
            }
            else
            {
                _ApplyCorrection(newWeight, true, globalVolId, it->first, it->second, totalConnection);
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
QosCorrectionManager::_ApplyCorrection(uint64_t value, bool iops, uint64_t globalVolId, uint64_t reactor, uint64_t count, uint64_t totalConnection)
{
    uint32_t volId = globalVolId % MAX_VOLUME_COUNT;
    uint32_t arrayId = globalVolId / MAX_VOLUME_COUNT;
    if (true == iops)
    {
        value = (value <= DEFAULT_MIN_IO_PCS) ? DEFAULT_MIN_IO_PCS : value;
        int64_t weight = (value * count) / totalConnection;
        if (0 == weight)
        {
            weight = 1;
        }
        qosManager->SetVolumeLimit(reactor, volId, weight, true, arrayId);
    }
    else
    {
        value = (value <= DEFAULT_MIN_BW_PCS) ? DEFAULT_MIN_BW_PCS : value;
        qosManager->SetVolumeLimit(reactor, volId, (int64_t)(value * (count)) / (totalConnection), false, arrayId);
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
QosCorrectionManager::_HandleMaxThrottling(void)
{
 /*   std::map<uint32_t, uint32_t> activeVolumeMap = qosContext->GetActiveVolumes();
    std::map<uint32_t, map<uint32_t, uint32_t>> volReactorMap = qosContext->GetActiveVolumeReactors();
    QosUserPolicy& qosUserPolicy = qosContext->GetQosUserPolicy();
    AllVolumeUserPolicy& allVolUserPolicy = qosUserPolicy.GetAllVolumeUserPolicy();
    std::vector<std::pair<uint32_t, uint32_t>> minVolId = allVolUserPolicy.GetMinimumGuaranteeVolume();
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
        reactorMinHeap->ClearHeap();
        for (map<uint32_t, uint32_t>::iterator it = volReactorMap[volId].begin(); it != volReactorMap[volId].end(); ++it)
        {
            currentBwWeight += qosManager->GetVolumeLimit(it->first, volumeId, false, arrayId);
            uint64_t iopsWeight = qosManager->GetVolumeLimit(it->first, volumeId, true, arrayId);
            currentIopsWeight += iopsWeight;
            reactorMinHeap->InsertPairInHeap(iopsWeight, it->first);
        }

        uint64_t userSetBwWeight = volumeUserPolicy->GetMaxBandwidth();
        uint64_t userSetIopsWeight = volumeUserPolicy->GetMaxIops();
        uint64_t differenceInWeight = 0;
        if (minVolId.size() != 0)
        {
            currentBwWeight = currentBwWeight >= userSetBwWeight ? userSetBwWeight : currentBwWeight;
            currentIopsWeight = currentIopsWeight >= userSetIopsWeight ? userSetIopsWeight : currentIopsWeight;
            if (currentBwWeight == 0)
            {
                currentBwWeight = userSetBwWeight;
            }
            if (currentIopsWeight == 0)
            {
                currentIopsWeight = userSetIopsWeight;
            }
        }
        else
        {
            currentBwWeight = userSetBwWeight;
            currentIopsWeight = userSetIopsWeight;
        }

        uint64_t newBwLimit = currentBwWeight / totalConnection;
        uint64_t newIopsLimit = currentIopsWeight / totalConnection;

        if (newIopsLimit == 0)
        {
            newIopsLimit = 1;
        }
        uint64_t totalWeightSet = newIopsLimit * totalConnection;
        differenceInWeight = userSetIopsWeight - totalWeightSet;

        std::vector<uint32_t> reactors = reactorMinHeap->GetAllReactorIds();
        uint32_t extraIops = 1;
        for (auto& it : reactors)
        {
            uint32_t reactorId = it;
            qosManager->SetVolumeLimit(reactorId, volumeId, newBwLimit, false, arrayId);
            if (differenceInWeight == 0)
            {
                extraIops = 0;
            }
            else
            {
                differenceInWeight--;
            }
            qosManager->SetVolumeLimit(reactorId, volumeId, newIopsLimit + extraIops, true, arrayId);
        }
        std::vector<uint32_t> inactiveReactors = qosContext->GetInactiveReactorsList(volId);
        for (auto& it : inactiveReactors)
        {
            qosManager->SetVolumeLimit(it, volumeId, 0, false, arrayId);
            qosManager->SetVolumeLimit(it, volumeId, 0, true, arrayId);
        }
    }*/
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
    std::vector<std::pair<uint32_t, uint32_t>> minVolId = allVolUserPolicy.GetMinimumGuaranteeVolume();
    if (minVolId.size() != 0)
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

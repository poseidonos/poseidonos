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
#include "src/qos/throttling_policy_deficit.h"
#include <queue>
#include <list>
#include <tuple>

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

uint64_t
QosCorrectionManager::_GetUserMaxWeight(uint32_t arrayId, uint32_t volId, bool iops)
{
    QosUserPolicy& qosUserPolicy = qosContext->GetQosUserPolicy();
    AllVolumeUserPolicy& allVolUserPolicy = qosUserPolicy.GetAllVolumeUserPolicy();
    VolumeUserPolicy* volumeUserPolicy = allVolUserPolicy.GetVolumeUserPolicy(arrayId, volId);
    if (volumeUserPolicy == nullptr)
    {
        return 0;
    }
    if (iops == false)
    {
        return volumeUserPolicy->GetMaxBandwidth();
    }
    return volumeUserPolicy->GetMaxIops();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosCorrectionManager::_HandleMaxThrottling(bool minPolicy)
{
    std::list<std::pair<uint32_t, uint32_t>> allMountedVolumeList;
    qosManager->GetMountedVolumes(allMountedVolumeList);
    for (auto volPair : allMountedVolumeList)
    {
        uint32_t arrayId = volPair.first;
        uint32_t volId = volPair.second;
        uint64_t userSetBwWeight = _GetUserMaxWeight(arrayId, volId, false);
        uint64_t userSetIops = _GetUserMaxWeight(arrayId, volId, true);
        qosManager->SetVolumeLimit(volId, userSetBwWeight, false, arrayId);
        qosManager->SetVolumeLimit(volId, userSetIops, true, arrayId);
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
    std::vector<std::pair<uint32_t, uint32_t>> minVolId = allVolUserPolicy.GetMinimumGuaranteeVolume();
    _HandleMaxThrottling(true);
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

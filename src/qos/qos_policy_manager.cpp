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

#include "src/qos/qos_policy_manager.h"

#include <utility>

#include "src/logger/logger.h"
#include "src/network/nvmf_volume_pos.hpp"
#include "src/qos/qos_avg_compute.h"
#include "src/qos/qos_manager.h"
#include "src/sys_event/volume_event_publisher.h"

namespace pos
{
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int
QosPolicyManager::SetEventPolicy(BackendEvent event, EventPriority priority, uint32_t weight)
{
    if (true == qosVolumePolicyManager->VolumeMinimumPolicyInEffect())
    {
        return QosReturnCode::VOLUME_POLICY_IN_EFFECT;
    }

    return qosEventPolicyManager->SetEventPolicy(event, priority, weight);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int
QosEventPolicyManager::SetEventPolicy(BackendEvent event, EventPriority priority, uint32_t weight)
{
    std::unique_lock<std::mutex> uniqueLock(eventPolicyLock);
    std::pair<EventPriority, uint32_t> eventPair;
    eventPair.first = priority;
    eventPair.second = weight;
    eventPolicyMapCli[event] = eventPair;
    eventDirty = true;
    eventWrrPolicyInEffect = true;

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
QosPolicyManager::CopyEventPolicy(void)
{
    qosEventPolicyManager->CopyEventPolicy();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosPolicyManager::ResetEventPolicy(void)
{
    qosEventPolicyManager->ResetEventPolicy();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosEventPolicyManager::ResetEventPolicy(void)
{
    std::unique_lock<std::mutex> uniqueLock(eventPolicyLock);
    for (uint32_t event = BackendEvent_Start; event < BackendEvent_Count; event++)
    {
        eventPolicyMapCli[(BackendEvent)event] = {EventPriority_Critical, 1};
    }
    eventDirty = true;
    eventWrrPolicyInEffect = false;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosEventPolicyManager::CopyEventPolicy(void)
{
    if (eventDirty == true)
    {
        std::unique_lock<std::mutex> uniqueLock(eventPolicyLock);
        eventPolicyMapQos = eventPolicyMapCli;
        _ApplyEventPolicy();
        eventDirty = false;
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
QosPolicyManager::CopyVolumePolicy(void)
{
    qosVolumePolicyManager->CopyVolumePolicy();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumePolicyManager::CopyVolumePolicy(void)
{
    std::unique_lock<std::mutex> uniqueLock(volumePolicyLock);
    if (false == volMinPolicyInEffect)
    {
        return;
    }
    for (int id = 0; ((id < MAX_VOLUME_COUNT)); id++)
    {
        if (policyDirty[id] == false)
            continue;
        memcpy(&volPolicy[id], &volPolicyCli[id], sizeof(qos_vol_policy));
        policyDirty[id] = false;
        volPolicy[id].minBW = volPolicy[id].minBW * (M_KBYTES * M_KBYTES / IBOF_QOS_TIMESLICE_IN_USEC);
        if (volPolicy[id].minBW == 0)
        {
            volPolicy[id].minBW = M_DEFAULT_MIN_BW;
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
int
QosPolicyManager::UpdateVolumePolicy(int volId, qos_vol_policy policy)
{
    if (true == qosEventPolicyManager->EventWrrPolicyInEffect())
    {
        return QosReturnCode::EVENT_POLICY_IN_EFFECT;
    }
    return qosVolumePolicyManager->UpdateVolumePolicy(volId, policy);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int
QosVolumePolicyManager::UpdateVolumePolicy(int volId, qos_vol_policy policy)
{
    std::unique_lock<std::mutex> uniqueLock(volumePolicyLock);
    volPolicyCli[volId].workLoad = policy.workLoad;
    volPolicyCli[volId].minBW = policy.minBW;
    policyDirty[volId] = true;
    if (volPolicyCli[volId].minBW != 0)
    {
        if (minGuranteeVolId != volId)
        {
            volPolicyCli[minGuranteeVolId].minBW = 0;
            volPolicyCli[minGuranteeVolId].minGurantee = false;
            policyDirty[minGuranteeVolId] = true;
        }
        volPolicyCli[volId].minGurantee = true;
        minGuranteeVolId = volId;
    }

    if (volPolicyCli[volId].minBW == 0)
    {
        if (volId == minGuranteeVolId)
        {
            minGuranteeVolId = MAX_VOLUME_COUNT + 1;
            volPolicyCli[volId].minGurantee = false;
            volPolicyCli[volId].minBW = 0;
        }
    }
    if ((MAX_VOLUME_COUNT + 1) != minGuranteeVolId)
    {
        volMinPolicyInEffect = true;
    }
    else
    {
        volMinPolicyInEffect = false;
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
qos_vol_policy
QosPolicyManager::GetVolumePolicy(int volId)
{
    return qosVolumePolicyManager->GetVolumePolicy(volId);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
qos_vol_policy
QosVolumePolicyManager::GetVolumePolicy(int volId)
{
    return volPolicyCli[volId];
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

QosPolicyManager::QosPolicyManager(void)
{
    qosVolumePolicyManager = new QosVolumePolicyManager();
    qosEventPolicyManager = new QosEventPolicyManager();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

QosPolicyManager::~QosPolicyManager(void)
{
    delete qosVolumePolicyManager;
    delete qosEventPolicyManager;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosEventPolicyManager::_ApplyEventPolicy(void)
{
    int32_t weight = 0;
    EventPriority priority = EventPriority_Unknown;
    std::list<int32_t> priorityWeights;
    for (uint32_t event = 0; event < BackendEvent_Count; event++)
    {
        weight = eventPolicyMapQos[(BackendEvent)event].second;
        priority = eventPolicyMapQos[(BackendEvent)event].first;
        int64_t val = eventPriorityWeights[priority][weight - 1];
        QosManagerSingleton::Instance()->SetEventWeightWRR((BackendEvent)event, val);
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
QosPolicyManager::Initialize(void)
{
    qosVolumePolicyManager->Initialize(this);
    qosEventPolicyManager->Initialize(this);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosPolicyManager::ResetAll(void)
{
    qosVolumePolicyManager->ResetAll();
    qosEventPolicyManager->ResetAll();
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

bool
QosVolumePolicyManager::_AvgBwCompute(int timeIndex)
{
    bool ret = true;
    for (map<int, int>::iterator it = activeVolMap.begin(); it != activeVolMap.end(); it++)
    {
        int volId = it->first;
        avgVolBw[bufIndex][volId] = curVolState[volId].currentData / timeIndex;
        if (false == movAvg->EnqueueAvgData(volId, avgVolBw[bufIndex][volId]))
        {
            ret = false;
        }
        avgVolBw[bufIndex][volId] = movAvg->GetMovingAvg(volId);
    }
    return ret;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

bool
QosVolumePolicyManager::_IsQosCompromized(int volId, qos_state_ctx* ctx)
{
    bool ret = false;
    uint64_t cycleMin = volPolicy[volId].minBW;
    uint64_t cycleMax = QosManagerSingleton::Instance()->GetMaxVolumeWeight(volId);

    if ((avgVolBw[bufIndex][volId] <= cycleMin))
    {
        volBwDeficit = cycleMin - ((avgVolBw[bufIndex][volId]));
        switch (volPolicy[volId].workLoad)
        {
            case QosWorkloadType_Write:
            {
                eventState userDataRebuildState = qosPolicyManager->GetCurrentEventState(BackendEvent_UserdataRebuild);
                eventState flushState = qosPolicyManager->GetCurrentEventState(BackendEvent_Flush);
                if (userDataRebuildState.pendingCPUEvents > 0 && (flushState.pendingCPUEvents > 0))
                {
                    ctx->causeType = QosCause_CPU;
                }
                else
                {
                    ctx->causeType = QosCause_NVRAM;
                }

                break;
            }
            case QosWorkloadType_Read:
            {
                eventState userDataRebuildState = qosPolicyManager->GetCurrentEventState(BackendEvent_UserdataRebuild);
                eventState frontendIOState = qosPolicyManager->GetCurrentEventState(BackendEvent_FrontendIO);
                if ((userDataRebuildState.pendingCPUEvents > 0) && (frontendIOState.pendingCPUEvents > 0))
                {
                    ctx->causeType = QosCause_CPU;
                }
                else
                {
                    ctx->causeType = QosCause_Disk;
                }
                break;
            }
            default:
                break;
        }
        if (volPolicy[volId].minGurantee == false)
        {
            lowConsumerVolList.insert(lowConsumerVolList.end(), volId);
            return false;
        }
        ctx->victimVol = volId;
        ctx->previousAvgBW = avgVolBw[bufIndex][volId];
        return true;
    }
    else if ((avgVolBw[bufIndex][volId]) < cycleMax)
    {
        if (volPolicy[volId].minGurantee == true)
        {
            if (ctx->causeType != QosCause_None)
            {
                ctx->previousCauseType = ctx->causeType;
                ctx->causeType = QosCause_None;
            }
            return false;
        }
        ret = false;
        intermediateConsumerVolList.insert(intermediateConsumerVolList.end(), volId);
    }
    else
    {
        if (volPolicy[volId].minGurantee == true)
        {
            if (ctx->causeType != QosCause_None)
            {
                ctx->previousCauseType = ctx->causeType;
                ctx->causeType = QosCause_None;
            }
            return false;
        }
        ret = false;
        highConsumerVolList.insert(highConsumerVolList.end(), volId);
    }

    return ret;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

bool
QosVolumePolicyManager::_MonitorParams(qos_state_ctx* ctx)
{
    bool ret = false;
    int volId = 0;

    if (qosCycle >= M_QOS_CORRECTION_CYCLE)
    {
        highConsumerVolList.clear();
        intermediateConsumerVolList.clear();
        lowConsumerVolList.clear();
        for (map<int, int>::iterator it = activeVolMap.begin(); it != activeVolMap.end(); it++)
        {
            volId = it->first;
            int temp_ret = _IsQosCompromized(volId, ctx);
            if (true == temp_ret)
            {
                ret = true;
            }
        }
        consumerCount = highConsumerVolList.size() + intermediateConsumerVolList.size();
    }
    return ret;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosVolumePolicyManager::_UpdateOldParamVolumeState(int volId)
{
    curVolState[volId].pState.currentData = curVolState[volId].currentData;
    curVolState[volId].pState.nvramUsage = curVolState[volId].nvramUsage;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosEventPolicyManager::_UpdateOldParamEventState(BackendEvent event)
{
    curEventState[event].pState.currentData = curEventState[event].currentData;
    curEventState[event].pState.pendingCPUEvents = curEventState[event].pendingCPUEvents;
    curEventState[event].pState.totalEvents = curEventState[event].totalEvents;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosEventPolicyManager::_FillFlushEventState(BackendEvent event)
{
    QosManager* qosMgr = QosManagerSingleton::Instance();
    curEventState[event].currentData += eventParams[event].currentBW;
    curEventState[event].pendingCPUEvents = qosMgr->GetPendingEvents(event);
    curEventState[event].totalEvents = qosMgr->GetEventLog(event);
    curEventState[event].flushData.nvramStripes = qosMgr->GetUsedStripeCnt();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosEventPolicyManager::_FillGCEventState(BackendEvent event)
{
    QosManager* qosMgr = QosManagerSingleton::Instance();
    curEventState[event].currentData += eventParams[event].currentBW;
    curEventState[event].pendingCPUEvents = qosMgr->GetPendingEvents(event);
    curEventState[event].totalEvents = qosMgr->GetEventLog(event);
    curEventState[event].gcData.freeSegment = qosMgr->GetFreeSegmentCnt();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosEventPolicyManager::_FillMetaRebuildEventState(BackendEvent event)
{
    QosManager* qosMgr = QosManagerSingleton::Instance();
    curEventState[event].currentData += eventParams[event].currentBW;
    curEventState[event].pendingCPUEvents = qosMgr->GetPendingEvents(event);
    curEventState[event].totalEvents = qosMgr->GetEventLog(event);
    curEventState[event].rebuildData.metaRebuildStripe = qosMgr->GetRemainingMetaRebuildStripeCnt();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosEventPolicyManager::_FillUserRebuildEventState(BackendEvent event)
{
    QosManager* qosMgr = QosManagerSingleton::Instance();
    curEventState[event].currentData += eventParams[event].currentBW;
    curEventState[event].pendingCPUEvents = qosMgr->GetPendingEvents(event);
    curEventState[event].totalEvents = qosMgr->GetEventLog(event);
    curEventState[event].rebuildData.userRebuidSegment = qosMgr->GetRemainingUserRebuildSegmentCnt();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosEventPolicyManager::_FillMetaEventState(BackendEvent event)
{
    QosManager* qosMgr = QosManagerSingleton::Instance();
    curEventState[event].currentData += eventParams[event].currentBW;
    curEventState[event].pendingCPUEvents = qosMgr->GetPendingEvents(event);
    curEventState[event].totalEvents = qosMgr->GetEventLog(event);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosEventPolicyManager::_FillFrontEventState(BackendEvent event)
{
    static long counteravg1 = 0;
    static long term1 = 0;
    QosManager* qosMgr = QosManagerSingleton::Instance();
    counteravg1 = counteravg1 + curEventState[event].pendingCPUEvents;
    curEventState[event].currentData += eventParams[event].currentBW;
    curEventState[event].pendingCPUEvents = qosMgr->GetPendingEvents(event);
    curEventState[event].totalEvents = qosMgr->GetEventLog(event);
    if (++term1 == 1000)
    {
        term1 = 0;
        counteravg1 = 0;
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
QosPolicyManager::FillEventState(BackendEvent event)
{
    qosEventPolicyManager->FillEventState(event);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosEventPolicyManager::FillEventState(BackendEvent event)
{
    switch (event)
    {
        case BackendEvent_Flush:
            _FillFlushEventState(event);
            break;
        case BackendEvent_GC:
            _FillGCEventState(event);
            break;
        case BackendEvent_UserdataRebuild:
            _FillUserRebuildEventState(event);
            break;
        case BackendEvent_MetadataRebuild:
            _FillMetaRebuildEventState(event);
            break;
        case BackendEvent_MetaIO:
            _FillMetaEventState(event);
            break;
        case BackendEvent_FrontendIO:
            _FillFrontEventState(event);
            break;
        default:
            break;
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
QosPolicyManager::FillVolumeState(int volId)
{
    qosVolumePolicyManager->FillVolumeState(volId);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumePolicyManager::FillVolumeState(int volId)
{
    curVolState[volId].currentData += volParams[volId].currentBW;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosVolumePolicyManager::_ResetVolumeState(int volId)
{
    curVolState[volId].currentData = 0;
    curVolState[volId].entries = 0;
    curVolState[volId].pendingData = 0;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosEventPolicyManager::_ResetEventState(BackendEvent event)
{
    curEventState[event].currentData = 0;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosVolumePolicyManager::_ApplyThrottling(uint64_t throttleValue, bool increase)
{
    for (list<int>::iterator it = highConsumerVolList.begin(); it != highConsumerVolList.end(); it++)
    {
        if (increase == true)
        {
            _IncreaseThrottling(*it, throttleValue);
        }
    }
    for (list<int>::iterator it = intermediateConsumerVolList.begin(); it != intermediateConsumerVolList.end(); it++)
    {
        if (increase == true)
        {
            _IncreaseThrottling(*it, throttleValue);
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
QosVolumePolicyManager::_ApplyCorrection(qos_state_ctx* context)
{
    switch (context->causeType)
    {
        case QosCause_Disk:
        case QosCause_NVRAM:
            if (consumerCount != 0)
            {
                context->correctionValue = (10 * M_KBYTES * M_KBYTES / IBOF_QOS_TIMESLICE_IN_USEC);
                _ApplyThrottling(context->correctionValue, true);
            }
            break;
        default:
            break;
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
QosVolumePolicyManager::_IncreaseThrottling(uint32_t volId, uint64_t correction)
{
    QosManager* qosMgr = QosManagerSingleton::Instance();
    uint64_t volumeSpecific = 0;
    std::map<uint32_t, uint32_t>& volReactorMap = qosMgr->GetVolumeReactorMap(volId);
    for (std::map<uint32_t, uint32_t>::iterator it = volReactorMap.begin(); it != volReactorMap.end(); ++it)
    {
        volumeSpecific += qosMgr->GetVolumeWeight(it->first, volId);
    }

    if (volumeSpecific >= qosMgr->GetMaxVolumeWeight(volId))
    {
        volumeSpecific = avgVolBw[bufIndex][volId] - correction;
    }
    else
    {
        volumeSpecific -= correction;
    }

    if (volumeSpecific < volPolicy[volId].minBW)
    {
        volumeSpecific = volPolicy[volId].minBW;
    }
    if (correction > volumeSpecific)
    {
        volumeSpecific = volPolicy[volId].minBW;
    }
    for (std::map<uint32_t, uint32_t>::iterator it = volReactorMap.begin(); it != volReactorMap.end(); ++it)
    {
        qosMgr->SetVolumeWeight(it->first, volId, (volumeSpecific * (it->second)) / qosMgr->GetVolumeTotalConnection(volId));
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
QosPolicyManager::CheckSystem(uint64_t timeSpent)
{
    qosVolumePolicyManager->CheckSystem(timeSpent);
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumePolicyManager::CheckSystem(uint64_t timeSpent)
{
    qosCycle++;
    bool loopContinue = true;
    do
    {
        switch (currentState)
        {
            case QosState_IssueDetect:
            {
                if (false == _AvgBwCompute(timeSpent))
                {
                    loopContinue = false;
                    break;
                }
                if (true == _MonitorParams(&context))
                {
                    currentState = QosState_ApplyCorrection;
                }
                else
                {
                    if ((qosCycle % (uint32_t)(0.4 * (M_QOS_CORRECTION_CYCLE))) == 0)
                    {
                        currentState = QosState_ResetThrottling;
                    }
                    loopContinue = false;
                }
                break;
            }
            case QosState_ApplyCorrection:
            {
                _ApplyCorrection(&context);
                currentState = QosState_IssueDetect;
                loopContinue = false;
                break;
            }
            default:
                loopContinue = false;
                break;
        }
    } while (loopContinue);
    if (qosCycle >= M_QOS_CORRECTION_CYCLE)
    {
        qosCycle = 0;
        bufIndex = bufIndex ^ 1;
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bool
QosPolicyManager::VolumeMinimumPolicyInEffect(void)
{
    return qosVolumePolicyManager->VolumeMinimumPolicyInEffect();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bool
QosVolumePolicyManager::VolumeMinimumPolicyInEffect(void)
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
QosPolicyManager::InsertActiveVolumeMap(std::pair<int, int> volPair)
{
    qosVolumePolicyManager->InsertActiveVolumeMap(volPair);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosPolicyManager::UpdateVolumeParam(uint32_t volId, volume_qos_params volParam)
{
    qosVolumePolicyManager->UpdateVolumeParam(volId, volParam);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosPolicyManager::InsertActiveConnection(std::pair<std::pair<uint32_t, uint32_t>, uint32_t> reactorVolPair)
{
    qosVolumePolicyManager->InsertActiveConnection(reactorVolPair);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
uint32_t
QosPolicyManager::GetActiveConnectionsCount(void)
{
    return qosVolumePolicyManager->GetActiveConnectionsCount();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
std::map<int, int>&
QosPolicyManager::GetActiveVolumeMap(void)
{
    return qosVolumePolicyManager->GetActiveVolumeMap();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosPolicyManager::UpdateEventParam(uint32_t eventId, event_qos_params eventParam)
{
    qosEventPolicyManager->UpdateEventParam(eventId, eventParam);
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosPolicyManager::ActiveVolumeClear(void)
{
    qosVolumePolicyManager->ActiveVolumeClear();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosWorkloadType
QosPolicyManager::GetMinVolumeWorkloadType(void)
{
    return qosVolumePolicyManager->GetMinVolumeWorkloadType();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosPolicyManager::ActiveEventClear(void)
{
    qosEventPolicyManager->ActiveEventClear();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
eventState
QosPolicyManager::GetCurrentEventState(BackendEvent event)
{
    return qosEventPolicyManager->GetCurrentEventState(event);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumePolicyManager::InsertActiveVolumeMap(std::pair<int, int> volPair)
{
    activeVolMap.insert(volPair);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumePolicyManager::UpdateVolumeParam(uint32_t volId, volume_qos_params volParam)
{
    volParams[volId] = volParam;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumePolicyManager::InsertActiveConnection(std::pair<std::pair<uint32_t, uint32_t>, uint32_t> reactorVolPair)
{
    activeConnections.insert(reactorVolPair);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
uint32_t
QosVolumePolicyManager::GetActiveConnectionsCount(void)
{
    return activeConnections.size();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
std::map<int, int>&
QosVolumePolicyManager::GetActiveVolumeMap(void)
{
    return activeVolMap;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumePolicyManager::ActiveVolumeClear(void)
{
    for (std::map<int, int>::iterator it = activeVolMap.begin(); it != activeVolMap.end(); it++)
    {
        _UpdateOldParamVolumeState(it->first);
        _ResetVolumeState(it->first);
    }
    activeVolMap.clear();
    activeConnections.clear();
    for (uint16_t i = 0; i < MAX_VOLUME_COUNT; i++)
    {
        volParams[i].valid = M_INVALID_ENTRY;
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosWorkloadType
QosVolumePolicyManager::GetMinVolumeWorkloadType(void)
{
    return volPolicy[minGuranteeVolId].workLoad;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosVolumePolicyManager::QosVolumePolicyManager(void)
{
    bufIndex = 0;
    volMinPolicyInEffect = false;
    minGuranteeVolId = MAX_VOLUME_COUNT + 1;
    context.victimVol = MAX_VOLUME_COUNT + 1;
    qosCycle = 0;
    volBwDeficit = 0;
    consumerCount = 0;
    currentState = QosState_IssueDetect;
    for (int volId = 0; volId < MAX_VOLUME_COUNT; volId++)
    {
        volPolicy[volId].workLoad = QosWorkloadType_Read;
        volPolicy[volId].minBW = M_DEFAULT_MIN_BW;
        volPolicy[volId].priority = QosPriority_Low;
        volPolicy[volId].minGurantee = false;
        _ResetVolumeState(volId);
        policyDirty[volId] = false;
    }
    for (int i = 0; i < M_PING_PONG_BUFFER; i++)
    {
        for (int j = 0; j < M_MAX_REACTORS; j++)
        {
            avgVolBw[i][j] = 0;
        }
    }
    movAvg = new MovingAvgCompute(M_QOS_CORRECTION_CYCLE);
    qosPolicyManager = nullptr;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosVolumePolicyManager::~QosVolumePolicyManager(void)
{
    delete movAvg;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumePolicyManager::Initialize(QosPolicyManager* qosPolicyMgr)
{
    qosPolicyManager = qosPolicyMgr;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumePolicyManager::ResetAll(void)
{
    activeVolMap.clear();
    bufIndex = 0;
    for (int volId = 0; volId < MAX_VOLUME_COUNT; volId++)
    {
        movAvg->ResetMovingAvg(volId);
    }
    context.causeType = QosCause_None;
    context.previousCauseType = QosCause_None;
    context.victimVol = MAX_VOLUME_COUNT + 1;
    context.correctionValue = 0;
    context.previousAvgBW = 0;
    context.iterationCnt = 0;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosEventPolicyManager::QosEventPolicyManager(void)
{
    for (uint32_t event = BackendEvent_Start; event < BackendEvent_Count; event++)
    {
        eventPolicyMapCli[(BackendEvent)event] = {EventPriority_Critical, 1};
        eventPolicyMapQos[(BackendEvent)event] = {EventPriority_Critical, 1};
    }
    std::vector<int32_t> critWt({PRIO_CRIT_WT_1, PRIO_CRIT_WT_2, PRIO_CRIT_WT_3});
    eventPriorityWeights[EventPriority_Critical] = critWt;
    std::vector<int32_t> highWt({PRIO_HIGH_WT_1, PRIO_HIGH_WT_2, PRIO_HIGH_WT_3});
    eventPriorityWeights[EventPriority_High] = highWt;
    std::vector<int32_t> lowWt({PRIO_LOW_WT_1, PRIO_LOW_WT_2, PRIO_LOW_WT_3});
    eventPriorityWeights[EventPriority_Low] = lowWt;
    eventWrrPolicyInEffect = false;
    eventDirty = false;
    qosPolicyManager = nullptr;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosEventPolicyManager::~QosEventPolicyManager(void)
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
QosEventPolicyManager::Initialize(QosPolicyManager* qosPolicyMgr)
{
    qosPolicyManager = qosPolicyMgr;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bool
QosEventPolicyManager::EventWrrPolicyInEffect(void)
{
    return eventWrrPolicyInEffect;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosEventPolicyManager::ResetAll(void)
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
QosEventPolicyManager::UpdateEventParam(uint32_t eventId, event_qos_params eventParam)
{
    eventParams[eventId] = eventParam;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosEventPolicyManager::ActiveEventClear(void)
{
    for (uint32_t event = 0; (BackendEvent)event < BackendEvent_Count; event++)
    {
        _UpdateOldParamEventState((BackendEvent)event);
        _ResetEventState((BackendEvent)event);
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
eventState
QosEventPolicyManager::GetCurrentEventState(BackendEvent event)
{
    return curEventState[event];
}

} // namespace pos

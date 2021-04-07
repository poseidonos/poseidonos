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
#include "src/network/nvmf_volume_ibof.hpp"
#include "src/qos/qos_avg_compute.h"
#include "src/qos/qos_manager.h"
#include "src/sys_event/volume_event_publisher.h"

#if defined QOS_ENABLED_FE
extern uint32_t spdk_nvmf_get_reactor_subsystem_mapping(uint32_t reactor, uint32_t subsys);
extern void spdk_nvmf_initialize_reactor_subsystem_mapping(void);
#endif
namespace ibofos
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
    if (true == volMinPolicyInEffect)
    {
        return QosReturnCode::VOLUME_POLICY_IN_EFFECT;
    }
    std::unique_lock<std::mutex> uniqueLock(policyUpdateLock);
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
QosPolicyManager::ResetEventPolicy(void)
{
    std::unique_lock<std::mutex> uniqueLock(policyUpdateLock);
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
QosPolicyManager::CopyEventPolicy(void)
{
    if (eventDirty == true)
    {
        std::unique_lock<std::mutex> uniqueLock(policyUpdateLock);
        eventPolicyMapQos = eventPolicyMapCli;
        ApplyEventPolicy();
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
QosPolicyManager::InitilizeVolumePolicy(int volId)
{
    volPolicy[volId].workLoad = QosWorkloadType_Read;
    volPolicy[volId].minBW = M_DEFAULT_MIN_BW;
    volPolicy[volId].priority = QosPriority_Low;
    volPolicy[volId].minGurantee = false;
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
    int id = 0;
    std::unique_lock<std::mutex> uniqueLock(policyUpdateLock);
    if (false == volMinPolicyInEffect)
    {
        return;
    }
    for (id = 0; ((id < MAX_VOLUME_COUNT)); id++)
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
    if (true == eventWrrPolicyInEffect)
    {
        return QosReturnCode::EVENT_POLICY_IN_EFFECT;
    }
    std::unique_lock<std::mutex> uniqueLock(policyUpdateLock);
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
        IBOF_TRACE_INFO(QosReturnCode::MINIMUM_BW_APPLIED, "Volume Id:{} Min BW:{}", volId, volPolicyCli[volId].minBW);
    }

    if (volPolicyCli[volId].minBW == 0)
    {
        if (volId == minGuranteeVolId)
        {
            minGuranteeVolId = -1;
            volPolicyCli[volId].minGurantee = false;
            volPolicyCli[volId].minBW = 0;
        }
    }
    if (-1 != minGuranteeVolId)
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
    return volPolicyCli[volId];
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

QosPolicyManager::QosPolicyManager()
: volParams{
      0,
  },
  eventParams{
      0,
  },
  bufIndex(UINT16_MAX),
  eventDirty(false),
  minGuranteeVolId(INT32_MAX),
  currentState(QosState_Unknown),
  volBwDeficit(UINT64_MAX),
  consumerCount(UINT32_MAX),
  curVolState{
      0,
  },
  volCycleExcess{
      0,
  },
  curEventState{
      0,
  },
  context{
      QosCause_Unknown,
  },
  qosMgr(nullptr),
  volMinPolicyInEffect(false),
  eventWrrPolicyInEffect(false)
{
    uint32_t maxVolumeCount = MAX_VOLUME_COUNT;
    for (uint32_t volumeId = 0; volumeId < maxVolumeCount; volumeId++)
    {
        policyDirty[volumeId] = false;
    }
    movAvg = new MovingAvgCompute(M_QOS_CORRECTION_CYCLE);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

QosPolicyManager::~QosPolicyManager()
{
    eventPolicyMapCli.clear();
    eventPolicyMapQos.clear();
    eventPriorityWeights.clear();
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
QosPolicyManager::InitializeEventPolicy()
{
    for (uint32_t event = BackendEvent_Start; event < BackendEvent_Count; event++)
    {
        eventPolicyMapCli[(BackendEvent)event] = {EventPriority_Critical, 1};
        eventPolicyMapQos[(BackendEvent)event] = {EventPriority_Critical, 1};
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
QosPolicyManager::_InitializeEventPriorityWeights()
{
    std::vector<int32_t> critWt({PRIO_CRIT_WT_1, PRIO_CRIT_WT_2, PRIO_CRIT_WT_3});
    eventPriorityWeights[EventPriority_Critical] = critWt;
    std::vector<int32_t> highWt({PRIO_HIGH_WT_1, PRIO_HIGH_WT_2, PRIO_HIGH_WT_3});
    eventPriorityWeights[EventPriority_High] = highWt;
    std::vector<int32_t> lowWt({PRIO_LOW_WT_1, PRIO_LOW_WT_2, PRIO_LOW_WT_3});
    eventPriorityWeights[EventPriority_Low] = lowWt;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosPolicyManager::ApplyEventPolicy(void)
{
    int32_t weight = 0;
    EventPriority priority = EventPriority_Unknown;
    std::list<int32_t> priorityWeights;
    for (uint32_t event = 0; event < BackendEvent_Count; event++)
    {
        weight = eventPolicyMapQos[(BackendEvent)event].second;
        priority = eventPolicyMapQos[(BackendEvent)event].first;
        int64_t val = eventPriorityWeights[priority][weight - 1];
        qosMgr->SetEventWeightWRR((BackendEvent)event, val);
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
QosPolicyManager::Initialize()
{
    QosState state;
    state = QosState_IssueDetect;
    SetCurrentState(state);
    for (int volId = 0; volId < MAX_VOLUME_COUNT; volId++)
    {
        volCycleExcess[volId] = 0;
        InitilizeVolumePolicy(volId);
        ResetVolumeState(volId);

        movAvg->dataReadyToProcess[volId] = false;
        policyDirty[volId] = false;
    }
    _InitializeEventPriorityWeights();
    InitializeEventPolicy();
    bufIndex = 0;
    context.victimVol = -1;
    minGuranteeVolId = -1;
    qosMgr = QosManagerSingleton::Instance();
    volMinPolicyInEffect = false;
    eventWrrPolicyInEffect = false;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

QosState
QosPolicyManager::GetCurrentState()
{
    return currentState;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosPolicyManager::SetCurrentState(QosState state)
{
    currentState = state;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

uint32_t
QosPolicyManager::GetMinBWPolicy(int volId)
{
    return (volPolicy[volId].minBW);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

uint32_t
QosPolicyManager::GetMaxBWPolicy(int volId)
{
    return (qosMgr->maxVolWeight[volId]);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

QosWorkloadType
QosPolicyManager::GetWorkloadType(int volId)
{
    return (volPolicy[volId].workLoad);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosPolicyManager::ResetAll()
{
    int volId;
    bufIndex = 0;
    context.causeType = QosCause_None;
    context.previousCauseType = QosCause_None;
    context.victimVol = -1;
    context.correctionValue = 0;
    context.previousAvgBW = 0;
    context.iterationCnt = 0;
    for (volId = 0; volId < MAX_VOLUME_COUNT; volId++)
    {
        movAvg->ResetMovingAvg(volId);
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
QosPolicyManager::AvgBwCompute(int timeIndex)
{
    bool ret = true;
    int volId = 0;
    for (map<int, int>::iterator it = activeVolMap.begin(); it != activeVolMap.end(); it++)
    {
        volId = it->first;
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
QosPolicyManager::IsQosCompromized(int volId, qos_state_ctx* ctx)
{
    bool ret = false;
    uint64_t cycleMin = GetMinBWPolicy(volId);
    uint64_t cycleMax = GetMaxBWPolicy(volId);

    if ((avgVolBw[bufIndex][volId] <= cycleMin))
    {
        volBwDeficit = cycleMin - ((avgVolBw[bufIndex][volId]));
        switch (GetWorkloadType(volId))
        {
            case QosWorkloadType_Write:
            {
                /*
                    if ((qosMgr->GetUsedStripeCnt() < 900))
                    {
                        return false;
                    }
                    */

                if (curEventState[BackendEvent_UserdataRebuild].pendingCPUEvents > 0 && (curEventState[BackendEvent_Flush].pendingCPUEvents > 0))
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
                if ((curEventState[BackendEvent_UserdataRebuild].pendingCPUEvents > 0) && (curEventState[BackendEvent_FrontendIO].pendingCPUEvents > 0))
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
        IBOF_TRACE_INFO(QosReturnCode::MINIMUM_BW_COMPROMISED, "Volume Id:{}, Min BW Compromised", volId);
        ctx->victimVol = volId;
        ctx->previousAvgBW = avgVolBw[bufIndex][volId];
        ret = true;
        return ret;
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

void
QosPolicyManager::SelectQosPolicy(int volId)
{
    if (QosWorkloadType_Write == GetWorkloadType(volId))
    {
        // Check for NVRAM avalibility
        // if No NVRAM available --> Check for the System BW usage (Sum of FE and BE BW)
        // if BW is issue --> check BE BW consumption, GC or Rebuild.
        // if no GC or rebuild --> Check for FE BW , any low priority vol consuming more
        // Apply suitable threshold
        // Save the context
    }
    else if (QosWorkloadType_Read == GetWorkloadType(volId))
    {
        // is Disk consumption is a limitation?
        // Is Rebuild going on
        // Is multitenancy a problem
    }
    // Here we must need to have a context set, this context should give me an info on
    // What is policy decision to supress the issue,
    // How much change is added
    // When we come in the next iteration, we should see if the changes are taken effect.
    // Once the issue is resolved , then the ctx should be cleared and state should be set to detect error
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

bool
QosPolicyManager::MonitorParams(qos_state_ctx* ctx)
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
            int temp_ret = IsQosCompromized(volId, ctx);
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
QosPolicyManager::UpdateOldParamVolumeState(int volId)
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
QosPolicyManager::UpdateOldParamEventState(BackendEvent event)
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
QosPolicyManager::FillFlushEventState(BackendEvent event)
{
    curEventState[event].currentData += eventParams[event].currentBW;
    curEventState[event].pendingCPUEvents = qosMgr->GetPendingEvents(event);
    curEventState[event].totalEvents = qosMgr->GetEventLog(event);
    curEventState[event].flushData.nvramStripes = qosMgr->GetUsedStripeCnt();
    /*
    if(usedStripe < curEventState[event].flushData.nvramStripes)
    {
        usedStripe = curEventState[event].flushData.nvramStripes;
    }
    if (maxCpu < curEventState[event].pendingCPUEvents)
    {
        maxCpu = curEventState[event].pendingCPUEvents;
    }
    */
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosPolicyManager::FillGCEventState(BackendEvent event)
{
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
QosPolicyManager::FillMetaRebuildEventState(BackendEvent event)
{
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
QosPolicyManager::FillUserRebuildEventState(BackendEvent event)
{
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
QosPolicyManager::FillMetaEventState(BackendEvent event)
{
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
QosPolicyManager::FillFrontEventState(BackendEvent event)
{
    static long counteravg1 = 0;
    static long term1 = 0;
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
    switch (event)
    {
        case BackendEvent_Flush:
            FillFlushEventState(event);
            break;
        case BackendEvent_GC:
            FillGCEventState(event);
            break;
        case BackendEvent_UserdataRebuild:
            FillUserRebuildEventState(event);
            break;
        case BackendEvent_MetadataRebuild:
            FillMetaRebuildEventState(event);
            break;
        case BackendEvent_MetaIO:
            FillMetaEventState(event);
            break;
        case BackendEvent_FrontendIO:
            FillFrontEventState(event);
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
QosPolicyManager::ResetVolumeState(int volId)
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
QosPolicyManager::ResetEventState(BackendEvent event)
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
QosPolicyManager::ApplyThrottling(uint64_t throttleValue, bool increase)
{
    for (list<int>::iterator it = highConsumerVolList.begin(); it != highConsumerVolList.end(); it++)
    {
        if (increase == true)
        {
            IncreaseThrottling(*it, throttleValue);
        }
        else
        {
            DecreaseThrottling(*it, throttleValue);
        }
    }
    for (list<int>::iterator it = intermediateConsumerVolList.begin(); it != intermediateConsumerVolList.end(); it++)
    {
        if (increase == true)
        {
            IncreaseThrottling(*it, throttleValue);
        }
        else
        {
            DecreaseThrottling(*it, throttleValue);
        }
    }

    for (list<int>::iterator it = lowConsumerVolList.begin(); it != lowConsumerVolList.end(); it++)
    {
        if (increase == true)
        {
            // IncreaseThrottling(*it, throttleValue);
        }
        else
        {
            DecreaseThrottling(*it, throttleValue);
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
QosPolicyManager::ApplyCorrection(qos_state_ctx* context)
{
    uint64_t perVolThrottle = 0;
    switch (context->causeType)
    {
        case QosCause_CPU:
            break;
        case QosCause_Disk:

        case QosCause_NVRAM:
            if (consumerCount != 0)
            {
                perVolThrottle = (10 * M_KBYTES * M_KBYTES / IBOF_QOS_TIMESLICE_IN_USEC);
                context->correctionValue = perVolThrottle;
                ApplyThrottling(perVolThrottle, true);
                IBOF_TRACE_INFO(QosReturnCode::INCREASE_BW_THROTTLING, "Increasing Volume BW Throttling");
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
QosPolicyManager::IncreaseThrottling(uint32_t volId, uint64_t correction)
{
    uint64_t volumeSpecific = 0;
    for (std::map<uint32_t, uint32_t>::iterator it = qosMgr->volReactorMap[volId].begin(); it != qosMgr->volReactorMap[volId].end(); ++it)
    {
        volumeSpecific += qosMgr->GetVolumeWeight(it->first, volId);
    }

    if (volumeSpecific >= qosMgr->maxVolWeight[volId])
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
    for (std::map<uint32_t, uint32_t>::iterator it = qosMgr->volReactorMap[volId].begin(); it != qosMgr->volReactorMap[volId].end(); ++it)
    {
        qosMgr->SetVolumeWeight(it->first, volId, (volumeSpecific * (it->second)) / qosMgr->totalConnection[volId]);
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
QosPolicyManager::DecreaseThrottling(int volId, uint64_t correction)
{
#if 0
    value = (qosMgr->GetVolumeWeight(volId)) + correction;
    if (value >  volPolicy[volId].maxBW)
    {
        value =  volPolicy[volId].maxBW;
    }
    qosMgr->SetVolumeWeight(volId, value);
#endif
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

bool
QosPolicyManager::ResetThrottling(qos_state_ctx* ctx)
{
    bool ret = false;
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
QosPolicyManager::MonitorCorrection(qos_state_ctx* ctx)
{
    bool ret = false;
    switch (ctx->causeType)
    {
        case QosCause_CPU:
        {
            if ((ctx->previousAvgBW >= avgVolBw[bufIndex][ctx->victimVol]))
            {
                if (curEventState[BackendEvent_UserdataRebuild].pendingCPUEvents > 0 && (curEventState[BackendEvent_Flush].pendingCPUEvents > 0))
                {
                    // Increase the weight of WRR
                    ret = false;
                }
            }
            else
            {
                if (avgVolBw[bufIndex][ctx->victimVol] >= GetMinBWPolicy(ctx->victimVol))
                {
                    ret = true;
                }
            }
            break;
        }
        case QosCause_Disk:
        case QosCause_NVRAM:
        {
            if (ctx->previousAvgBW >= avgVolBw[bufIndex][ctx->victimVol])
            {
                ApplyThrottling(ctx->correctionValue, true);
                IBOF_TRACE_INFO(QosReturnCode::INCREASE_BW_THROTTLING, "Increasing Volume BW Throttling");
                ret = false;
            }
            else
            {
                if (avgVolBw[bufIndex][ctx->victimVol] >= GetMinBWPolicy(ctx->victimVol))
                {
                    ret = true;
                }
            }
            break;
        }
        default:
            break;
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
QosPolicyManager::CheckSystem(uint64_t timeSpent)
{
    QosState state;
    state = GetCurrentState();
    qosCycle++;
    bool loopContinue = true;
    do
    {
        switch (state)
        {
            case QosState_IssueDetect:
            {
                if (false == AvgBwCompute(timeSpent))
                {
                    loopContinue = false;
                    break;
                }
                if (true == MonitorParams(&context))
                {
                    state = QosState_ApplyCorrection;
                    SetCurrentState(state);
                }
                else
                {
                    if ((qosCycle % (uint32_t)(0.4 * (M_QOS_CORRECTION_CYCLE))) == 0)
                    {
                        state = QosState_ResetThrottling;
                        SetCurrentState(state);
                    }
                    loopContinue = false;
                }
                break;
            }
            case QosState_ApplyCorrection:
            {
                ApplyCorrection(&context);
                state = QosState_IssueDetect;
                SetCurrentState(state);
                loopContinue = false;
                break;
            }
            case QosState_ResetThrottling:
            {
                AvgBwCompute(timeSpent);
                if (context.causeType == QosCause_None)
                {
                    ResetThrottling(&context);
                }
                state = QosState_IssueDetect;
                SetCurrentState(state);
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
    return volMinPolicyInEffect;
}

} // namespace ibofos

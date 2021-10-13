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

#include "src/qos/event_cpu_policy.h"
namespace pos
{
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
EventCpuPolicy::EventCpuPolicy(QosContext* qosCtx)
{
    qosContext = qosCtx;
    lastArrayState.Reset();
    changeCorrection = false;
    lastMimimumPolicy = false;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
EventCpuPolicy::~EventCpuPolicy(void)
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
EventCpuPolicy::HandlePolicy(void)
{
    // This code will be disabled temporary
    // QosResource& qosResource = qosContext->GetQosResource();
    // ResourceCpu& cpuState = qosResource.GetResourceCpu();
    // uint32_t rebuildPendingCpu = cpuState.GetEventPendingCpuCount(BackendEvent_UserdataRebuild);
    // if (rebuildPendingCpu > 0)
    // {
    //    _RebuildScenario();
    // }
    // else
    // {
    //    _NoRebuildScenario();
    //}
    _SetRebuildPolicyWeight();
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
EventCpuPolicy::_SetRebuildPolicyWeight(void)
{
    QosUserPolicy& userPolicy = qosContext->GetQosUserPolicy();
    RebuildUserPolicy& rebuildUserPolicy = userPolicy.GetRebuildUserPolicy();
    uint8_t priority = rebuildUserPolicy.GetRebuildImpact();
    // initialise with default priority weight(highest)
    QosCorrectionDir rebuildCorrection = QosCorrectionDir_PriorityHighest;
    switch (priority)
    {
        case PRIORITY_HIGHEST:
            rebuildCorrection = QosCorrectionDir_PriorityHighest;
            break;
        case PRIORITY_HIGHER:
            rebuildCorrection = QosCorrectionDir_PriorityHigher;
            break;
        case PRIORITY_HIGH:
            rebuildCorrection = QosCorrectionDir_PriorityHigh;
            break;
        case PRIORITY_MEDIUM:
            rebuildCorrection = QosCorrectionDir_PriorityMedium;
            break;
        case PRIORITY_LOW:
            rebuildCorrection = QosCorrectionDir_PriorityLow;
            break;
        case PRIORITY_LOWER:
            rebuildCorrection = QosCorrectionDir_PriorityLower;
            break;
        case PRIORITY_LOWEST:
            rebuildCorrection = QosCorrectionDir_PriorityLowest;
            break;
        default:
            break;
    }
    QosCorrection& qosCorrection = qosContext->GetQosCorrection();
    QosEventWrrWeight& qosEventWrr = qosCorrection.GetEventWrrWeightPolicy();
    // if previous set rebuild weigh is same as current weight, do nothing
    if (qosEventWrr.CorrectionType(BackendEvent_UserdataRebuild) == rebuildCorrection)
    {
        return;
    }
    qosEventWrr.SetCorrectionType(BackendEvent_UserdataRebuild, rebuildCorrection);
    qosContext->SetApplyCorrection(true);
    qosCorrection.SetCorrectionType(QosCorrection_EventWrr, true);
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
EventCpuPolicy::_CheckMinimum(QosCorrectionDir priority)
{
    QosResource& qosResource = qosContext->GetQosResource();
    QosGcState gcState = QosGcState_Start;
    ResourceArray arrayState;
    for (uint32_t i = 0; i < MAX_ARRAY_COUNT; i++)
    {
        ResourceArray& arrayStateTemp = qosResource.GetResourceArray(i);
        QosGcState gcStateArray = arrayStateTemp.GetGcThreshold();
        if (gcStateArray >= gcState)
        {
            gcState = gcStateArray;
            arrayState = arrayStateTemp;
        }
    }
    ResourceCpu& cpuState = qosResource.GetResourceCpu();
    gcState = arrayState.GetGcThreshold();

    if (gcState == QosGcState_Normal)
    {
        uint32_t flushPendingCpu = cpuState.GetEventPendingCpuCount(BackendEvent_Flush);
        QosCorrection& qosCorrection = qosContext->GetQosCorrection();
        QosEventWrrWeight& qosEventWrr = qosCorrection.GetEventWrrWeightPolicy();
        if (flushPendingCpu > 0)
        {
            qosEventWrr.SetCorrectionType(BackendEvent_UserdataRebuild, QosCorrectionDir_Increase);
            qosEventWrr.SetCorrectionType(BackendEvent_MetadataRebuild, QosCorrectionDir_Increase);
            qosEventWrr.SetCorrectionType(BackendEvent_Flush, QosCorrectionDir_Decrease);
        }
        else
        {
            qosEventWrr.SetCorrectionType(BackendEvent_UserdataRebuild, QosCorrectionDir_PriorityLow);
            qosEventWrr.SetCorrectionType(BackendEvent_MetadataRebuild, QosCorrectionDir_PriorityLow);
            qosEventWrr.SetCorrectionType(BackendEvent_Flush, QosCorrectionDir_Reset);
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
EventCpuPolicy::_ManageEventCorrectionOnGcState(QosCorrectionDir priority)
{
    QosResource& qosResource = qosContext->GetQosResource();
    QosGcState gcState = QosGcState_Start;
    ResourceArray arrayState;
    for (uint32_t i = 0; i < MAX_ARRAY_COUNT; i++)
    {
        ResourceArray& arrayStateTemp = qosResource.GetResourceArray(i);
        QosGcState gcStateArray = arrayStateTemp.GetGcThreshold();
        if (gcStateArray >= gcState)
        {
            gcState = gcStateArray;
            arrayState = arrayStateTemp;
        }
    }
    ResourceArray& prevArrayState = lastArrayState;
    gcState = QosGcState_Unknown;
    gcState = arrayState.GetGcThreshold();
    uint32_t currFreeSegment = arrayState.GetGcFreeSegment();
    uint32_t lastFreeSegment = prevArrayState.GetGcFreeSegment();
    QosCorrection& qosCorrection = qosContext->GetQosCorrection();
    QosEventWrrWeight& qosEventWrr = qosCorrection.GetEventWrrWeightPolicy();
    switch (gcState)
    {
        case QosGcState_Normal:
            if (qosEventWrr.CorrectionType(BackendEvent_UserdataRebuild) == priority)
            {
                return;
            }
            qosEventWrr.SetCorrectionType(BackendEvent_UserdataRebuild, priority);
            qosEventWrr.SetCorrectionType(BackendEvent_MetadataRebuild, priority);
            qosEventWrr.SetCorrectionType(BackendEvent_GC, QosCorrectionDir_Reset);
            break;
        case QosGcState_Medium:
        case QosGcState_High:
        case QosGcState_Critical:
            if (currFreeSegment <= lastFreeSegment)
            {
                qosEventWrr.SetCorrectionType(BackendEvent_GC, QosCorrectionDir_Reset);
                qosEventWrr.SetCorrectionType(BackendEvent_UserdataRebuild, QosCorrectionDir_Increase);
                qosEventWrr.SetCorrectionType(BackendEvent_MetadataRebuild, QosCorrectionDir_Increase);
            }
            break;
        default:
            assert(1);
            break;
    }
    qosContext->SetApplyCorrection(true);
    qosCorrection.SetCorrectionType(QosCorrection_EventWrr, true);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
EventCpuPolicy::_HighPriorityRebuildScenario(void)
{
    _ManageEventCorrectionOnGcState(QosCorrectionDir_PriorityHigh);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
EventCpuPolicy::_LowPriorityRebuildScenario(void)
{
    _ManageEventCorrectionOnGcState(QosCorrectionDir_PriorityLow);
    _CheckMinimum(QosCorrectionDir_PriorityLow);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
EventCpuPolicy::_RebuildScenario(void)
{
    QosUserPolicy& userPolicy = qosContext->GetQosUserPolicy();
    RebuildUserPolicy& rebuildUserPolicy = userPolicy.GetRebuildUserPolicy();
    uint8_t priority = rebuildUserPolicy.GetRebuildImpact();
    switch (priority)
    {
        case PRIORITY_HIGH:
            _HighPriorityRebuildScenario();
            break;
        case PRIORITY_LOW:
            _LowPriorityRebuildScenario();
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
EventCpuPolicy::_NoRebuildScenario(void)
{
    QosResource& qosResource = qosContext->GetQosResource();
    QosGcState gcState = QosGcState_Start;
    ResourceArray arrayState;
    for (uint32_t i = 0; i < MAX_ARRAY_COUNT; i++)
    {
        ResourceArray& arrayStateTemp = qosResource.GetResourceArray(i);
        QosGcState gcStateArray = arrayStateTemp.GetGcThreshold();
        if (gcStateArray >= gcState)
        {
            gcState = gcStateArray;
            arrayState = arrayStateTemp;
        }
    }
    ResourceArray& prevArrayState = lastArrayState;
    gcState = QosGcState_Unknown;
    QosGcState prevGcState = QosGcState_Unknown;
    gcState = arrayState.GetGcThreshold();
    prevGcState = prevArrayState.GetGcThreshold();
    uint32_t currFreeSegment = arrayState.GetGcFreeSegment();
    uint32_t lastFreeSegment = prevArrayState.GetGcFreeSegment();
    QosCorrection& qosCorrection = qosContext->GetQosCorrection();
    QosEventWrrWeight& qosEventWrr = qosCorrection.GetEventWrrWeightPolicy();
    if ((gcState == QosGcState_Normal) && (prevGcState == QosGcState_Normal))
    {
        return;
    }
    switch (gcState)
    {
        case QosGcState_Normal:
            qosEventWrr.SetCorrectionType(BackendEvent_GC, QosCorrectionDir_Reset);
            qosEventWrr.SetCorrectionType(BackendEvent_Flush, QosCorrectionDir_Reset);
            qosEventWrr.SetCorrectionType(BackendEvent_UserdataRebuild, QosCorrectionDir_Reset);
            qosEventWrr.SetCorrectionType(BackendEvent_MetadataRebuild, QosCorrectionDir_Reset);
            qosContext->SetApplyCorrection(true);
            qosCorrection.SetCorrectionType(QosCorrection_EventWrr, true);
            break;
        case QosGcState_Medium:
            if (currFreeSegment < (lastFreeSegment))
            {
                qosEventWrr.SetCorrectionType(BackendEvent_GC, QosCorrectionDir_Decrease);
                qosEventWrr.SetCorrectionType(BackendEvent_Flush, QosCorrectionDir_Increase);
                qosContext->SetApplyCorrection(true);
                qosCorrection.SetCorrectionType(QosCorrection_EventWrr, true);
            }
            break;
        case QosGcState_High:
            if (currFreeSegment < (lastFreeSegment))
            {
                qosEventWrr.SetCorrectionType(BackendEvent_GC, QosCorrectionDir_Decrease2X);
                qosEventWrr.SetCorrectionType(BackendEvent_Flush, QosCorrectionDir_Increase2X);
                qosContext->SetApplyCorrection(true);
                qosCorrection.SetCorrectionType(QosCorrection_EventWrr, true);
            }
            break;
        case QosGcState_Critical:
            if (currFreeSegment < (lastFreeSegment))
            {
                qosEventWrr.SetCorrectionType(BackendEvent_GC, QosCorrectionDir_Decrease4X);
                qosEventWrr.SetCorrectionType(BackendEvent_Flush, QosCorrectionDir_Increase4X);
                qosContext->SetApplyCorrection(true);
                qosCorrection.SetCorrectionType(QosCorrection_EventWrr, true);
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
EventCpuPolicy::_StoreContext(void)
{
    QosResource& qosResource = qosContext->GetQosResource();
    QosGcState gcState = QosGcState_Start;
    ResourceArray arrayState;
    for (uint32_t i = 0; i < MAX_ARRAY_COUNT; i++)
    {
        ResourceArray& arrayStateTemp = qosResource.GetResourceArray(i);
        QosGcState gcStateArray = arrayStateTemp.GetGcThreshold();
        if (gcStateArray >= gcState)
        {
            gcState = gcStateArray;
            arrayState = arrayStateTemp;
        }
    }
    lastArrayState = arrayState;
    lastMimimumPolicy = qosContext->IsVolumeMinPolicyInEffect();
}
} // namespace pos

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

#include "src/qos/qos_manager.h"

#include <cmath>
#include <string>

#include "src/bio/ubio.h"
#include "src/bio/volume_io.h"
#include "src/cpu_affinity/affinity_manager.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/io/frontend_io/aio.h"
#include "src/io_scheduler/io_worker.h"
#include "src/logger/logger.h"
#include "src/master_context/config_manager.h"
#include "src/network/nvmf_volume_pos.h"
#include "src/qos/context_factory.h"
#include "src/qos/internal_manager.h"
#include "src/qos/internal_manager_factory.h"
#include "src/qos/qos_context.h"
#include "src/qos/qos_event_manager.h"
#include "src/qos/qos_spdk_manager.h"
#include "src/qos/qos_volume_manager.h"
#include "src/spdk_wrapper/connection_management.h"
#include "src/spdk_wrapper/event_framework_api.h"

namespace pos
{
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosManager::QosManager(void)
{
    qosThread = nullptr;
    feQosEnabled = false;
    pollerTime = UINT32_MAX;
    arrayNameMap.clear();
    arrayIdMap.clear();

    for (uint32_t event = 0; (BackendEvent)event < BackendEvent_Count; event++)
    {
        pendingEvents[event] = M_RESET_TO_ZERO;
        eventLog[event] = M_RESET_TO_ZERO;
        oldLog[event] = M_RESET_TO_ZERO;
    }

    ConfigManager& configManager = *ConfigManagerSingleton::Instance();
    bool enabled = false;
    int ret = configManager.GetValue("fe_qos", "enable", &enabled,
        CONFIG_TYPE_BOOL);
    if (ret == (int)POS_EVENT_ID::SUCCESS)
    {
        feQosEnabled = enabled;
    }

    SpdkConnection::SetQosInSpdk(feQosEnabled);
    initialized = false;
    try
    {
        qosEventManager = new QosEventManager;
        qosContext = ContextFactory::CreateQosContext();
        for (uint32_t i = 0; i < MAX_ARRAY_COUNT; i++)
        {
            qosArrayManager[i] = new QosArrayManager(i, qosContext);
        }
        spdkManager = new QosSpdkManager(qosContext, feQosEnabled);
        monitoringManager = InternalManagerFactory::CreateInternalManager(QosInternalManager_Monitor, qosContext);
        policyManager = InternalManagerFactory::CreateInternalManager(QosInternalManager_Policy, qosContext);
        processingManager = InternalManagerFactory::CreateInternalManager(QosInternalManager_Processing, qosContext);
        correctionManager = InternalManagerFactory::CreateInternalManager(QosInternalManager_Correction, qosContext);
    }
    catch (bad_alloc& ex)
    {
        assert(0);
    }
    currentNumberOfArrays = 0;
    systemMinPolicy = false;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosManager::~QosManager(void)
{
    _Finalize();
    initialized = false;
    delete qosThread;
    delete spdkManager;
    delete qosEventManager;
    delete monitoringManager;
    delete policyManager;
    delete processingManager;
    delete correctionManager;
    delete qosContext;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::Initialize(void)
{
    if (true == initialized)
    {
        return;
    }
    if (true == feQosEnabled)
    {
        spdkManager->Initialize();
    }

    AffinityManager* affinityManager = AffinityManagerSingleton::Instance();
    cpuSet = affinityManager->GetCpuSet(CoreType::QOS);
    qosThread = new std::thread(&QosManager::_QosWorker, this);
    initialized = true;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bool
QosManager::IsFeQosEnabled(void)
{
    return feQosEnabled;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::_Finalize(void)
{
    if (true == feQosEnabled)
    {
        spdkManager->Finalize();
    }
    POS_TRACE_INFO(POS_EVENT_ID::QOS_FINALIZATION, "QosSpdkManager Finalization complete");
    SetExitQos();
    for (uint32_t i = 0; i < MAX_ARRAY_COUNT; i++)
    {
        qosArrayManager[i]->SetExitQos();
    }
    qosEventManager->SetExitQos();
    monitoringManager->SetExitQos();
    policyManager->SetExitQos();
    processingManager->SetExitQos();
    correctionManager->SetExitQos();
    if (nullptr != qosThread)
    {
        qosThread->join();
    }
    POS_TRACE_INFO(POS_EVENT_ID::QOS_FINALIZATION, "QosManager Finalization complete");
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::HandlePosIoSubmission(IbofIoSubmissionAdapter* aioSubmission, pos_io* volIo)
{
    std::string arrayName(volIo->arrayName);
    uint32_t arrayId = arrayNameMap[arrayName];
    qosArrayManager[arrayId]->HandlePosIoSubmission(aioSubmission, volIo);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::HandleEventUbioSubmission(SubmissionAdapter* ioSubmission,
    SubmissionNotifier* submissionNotifier, uint32_t id, UbioSmartPtr ubio)
{
    qosEventManager->HandleEventUbioSubmission(ioSubmission,
        submissionNotifier,
        id,
        ubio);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::_QosWorker(void)
{
    sched_setaffinity(0, sizeof(cpuSet), &cpuSet);
    pthread_setname_np(pthread_self(), "QoSWorker");
    QosInternalManager* currentManager = monitoringManager;
    QosInternalManagerType nextManagerType = QosInternalManager_Unknown;
    QosInternalManager* nextManager = nullptr;
    while (nullptr != currentManager)
    {
        if (true == IsExitQosSet())
        {
            POS_TRACE_INFO(POS_EVENT_ID::QOS_FINALIZATION, "QosManager Finalization Triggered, QosWorker thread exit");
            break;
        }
        currentManager->Execute();
        nextManagerType = currentManager->GetNextManagerType();
        nextManager = _GetNextInternalManager(nextManagerType);
        currentManager = nextManager;
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosInternalManager*
QosManager::_GetNextInternalManager(QosInternalManagerType internalManagerType)
{
    QosInternalManager* internalManager = nullptr;
    switch (internalManagerType)
    {
        case QosInternalManager_Monitor:
            internalManager = monitoringManager;
            break;

        case QosInternalManager_Policy:
            internalManager = policyManager;
            break;

        case QosInternalManager_Processing:
            internalManager = processingManager;
            break;

        case QosInternalManager_Correction:
            internalManager = correctionManager;
            break;

        default:
            break;
    }
    return internalManager;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bw_iops_parameter
QosManager::DequeueVolumeParams(uint32_t reactor, uint32_t volId, uint32_t arrayId)
{
    return qosArrayManager[arrayId]->DequeueVolumeParams(reactor, volId);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bw_iops_parameter
QosManager::DequeueEventParams(uint32_t workerId, BackendEvent event)
{
    return qosEventManager->DequeueParams(workerId, event);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::SetEventWeightWRR(BackendEvent eventId, int64_t weight)
{
    qosEventManager->SetEventWeightWRR(eventId, weight);
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int64_t
QosManager::GetDefaultEventWeightWRR(BackendEvent eventId)
{
    return qosEventManager->GetDefaultEventWeightWRR(eventId);
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int64_t
QosManager::GetEventWeightWRR(BackendEvent eventId)
{
    return qosEventManager->GetEventWeightWRR(eventId);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::IncreaseUsedStripeCnt(uint32_t arrayId)
{
    qosArrayManager[arrayId]->IncreaseUsedStripeCnt();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::DecreaseUsedStripeCnt(std::string arrayName)
{
    uint32_t arrayId = arrayNameMap[arrayName];
    qosArrayManager[arrayId]->DecreaseUsedStripeCnt();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
uint32_t
QosManager::GetUsedStripeCnt(uint32_t arrayId)
{
    return qosArrayManager[arrayId]->GetUsedStripeCnt();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
std::vector<int>
QosManager::GetVolumeFromActiveSubsystem(uint32_t nqnId, uint32_t arrayId)
{
    return qosArrayManager[arrayId]->GetVolumeFromActiveSubsystem(nqnId);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::UpdateSubsystemToVolumeMap(uint32_t nqnId, uint32_t volId, std::string arrayName)
{
    uint32_t arrayId = GetArrayIdFromMap(arrayName);
    qosArrayManager[arrayId]->UpdateSubsystemToVolumeMap(nqnId, volId);
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::DeleteVolumeFromSubsystemMap(uint32_t nqnId, uint32_t volId, std::string arrayName)
{
    uint32_t arrayId = GetArrayIdFromMap(arrayName);
    qosArrayManager[arrayId]->DeleteVolumeFromSubsystemMap(nqnId, volId);
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int
QosManager::VolumeQosPoller(poller_structure* param, IbofIoSubmissionAdapter* aioSubmission)
{
    if (true == feQosEnabled)
    {
        uint64_t now = SpdkConnection::SpdkGetTicks();
        uint32_t reactor = param->id;
        uint64_t next_tick = param->nextTimeStamp;
        if (now < next_tick)
        {
            return 0;
        }
        double offset = (double)(now - next_tick) / param->qosTimeSlice;
        offset = offset + 1.0;
        for (uint32_t i = 0; i < MAX_ARRAY_COUNT; i++)
        {
            qosArrayManager[i]->VolumeQosPoller(reactor, aioSubmission, offset);
        }
        qosContext->SetReactorProcessed(reactor, true);
        now = SpdkConnection::SpdkGetTicks();
        param->nextTimeStamp = now + param->qosTimeSlice;
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
int
QosManager::IOWorkerPoller(uint32_t id, SubmissionAdapter* ioSubmission)
{
    return qosEventManager->IOWorkerPoller(id, ioSubmission);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

bool
QosManager::IsMinimumPolicyInEffectInSystem(void)
{
    return systemMinPolicy;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::ResetCorrection(void)
{
    correctionManager->Reset();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int
QosManager::UpdateVolumePolicy(uint32_t volId, qos_vol_policy policy, uint32_t arrayId)
{
    if (systemMinPolicy == false)
    {
        systemMinPolicy = ((true == policy.minBwGuarantee) || (true == policy.minIopsGuarantee));
    }
    qosArrayManager[arrayId]->UpdateVolumePolicy(volId, policy);
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
QosManager::GetVolumePolicy(uint32_t volId, std::string arrayName)
{
    std::unique_lock<std::mutex> uniqueLock(policyUpdateLock);
    uint32_t arrayId = arrayNameMap[arrayName];
    return qosArrayManager[arrayId]->GetVolumePolicy(volId);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
uint32_t
QosManager::GetPendingEvents(BackendEvent event)
{
    return pendingEvents[event];
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::DecreasePendingEvents(BackendEvent event)
{
    pendingEvents[event]--;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::IncreasePendingEvents(BackendEvent event)
{
    pendingEvents[event]++;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::LogEvent(BackendEvent event)
{
    eventLog[event]++;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
uint32_t
QosManager::GetEventLog(BackendEvent event)
{
    uint32_t count = eventLog[event];
    uint32_t oldCount = oldLog[event];
    oldLog[event] = count;
    if (oldCount > count)
    {
        return (count + (M_UINT32MAX - oldCount));
    }
    else
    {
        return (count - oldCount);
    }
}

/* --------------------------------------------------------------------------*/
/**
  * @Synopsis
  *
  * @Returns
  */
/* --------------------------------------------------------------------------*/
qos_rebuild_policy
QosManager::GetRebuildPolicy(std::string arrayName)
{
    std::unique_lock<std::mutex> uniqueLock(policyUpdateLock);
    uint32_t arrayId = arrayNameMap[arrayName];
    return qosArrayManager[arrayId]->GetRebuildPolicy();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int
QosManager::UpdateRebuildPolicy(qos_rebuild_policy rebuildPolicy)
{
    for (uint32_t i = 0; i < MAX_ARRAY_COUNT; i++)
    {
        std::unique_lock<std::mutex> uniqueLock(policyUpdateLock);
        qosArrayManager[i]->UpdateRebuildPolicy(rebuildPolicy);
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
void
QosManager::SetVolumeLimit(uint32_t reactor, uint32_t volId, int64_t weight, bool iops, uint32_t arrayId)
{
    qosArrayManager[arrayId]->SetVolumeLimit(reactor, volId, weight, iops);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int64_t
QosManager::GetVolumeLimit(uint32_t reactor, uint32_t volId, bool iops, uint32_t arrayId)
{
    return qosArrayManager[arrayId]->GetVolumeLimit(reactor, volId, iops);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bool
QosManager::IsVolumePolicyUpdated(uint32_t arrayId)
{
    return qosArrayManager[arrayId]->IsVolumePolicyUpdated();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::SetGcFreeSegment(uint32_t freeSegments, uint32_t arrayId)
{
    qosArrayManager[arrayId]->SetGcFreeSegment(freeSegments);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
uint32_t
QosManager::GetGcFreeSegment(uint32_t arrayId)
{
    return qosArrayManager[arrayId]->GetGcFreeSegment();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::GetVolumePolicyMap(uint32_t arrayId, std::map<uint32_t, qos_vol_policy>& volumePolicyMapCopy)
{
    qosArrayManager[arrayId]->GetVolumePolicyMap(volumePolicyMapCopy);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
uint32_t
QosManager::GetArrayIdFromMap(std::string arrayName)
{
    return arrayNameMap[arrayName];
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
std::string
QosManager::GetArrayNameFromMap(uint32_t arrayId)
{
    return arrayIdMap[arrayId];
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
uint32_t
QosManager::GetNumberOfArrays(void)
{
    return currentNumberOfArrays;
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::UpdateArrayMap(std::string arrayName)
{
    if (arrayNameMap.find(arrayName) != arrayNameMap.end())
    {
        return;
    }
    else
    {
        mapUpdateLock.lock();
        arrayNameMap.insert({arrayName, currentNumberOfArrays});
        qosArrayManager[currentNumberOfArrays]->SetArrayName(arrayName);
        arrayIdMap.insert({currentNumberOfArrays, arrayName});
        mapUpdateLock.unlock();
        currentNumberOfArrays++;
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
QosManager::GetSubsystemVolumeMap(std::unordered_map<int32_t, std::vector<int>>& subsysVolMap, uint32_t arrayId)
{
    qosArrayManager[arrayId]->GetSubsystemVolumeMap(subsysVolMap);
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
uint32_t
QosManager::GetNoContentionCycles(void)
{
    return NO_CONTENTION_CYCLES;
}
} // namespace pos

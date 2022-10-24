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

#include "src/qos/qos_manager.h"

#include <cmath>
#include <string>

#include "src/bio/ubio.h"
#include "src/bio/volume_io.h"
#include "src/cpu_affinity/affinity_manager.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/io/frontend_io/aio.h"
#include "src/io_scheduler/io_dispatcher_submission.h"
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
#include "src/spdk_wrapper/event_framework_api.h"

namespace pos
{
bool QosManager::needThrottling;
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

QosManager::QosManager(SpdkEnvCaller* spdkEnvCaller,
        SpdkPosNvmfCaller* spdkPosNvmfCaller,
        ConfigManager* configManager,
        EventFrameworkApi* eventFrameworkApi,
        AffinityManager* affinityManager)
    : spdkEnvCaller(spdkEnvCaller),
      spdkPosNvmfCaller(spdkPosNvmfCaller),
      configManager(configManager),
      eventFrameworkApi(eventFrameworkApi),
      affinityManager(affinityManager)
{
    qosThread = nullptr;
    feQosEnabled = false;
    pollerTime = UINT32_MAX;
    arrayNameMap.clear();
    arrayIdMap.clear();

    for (uint32_t event = 0; (BackendEvent)event < BackendEvent_Count; event++)
    {
        backendPolicyCli[event].priorityImpact = PRIORITY_DEFAULT;
        pendingBackendEvents[event] = M_RESET_TO_ZERO;
        eventLog[event] = M_RESET_TO_ZERO;
        oldLog[event] = M_RESET_TO_ZERO;
    }
    bool enabled = false;
    int ret = configManager->GetValue("fe_qos", "enable", &enabled, CONFIG_TYPE_BOOL);
    if (ret == EID(SUCCESS))
    {
        feQosEnabled = enabled;
    }

    spdkPosNvmfCaller->SetQosInSpdk(feQosEnabled);

    initialized = false;
    qosEventManager = new QosEventManager;
    qosContext = ContextFactory::CreateQosContext();
    for (uint32_t i = 0; i < MAX_ARRAY_COUNT; i++)
    {
        qosArrayManager[i] = new QosArrayManager(i, qosContext, feQosEnabled, eventFrameworkApi, this);
    }

    monitoringManager = InternalManagerFactory::CreateInternalManager(QosInternalManager_Monitor, qosContext, this);
    policyManager = InternalManagerFactory::CreateInternalManager(QosInternalManager_Policy, qosContext, this);
    correctionManager = InternalManagerFactory::CreateInternalManager(QosInternalManager_Correction, qosContext, this);

    for (uint32_t reactor = 0; reactor < M_MAX_REACTORS; reactor++)
    {
        previousDelay[reactor] = 0;
    }

    currentNumberOfArrays = 0;
    systemMinPolicy = false;
    affinityManager = AffinityManagerSingleton::Instance();
    spdkManager = nullptr;
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
    for (uint32_t i = 0; i < MAX_ARRAY_COUNT; i++)
    {
        delete qosArrayManager[i];
    }
    delete qosThread;
    delete qosEventManager;
    delete monitoringManager;
    delete policyManager;
    delete correctionManager;
    delete qosContext;
    if (spdkEnvCaller != nullptr)
    {
        delete spdkEnvCaller;
    }
    if (spdkPosNvmfCaller != nullptr)
    {
        delete spdkPosNvmfCaller;
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
QosManager::InitializeSpdkManager(void)
{
    spdkManager = new QosSpdkManager(qosContext, feQosEnabled, eventFrameworkApi);
    if (true == feQosEnabled)
    {
        spdkManager->Initialize();
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
QosManager::Initialize(void)
{
    if (true == initialized)
    {
        return;
    }
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
QosManager::FinalizeSpdkManager(void)
{
    if (true == feQosEnabled)
    {
        spdkManager->Finalize();
    }
    POS_TRACE_INFO(EID(QOS_FINALIZATION), "QosSpdkManager Finalization complete");
    delete spdkManager;
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
    SetExitQos();
    for (uint32_t i = 0; i < MAX_ARRAY_COUNT; i++)
    {
        qosArrayManager[i]->SetExitQos();
    }
    qosEventManager->SetExitQos();
    monitoringManager->SetExitQos();
    policyManager->SetExitQos();
    correctionManager->SetExitQos();
    if (nullptr != qosThread)
    {
        qosThread->join();
    }
    POS_TRACE_INFO(EID(QOS_FINALIZATION), "QosManager Finalization complete");
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::HandlePosIoSubmission(IbofIoSubmissionAdapter* aioSubmission, VolumeIoSmartPtr volIo)
{
    qosArrayManager[volIo->GetArrayId()]->HandlePosIoSubmission(aioSubmission, volIo);
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
QosManager::PeriodicalJob(uint64_t* nextTick)
{
    uint64_t now = spdkEnvCaller->SpdkGetTicks();
    // We can check overlap case.
    uint64_t tickDiff = (now - *nextTick);
    if ((int64_t)tickDiff > 0)
    {
        for (uint32_t arrayId = 0; arrayId < MAX_ARRAY_COUNT; arrayId++)
        {
            qosArrayManager[arrayId]->ResetVolumeThrottling();
        }
        if (*nextTick == 0)
        {
            *nextTick = now;
        }
        *nextTick = *nextTick + IBOF_QOS_TIMESLICE_IN_USEC * spdkEnvCaller->SpdkGetTicksHz() / SPDK_SEC_TO_USEC;
        IODispatcherSubmissionSingleton::Instance()->CheckAndSetBusyMode();
        IODispatcherSubmissionSingleton::Instance()->RefillRemaining(SPDK_SEC_TO_USEC
            / IBOF_QOS_TIMESLICE_IN_USEC);
        _ControlThrottling();
    }
}

void
QosManager::_ControlThrottling(void)
{
    ResetGlobalThrottling();

    uint64_t totalMinVolumeBw = 0, totalMinVolumeIops = 0;
    QosUserPolicy& qosUserPolicy = qosContext->GetQosUserPolicy();
    AllVolumeUserPolicy& allVolUserPolicy = qosUserPolicy.GetAllVolumeUserPolicy();
    std::vector<std::pair<uint32_t, uint32_t>> minVols = allVolUserPolicy.GetMinimumGuaranteeVolume();
    if (minVols.empty())
    {
        needThrottling = false;
    }
    else
    {
        needThrottling = true;
    }
    for (auto iter : minVols)
    {
        uint32_t arrayId = iter.first;
        uint32_t minVolId = iter.second;
        VolumeUserPolicy* volumeUserPolicy = allVolUserPolicy.GetVolumeUserPolicy(arrayId, minVolId);
        if (volumeUserPolicy->IsMinimumVolume() == false)
        {
            SetMinimumVolume(arrayId, minVolId, 0, false);
            SetMinimumVolume(arrayId, minVolId, 0, true);
            continue;
        }
        bool flag = (volumeUserPolicy->IsBwPolicySet() == false);
        if (flag == true)
        {
            totalMinVolumeIops += qosArrayManager[arrayId]->GetDynamicVolumeThrottling(minVolId, flag);
            SetMinimumVolume(arrayId, minVolId, volumeUserPolicy->GetMinIops(), flag);
        }
        else
        {
            totalMinVolumeBw += qosArrayManager[arrayId]->GetDynamicVolumeThrottling(minVolId, flag);
            SetMinimumVolume(arrayId, minVolId, volumeUserPolicy->GetMinBandwidth(), flag);
        }
    }
    uint64_t globalBw = QosVolumeManager::GetGlobalThrottling(false);
    uint64_t globalIops = QosVolumeManager::GetGlobalThrottling(true);
    QosVolumeManager::SetRemainingThrottling(globalBw, totalMinVolumeBw, false);
    QosVolumeManager::SetRemainingThrottling(globalIops, totalMinVolumeIops, true);
}

void
QosManager::_QosWorker(void)
{
    sched_setaffinity(0, sizeof(cpuSet), &cpuSet);
    pthread_setname_np(pthread_self(), "QoSWorker");
    QosInternalManager* currentManager = monitoringManager;
    QosInternalManagerType nextManagerType = QosInternalManager_Unknown;
    QosInternalManager* nextManager = nullptr;
    uint64_t nextTick = 0;
    while (nullptr != currentManager)
    {
        if (true == IsExitQosSet())
        {
            POS_TRACE_INFO(EID(QOS_FINALIZATION), "QosManager Finalization Triggered, QosWorker thread exit");
            break;
        }
        currentManager->Execute();
        nextManagerType = currentManager->GetNextManagerType();
        nextManager = _GetNextInternalManager(nextManagerType);
        currentManager = nextManager;

        PeriodicalJob(&nextTick);
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
    std::lock_guard<std::mutex> lock(mapUpdateLock);
    if (arrayNameMap.size() != 0)
    {
        if (arrayNameMap.find(arrayName) != arrayNameMap.end())
        {
            uint32_t arrayId = arrayNameMap[arrayName];
            qosArrayManager[arrayId]->DecreaseUsedStripeCnt();
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
void
QosManager::ResetGlobalThrottling(void)
{
    QosVolumeManager::ResetGlobalThrottling();
}

void
QosManager::SetMinimumVolume(uint32_t arrayId, uint32_t volId, uint64_t value, bool iops)
{
    qosArrayManager[arrayId]->SetMinimumVolume(volId, value, iops);
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
    int32_t arrayId = GetArrayIdFromMap(arrayName);
    if (arrayId != -1)
    {
        qosArrayManager[arrayId]->UpdateSubsystemToVolumeMap(nqnId, volId);
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
QosManager::DeleteVolumeFromSubsystemMap(uint32_t nqnId, uint32_t volId, std::string arrayName)
{
    int32_t arrayId = GetArrayIdFromMap(arrayName);
    if (arrayId != -1)
    {
        qosArrayManager[arrayId]->DeleteVolumeFromSubsystemMap(nqnId, volId);
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
QosManager::VolumeQosPoller(poller_structure* param, IbofIoSubmissionAdapter* aioSubmission)
{
    if (true == feQosEnabled)
    {
        uint64_t now = spdkEnvCaller->SpdkGetTicks();
        uint32_t reactor = param->id;
        uint64_t next_tick = param->nextTimeStamp;
        if (now < next_tick)
        {
            return 0;
        }
        double offset = (double)(now - next_tick + previousDelay[reactor]) / param->qosTimeSlice;
        offset = offset + 1.0;
        for (uint32_t i = 0; i < MAX_ARRAY_COUNT; i++)
        {
            qosArrayManager[i]->VolumeQosPoller(aioSubmission, offset);
        }
        qosContext->SetReactorProcessed(reactor, true);
        uint64_t after = spdkEnvCaller->SpdkGetTicks();
        previousDelay[reactor] = after - now;
        param->nextTimeStamp = after + param->qosTimeSlice / POLLING_FREQ_PER_QOS_SLICE;
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
    return qosArrayManager[arrayId]->UpdateVolumePolicy(volId, policy);
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
    {
        std::lock_guard<std::mutex> lock(mapUpdateLock);
        if (arrayNameMap.size() != 0)
        {
            if (arrayNameMap.find(arrayName) != arrayNameMap.end())
            {
                std::unique_lock<std::mutex> uniqueLock(policyUpdateLock);
                uint32_t arrayId = arrayNameMap[arrayName];
                return qosArrayManager[arrayId]->GetVolumePolicy(volId);
            }
        }
    }
    qos_vol_policy qosVolumePolicyDefault;
    return qosVolumePolicyDefault;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
uint32_t
QosManager::GetPendingBackendEvents(BackendEvent event)
{
    return pendingBackendEvents[event];
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::DecreasePendingBackendEvents(BackendEvent event)
{
    if (feQosEnabled)
    {
        pendingBackendEvents[event]--;
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
QosManager::IncreasePendingBackendEvents(BackendEvent event)
{
    if (feQosEnabled)
    {
        pendingBackendEvents[event]++;
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
qos_backend_policy
QosManager::GetBackendPolicy(BackendEvent eventType)
{
    std::unique_lock<std::mutex> uniqueLock(policyUpdateLock);
    return backendPolicyCli[eventType];
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int
QosManager::UpdateBackendPolicy(BackendEvent eventType, qos_backend_policy backendPolicy)
{
    std::unique_lock<std::mutex> uniqueLock(policyUpdateLock);
    backendPolicyCli[eventType] = backendPolicy;
    if (eventType == BackendEvent_UserdataRebuild && backendPolicyCli[BackendEvent_UserdataRebuild].priorityImpact == PRIORITY_LOW)
    {
        qos_backend_policy policy;
        policy.priorityImpact = PRIORITY_HIGH;
        backendPolicyCli[BackendEvent_FrontendIO] = policy;
    }
    else if (eventType == BackendEvent_UserdataRebuild && backendPolicyCli[BackendEvent_UserdataRebuild].priorityImpact == PRIORITY_HIGH)
    {
        qos_backend_policy policy;
        policy.priorityImpact = PRIORITY_LOW;
        backendPolicyCli[BackendEvent_FrontendIO] = policy;
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
QosManager::SetVolumeLimit(uint32_t volId, int64_t weight, bool iops, uint32_t arrayId)
{
    qosArrayManager[arrayId]->SetVolumeLimit(volId, weight, iops);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int64_t
QosManager::GetVolumeLimit(uint32_t volId, bool iops, uint32_t arrayId)
{
    return qosArrayManager[arrayId]->GetVolumeLimit(volId, iops);
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
QosManager::GetMountedVolumes(std::list<std::pair<uint32_t, uint32_t>>& volumeList)
{
    for (uint32_t arrayId = 0; arrayId < MAX_ARRAY_COUNT; arrayId++)
    {
        std::list<uint32_t> tempVolumeList;
        tempVolumeList.clear();
        qosArrayManager[arrayId]->GetMountedVolumes(tempVolumeList);
        for (auto iter : tempVolumeList)
        {
           volumeList.push_back(std::make_pair(arrayId, iter));
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
int32_t
QosManager::GetArrayIdFromMap(std::string arrayName)
{
    std::lock_guard<std::mutex> lock(mapUpdateLock);
    if (arrayNameMap.size() != 0)
    {
        if (arrayNameMap.find(arrayName) != arrayNameMap.end())
        {
            return arrayNameMap[arrayName];
        }
    }
    return -1;
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
    std::string retName = "";
    if (arrayIdMap.find(arrayId) != arrayIdMap.end())
    {
        retName = arrayIdMap[arrayId];
    }

    return retName;
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
    std::lock_guard<std::mutex> lock(mapUpdateLock);
    if (arrayNameMap.find(arrayName) != arrayNameMap.end())
    {
        return;
    }
    if (currentNumberOfArrays >= MAX_ARRAY_COUNT)
    {
        POS_TRACE_WARN(static_cast<int>(EID(QOS_MAX_ARRAYS_EXCEEDED)), "Trying to create more arrays than maximum possible arrays");
        return;
    }
    else
    {
        if (prevIndexDeleted.size() == 0)
        {
            arrayNameMap.insert({arrayName, currentNumberOfArrays});
            qosArrayManager[currentNumberOfArrays]->SetArrayName(arrayName);
            arrayIdMap.insert({currentNumberOfArrays, arrayName});
            currentNumberOfArrays++;
        }
        else
        {
            uint32_t index = prevIndexDeleted.front();
            prevIndexDeleted.erase(prevIndexDeleted.begin());
            arrayNameMap.insert({arrayName, index});
            qosArrayManager[index]->SetArrayName(arrayName);
            arrayIdMap.insert({index, arrayName});
            currentNumberOfArrays++;
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
QosManager::DeleteEntryArrayMap(std::string arrayName)
{
    std::lock_guard<std::mutex> lock(mapUpdateLock);
    if (arrayNameMap.find(arrayName) == arrayNameMap.end())
    {
        POS_TRACE_ERROR(static_cast<int>(EID(QOS_ARRAY_DOES_NOT_EXIST)), "Deleting array which does not exist");
        return;
    }
    else
    {
        uint32_t index = arrayNameMap[arrayName];
        arrayNameMap.erase(arrayName);
        arrayIdMap.erase(index);
        prevIndexDeleted.push_back(index);
        currentNumberOfArrays--;
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

uint64_t
QosManager::GetRecentValue(int64_t cliInput, uint64_t prevValue)
{
    const int32_t NOT_INPUT = -1;
    if (cliInput == NOT_INPUT)
    {
        return prevValue;
    }
    return cliInput;
}

int32_t
QosManager::UpdateVolumePolicy(int64_t minBwFromCli,
    int64_t maxBwFromCli, int64_t minIopsFromCli, int64_t maxIopsFromCli, std::string& errorMsg,
    std::vector<std::pair<string, uint32_t>>& validVolumes, string arrayName)
{
    int retVal = -1;
    qos_vol_policy prevVolPolicy;
    qos_vol_policy newVolPolicy;
    std::list<uint32_t> mountedVolumes;

    for (auto vol : validVolumes)
    {
        uint32_t volId = vol.second;
        prevVolPolicy = GetVolumePolicy(volId, arrayName);
        newVolPolicy = prevVolPolicy;
        newVolPolicy.policyChange = false;
        newVolPolicy.maxValueChanged = false;

        uint64_t maxBw = GetRecentValue(maxBwFromCli, prevVolPolicy.maxBw);
        uint64_t minBw = GetRecentValue(minBwFromCli, prevVolPolicy.minBw);
        uint64_t maxIops = GetRecentValue(maxIopsFromCli, prevVolPolicy.maxIops);
        uint64_t minIops = GetRecentValue(minIopsFromCli, prevVolPolicy.minIops);

        if (maxBw != 0 && (maxBw < MIN_BW_LIMIT || maxBw > MAX_BW_LIMIT))
        {
            errorMsg = "Max Bandwidth value outside allowed range";
            return static_cast<int>(EID(VOL_REQ_QOS_OUT_OF_RANGE));
        }

        if (minBw != 0 && (minBw > MAX_BW_LIMIT))
        {
            errorMsg = "Min Bandwidth value outside allowed range";
            return static_cast<int>(EID(VOL_REQ_QOS_OUT_OF_RANGE));
        }

        if (maxIops != 0 && (maxIops < MIN_IOPS_LIMIT || maxIops > MAX_IOPS_LIMIT))
        {
            errorMsg = "Max Iops value outside allowed range";
            return static_cast<int>(EID(VOL_REQ_QOS_OUT_OF_RANGE));
        }

        if (minIops != 0 && (minIops > MAX_IOPS_LIMIT))
        {
            errorMsg = "Min Iops value outside allowed range";
            return static_cast<int>(EID(VOL_REQ_QOS_OUT_OF_RANGE));
        }

        if (maxBw != 0 && maxBw < minBw)
        {
            errorMsg = "Max bw less than min bw";
            return static_cast<int>(EID(VOL_REQ_QOS_OUT_OF_RANGE));
        }

        if (maxIops != 0 && maxIops < minIops)
        {
            errorMsg = "Max Iops less than min Iops";
            return static_cast<int>(EID(VOL_REQ_QOS_OUT_OF_RANGE));
        }

        uint64_t calculatedMaxIops
            = (maxBw * M_KBYTES * M_KBYTES) / (ArrayConfig::BLOCK_SIZE_BYTE) / KIOPS;
        if (maxBw != 0 && calculatedMaxIops < minIops)
        {
            errorMsg = "Min Iops more than calculated maxBw. IoSize considered 4KB.";
            return static_cast<int>(EID(VOL_REQ_QOS_OUT_OF_RANGE));
        }

        uint64_t calculatedMinIops
            = (minBw * M_KBYTES * M_KBYTES) / (ArrayConfig::BLOCK_SIZE_BYTE) / KIOPS;
        if (maxIops != 0 && maxIops < calculatedMinIops)
        {
            errorMsg = "calculated min bw more than max Iops. IoSize considered 4KB.";
            return static_cast<int>(EID(VOL_REQ_QOS_OUT_OF_RANGE));
        }

        if (minIops != 0 && minBw != 0)
        {
            errorMsg = "Both Min Bw and Iops shall not be set. Please reset already set value for setting new type value";
            return static_cast<int>(EID(VOL_REQ_QOS_OUT_OF_RANGE));
        }

        if (minBw != 0)
        {
            newVolPolicy.minBwGuarantee = true;
        }
        if (minIops != 0)
        {
            newVolPolicy.minIopsGuarantee = true;
        }

        newVolPolicy.maxBw = maxBw;
        newVolPolicy.minBw = minBw;
        newVolPolicy.maxIops = maxIops;
        newVolPolicy.minIops = minIops;

        newVolPolicy.policyChange = true;
        if (maxBwFromCli != 0 || maxIopsFromCli != 0)
        {
            newVolPolicy.maxValueChanged = true;
        }
       
        if (true == newVolPolicy.policyChange)
        {
            if (true == newVolPolicy.minBwGuarantee && true == newVolPolicy.minIopsGuarantee)
            {
                errorMsg = "Either Min IOPS or Min BW Allowed";
                return QosReturnCode::MIN_IOPS_OR_MIN_BW_ONLY_ONE;
            }
            int32_t arrayId = QosManagerSingleton::Instance()->GetArrayIdFromMap(arrayName);
            if (arrayId != -1)
            {
                retVal = QosManagerSingleton::Instance()->UpdateVolumePolicy(volId, newVolPolicy, arrayId);
            }
            if (retVal != SUCCESS)
            {
                switch (retVal)
                {
                    case QosReturnCode::EXCEED_MIN_GUARANTEED_VOLUME_MAX_CNT:
                    {
                        errorMsg = "the count of min guaranteeed volume exceeds configured max value (default : 5)";
                        break;
                    }
                    case QosReturnCode::MIN_IOPS_OR_MIN_BW_ONLY_ONE:
                    {
                        errorMsg = "Either Min IOPS or Min BW Allowed";
                        break;
                    }
                    default:
                    {
                        errorMsg = "Qos Volume Policy Updated in QosManager failed";
                        break;
                    }
                }
                return retVal;
            }
        }
    }
    return SUCCESS;
}

} // namespace pos

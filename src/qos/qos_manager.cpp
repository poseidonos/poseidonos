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
#include "src/qos/qos_context.h"
#include "src/qos/context_factory.h"
#include "src/qos/internal_manager.h"
#include "src/qos/internal_manager_factory.h"
#include "src/qos/qos_event_manager.h"
#include "src/qos/qos_spdk_manager.h"
#include "src/qos/qos_volume_manager.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "src/spdk_wrapper/connection_management.h"

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
    quitQos = false;
    qosThread = nullptr;
    feQosEnabled = false;
    pollerTime = UINT32_MAX;
    volMinPolicyInEffect = false;
    minBwGuarantee = false;
    minGuaranteeVolume = DEFAULT_MIN_VOL;
    usedStripeCnt = 0;
    for (uint32_t event = 0; (BackendEvent)event < BackendEvent_Count; event++)
    {
        pendingEvents[event] = M_RESET_TO_ZERO;
        eventLog[event] = M_RESET_TO_ZERO;
        oldLog[event] = M_RESET_TO_ZERO;
    }
    volumePolicyUpdated = false;
    gcFreeSegments = 0;
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
        qosVolumeManager = new QosVolumeManager(feQosEnabled);
        qosContext = ContextFactory::CreateQosContext();
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
    delete qosVolumeManager;
    delete qosEventManager;
    delete qosContext;
    delete monitoringManager;
    delete policyManager;
    delete processingManager;
    delete correctionManager;
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
    quitQos = true;
    if (nullptr != qosThread)
    {
        qosThread->join();
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
QosManager::HandlePosIoSubmission(IbofIoSubmissionAdapter* aioSubmission, pos_io* volIo)
{
    qosVolumeManager->HandlePosIoSubmission(aioSubmission, volIo);
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
        if (true == quitQos)
        {
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
QosManager::DequeueVolumeParams(uint32_t reactor, uint32_t volId)
{
    return qosVolumeManager->DequeueParams(reactor, volId);
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
QosManager::IncreaseUsedStripeCnt(void)
{
    usedStripeCnt++;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::DecreaseUsedStripeCnt(void)
{
    usedStripeCnt--;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
uint32_t
QosManager::GetUsedStripeCnt(void)
{
    return usedStripeCnt;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
std::vector<int>
QosManager::GetVolumeFromActiveSubsystem(uint32_t nqnId)
{
    return qosVolumeManager->GetVolumeFromActiveSubsystem(nqnId);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::UpdateSubsystemToVolumeMap(uint32_t nqnId, uint32_t volId)
{
    qosVolumeManager->UpdateSubsystemToVolumeMap(nqnId, volId);
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
        return qosVolumeManager->VolumeQosPoller(param, aioSubmission);
    }
    else
    {
        return 0;
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
int
QosManager::UpdateVolumePolicy(uint32_t volId, qos_vol_policy policy)
{
    uint32_t minGuaranteeVol = minGuaranteeVolume;
    bool minBwPolicy = minBwGuarantee;
    bool minPolicyInEffect = volMinPolicyInEffect;
    qos_vol_policy cliPolicy = policy;

    bool minPolicyReceived = ((true == policy.minBwGuarantee) || (true == policy.minIopsGuarantee));

    if (true == volMinPolicyInEffect)
    {
        if (true == minPolicyReceived)
        {
            if (minGuaranteeVolume != volId)
            {
                // Min Volume set, trying to set minimum for diff volume
                return QosReturnCode::ONE_MINIMUM_GUARANTEE_SUPPORTED;
            }
            else
            {
                if (true == minBwPolicy)
                {
                    if (true == policy.minIopsGuarantee)
                    {
                        // Min BW set, trying to set Min IOPS
                        return QosReturnCode::MIN_IOPS_OR_MIN_BW_ONLY_ONE;
                    }
                }
                else
                {
                    if (true == policy.minBwGuarantee)
                    {
                        // Min IOPS set, trying to set Min BW
                        return QosReturnCode::MIN_IOPS_OR_MIN_BW_ONLY_ONE;
                    }
                }
            }
        }
        else
        {
            if (minGuaranteeVolume == volId)
            {
                minGuaranteeVol = DEFAULT_MIN_VOL;
                minPolicyInEffect = false;
                minBwPolicy = false;
            }
        }
    }
    else
    {
        if (true == minPolicyReceived)
        {
            minGuaranteeVol = volId;
            minPolicyInEffect = true;

            if (true == policy.minBwGuarantee)
            {
                minBwPolicy = true;
            }
            else
            {
                minBwPolicy = false;
            }
        }
    }

    if (0 == policy.maxBw)
    {
        policy.maxBw = DEFAULT_MAX_BW_IOPS;
    }
    else
    {
        policy.maxBw = policy.maxBw * (M_KBYTES * M_KBYTES / (PARAMETER_COLLECTION_INTERVAL));
    }

    if (0 == policy.maxIops)
    {
        policy.maxIops = DEFAULT_MAX_BW_IOPS;
    }
    else
    {
        policy.maxIops = policy.maxIops / PARAMETER_COLLECTION_INTERVAL;
    }

    if (0 == policy.minBw)
    {
        policy.minBw = DEFAULT_MIN_BW_MBPS;
    }
    policy.minBw = policy.minBw * (M_KBYTES * M_KBYTES / (PARAMETER_COLLECTION_INTERVAL));

    if (0 == policy.minIops)
    {
        policy.minIops = DEFAULT_MIN_IOPS;
    }
    policy.minIops = policy.minIops / PARAMETER_COLLECTION_INTERVAL;
    {
        std::unique_lock<std::mutex> uniqueLock(policyUpdateLock);
        volPolicyCli[volId] = cliPolicy;
        volumePolicyMapCli[volId] = policy;
        minGuaranteeVolume = minGuaranteeVol;
        minBwGuarantee = minBwPolicy;
        volMinPolicyInEffect = minPolicyInEffect;
        volumePolicyUpdated = true;
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
QosManager::GetVolumePolicy(uint32_t volId)
{
    std::unique_lock<std::mutex> uniqueLock(policyUpdateLock);
    return volPolicyCli[volId];
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
QosManager::GetRebuildPolicy(void)
{
    std::unique_lock<std::mutex> uniqueLock(policyUpdateLock);
    return rebuildPolicyCli;
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
    std::unique_lock<std::mutex> uniqueLock(policyUpdateLock);
    rebuildPolicyCli = rebuildPolicy;
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
QosManager::SetVolumeLimit(uint32_t reactor, uint32_t volId, int64_t weight, bool iops)
{
    qosVolumeManager->SetVolumeLimit(reactor, volId, weight, iops);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int64_t
QosManager::GetVolumeLimit(uint32_t reactor, uint32_t volId, bool iops)
{
    return qosVolumeManager->GetVolumeLimit(reactor, volId, iops);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bool
QosManager::IsVolumePolicyUpdated(void)
{
    return volumePolicyUpdated;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::SetGcFreeSegment(uint32_t freeSegments)
{
    gcFreeSegments = freeSegments;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
uint32_t
QosManager::GetGcFreeSegment(void)
{
    return gcFreeSegments;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::GetVolumePolicyMap(std::map<uint32_t, qos_vol_policy>& volumePolicyMapCopy)
{
    std::unique_lock<std::mutex> uniqueLock(policyUpdateLock);
    volumePolicyMapCopy = volumePolicyMapCli;
    volumePolicyMapCli.clear();
    volumePolicyUpdated = false;
}
} // namespace pos

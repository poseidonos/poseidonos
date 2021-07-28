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

#include "src/qos/monitoring_manager.h"

#include "src/cpu_affinity/affinity_manager.h"
#include "src/include/pos_event_id.hpp"
#include "src/qos/qos_context.h"
#include "src/qos/qos_manager.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "src/spdk_wrapper/connection_management.h"

#define VALID_ENTRY (1)
#define INVALID_ENTRY (0)
#define NVMF_CONNECT (0)
#define NVMF_DISCONNECT (1)

struct nvmfConnectEntry
{
    uint32_t reactorId;
    uint32_t subsysId;
    uint32_t connectType; // 0=Connect 1=Disconnect
};

namespace pos
{
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosMonitoringManager::QosMonitoringManager(QosContext* qosCtx)
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
QosMonitoringManager::~QosMonitoringManager(void)
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
QosMonitoringManager::Execute(void)
{
    if (QosManagerSingleton::Instance()->IsFeQosEnabled() == true)
    {
        _UpdateContextUserVolumePolicy();
        if (true == _GatherActiveVolumeParameters())
        {
            _ComputeTotalActiveConnection();
        }
        _UpdateAllVolumeParameter();
    }
    _UpdateContextUserRebuildPolicy();
    _GatherActiveEventParameters();
    _UpdateContextResourceDetails();
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
QosMonitoringManager::_ComputeTotalActiveConnection(void)
{
    uint32_t totalConntection = 0;
    std::map<uint32_t, uint32_t> activeVolumeMap = qosContext->GetActiveVolumes();
    std::map<uint32_t, map<uint32_t, uint32_t>> volReactorMap = qosContext->GetActiveVolumeReactors();

    for (map<uint32_t, uint32_t>::iterator it = activeVolumeMap.begin(); it != activeVolumeMap.end(); it++)
    {
        uint32_t volId = it->first;
        for (map<uint32_t, uint32_t>::iterator it = volReactorMap[volId].begin(); it != volReactorMap[volId].end(); ++it)
        {
            totalConntection += it->second;
        }
        qosContext->SetTotalConnection(volId, totalConntection);
        totalConntection = 0;
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
QosMonitoringManager::_UpdateContextUserVolumePolicy(void)
{
    QosManager* qosManager = QosManagerSingleton::Instance();
    std::map<uint32_t, qos_vol_policy> volumePolicyMap;
    bool volumePolicyUpdated = qosManager->IsVolumePolicyUpdated();
    QosUserPolicy& userPolicy = qosContext->GetQosUserPolicy();
    AllVolumeUserPolicy& allVolumeUserPolicy = userPolicy.GetAllVolumeUserPolicy();
    allVolumeUserPolicy.SetMaxThrottlingChanged(false);
    uint32_t currentMinPolicyVolume = allVolumeUserPolicy.GetMinimumGuaranteeVolume();
    bool currentMinPolicyInEffect = allVolumeUserPolicy.IsMinPolicyInEffect();
    bool currentMinBwPolicy = allVolumeUserPolicy.IsMinBwPolicyInEffect();
    bool maxThrottlingChanged = false;

    if (false == volumePolicyUpdated)
    {
        return;
    }
    else
    {
        qosManager->GetVolumePolicyMap(volumePolicyMap);
    }

    for (auto vol = volumePolicyMap.begin(); vol != volumePolicyMap.end(); vol++)
    {
        qos_vol_policy volumePolicy = vol->second;

        if (true == volumePolicy.maxValueChanged)
        {
            maxThrottlingChanged = true;
        }

        VolumeUserPolicy volUserPolicy;
        volUserPolicy.SetMaxBandwidth(volumePolicy.maxBw);
        volUserPolicy.SetMaxIops(volumePolicy.maxIops);
        volUserPolicy.SetMinBandwidth(volumePolicy.minBw);
        volUserPolicy.SetMinIops(volumePolicy.minIops);
        allVolumeUserPolicy.InsertVolumeUserPolicy(vol->first, volUserPolicy);

        bool minBwPolicy = volumePolicy.minBwGuarantee;
        bool minIopsPolicy = volumePolicy.minIopsGuarantee;
        if (true == minBwPolicy || true == minIopsPolicy)
        {
            currentMinPolicyInEffect = true;
            if (true == minBwPolicy)
            {
                currentMinBwPolicy = true;
            }
            else
            {
                currentMinBwPolicy = false;
            }
            currentMinPolicyVolume = vol->first;
        }
        else
        {
            if (currentMinPolicyVolume == vol->first)
            {
                currentMinPolicyInEffect = false;
                currentMinBwPolicy = false;
                currentMinPolicyVolume = DEFAULT_MIN_VOL;
            }
        }
    }
    allVolumeUserPolicy.SetMinimumGuaranteeVolume(currentMinPolicyVolume);
    allVolumeUserPolicy.SetMinimumPolicyInEffect(currentMinPolicyInEffect);
    allVolumeUserPolicy.SetMinimumPolicyType(currentMinBwPolicy);
    if (true == maxThrottlingChanged)
    {
        allVolumeUserPolicy.SetMaxThrottlingChanged(true);
    }
    volumePolicyUpdated = false;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosMonitoringManager::_UpdateContextUserRebuildPolicy(void)
{
    QosManager* qosManager = QosManagerSingleton::Instance();
    QosUserPolicy& userPolicy = qosContext->GetQosUserPolicy();
    RebuildUserPolicy& rebuildUserPolicy = userPolicy.GetRebuildUserPolicy();

    qos_rebuild_policy rebuildPolicy = qosManager->GetRebuildPolicy();

    if (rebuildUserPolicy.GetRebuildImpact() != rebuildPolicy.rebuildImpact)
    {
        rebuildUserPolicy.SetRebuildImpact(rebuildPolicy.rebuildImpact);
        rebuildUserPolicy.SetPolicyChange(true);
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
QosMonitoringManager::_UpdateContextResourceDetails(void)
{
    QosManager* qosManager = QosManagerSingleton::Instance();
    QosResource& resourceDetails = qosContext->GetQosResource();
    ResourceNvramStripes& resourceNvramStripes = resourceDetails.GetResourceNvramStripes();
    ResourceCpu& resourceCpu = resourceDetails.GetResourceCpu();
    ResourceArray& resourceArray = resourceDetails.GetResourceArray();
    uint32_t usedStripeCnt = qosManager->GetUsedStripeCnt();
    resourceNvramStripes.SetNvramStripesUsedCount(usedStripeCnt);

    for (uint32_t event = 0; event < BackendEvent_Count; event++)
    {
        uint32_t pendingEventCount = qosManager->GetPendingEvents(static_cast<BackendEvent>(event));
        resourceCpu.SetEventPendingCpuCount(static_cast<BackendEvent>(event), pendingEventCount);
        uint32_t generatedCpuEvents = qosManager->GetEventLog(static_cast<BackendEvent>(event));
        resourceCpu.SetTotalGeneratedEvents(static_cast<BackendEvent>(event), generatedCpuEvents);
    }
    resourceArray.SetGcFreeSegment(qosManager->GetGcFreeSegment());
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosMonitoringManager::_UpdateContextActiveVolumes(uint32_t volId)
{
    qosContext->InsertActiveVolume(volId);
    _UpdateContextVolumeThrottle(volId);
    _UpdateContextVolumeParameter(volId);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosMonitoringManager::_UpdateContextActiveReactorVolumes(uint32_t reactor, uint32_t volId)
{
    qosContext->InsertActiveReactorVolume(reactor, volId);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosMonitoringManager::_UpdateContextActiveVolumeReactors(std::map<uint32_t, map<uint32_t, uint32_t>> map)
{
    qosContext->InsertActiveVolumeReactor(map);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosMonitoringManager::_UpdateVolumeReactorParameter(uint32_t volId, uint32_t reactor)
{
    QosParameters& qosParam = qosContext->GetQosParameters();
    AllVolumeParameter& allVolParam = qosParam.GetAllVolumeParameter();
    {
        VolumeParameter volParam;
        bool volFound = allVolParam.VolumeExists(volId);
        if (volFound == false)
        {
            allVolParam.InsertVolumeParameter(volId, volParam);
        }
    }

    VolumeParameter& volParam = allVolParam.GetVolumeParameter(volId);
    {
        ReactorParameter reactorParam;
        if (false == volParam.IsReactorExists(reactor))
        {
            volParam.InsertReactorParameter(reactor, reactorParam);
        }
    }
    ReactorParameter& reactorParam = volParam.GetReactorParameter(reactor);
    reactorParam.IncreaseBandwidth(volParams[volId].currentBW);
    reactorParam.IncreaseIops(volParams[volId].currentIOs);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosMonitoringManager::_UpdateAllVolumeParameter(void)
{
    std::map<uint32_t, uint32_t> activeVolumeMap = qosContext->GetActiveVolumes();
    for (map<uint32_t, uint32_t>::iterator it = activeVolumeMap.begin(); it != activeVolumeMap.end(); it++)
    {
        uint32_t volId = it->first;
        _UpdateVolumeParameter(volId);
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
QosMonitoringManager::_UpdateVolumeParameter(uint32_t volId)
{
    QosParameters& qosParam = qosContext->GetQosParameters();
    AllVolumeParameter& allVolParam = qosParam.GetAllVolumeParameter();
    VolumeParameter& volParam = allVolParam.GetVolumeParameter(volId);
    map<uint32_t, ReactorParameter> reactorParamMap = volParam.GetReactorParameterMap();
    for (map<uint32_t, ReactorParameter>::iterator it = reactorParamMap.begin(); it != reactorParamMap.end(); it++)
    {
        ReactorParameter& reactorParam = volParam.GetReactorParameter(it->first);
        volParam.IncreaseBandwidth(reactorParam.GetBandwidth());
        volParam.IncreaseIops(reactorParam.GetIops());
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
QosMonitoringManager::_GatherActiveVolumeParameters(void)
{
    QosManager* qosManager = QosManagerSingleton::Instance();
    qosContext->ResetActiveVolume();
    qosContext->ResetActiveReactorVolume();

    // Moved from VolumePolicy
    QosCorrection& qosCorrection = qosContext->GetQosCorrection();
    AllVolumeThrottle& allVolumeThrottle = qosCorrection.GetVolumeThrottlePolicy();
    allVolumeThrottle.Reset();
    QosParameters& qosParameters = qosContext->GetQosParameters();
    AllVolumeParameter& allVolumeParameter = qosParameters.GetAllVolumeParameter();
    allVolumeParameter.Reset();

    QosParameters& qosParameter = qosContext->GetQosParameters();
    qosParameter.Reset();
    std::vector<int> subsystemVolList;
    uint32_t activeConnectionCtr = 0;
    map<uint32_t, uint32_t> volConnectionMap;
    map<uint32_t, uint32_t> reactorConnectionMap;
    bool changeDetected = false;
    std::vector<uint32_t> reactorCoreList = qosContext->GetReactorCoreList();

    do
    {
        activeConnectionCtr = M_RESET_TO_ZERO;

        for (uint32_t index = 0; index < reactorCoreList.size(); index++)
        {
            if (true == IsExitQosSet())
            {
                activeConnectionCtr = M_RESET_TO_ZERO;
                break;
            }
            uint32_t reactor = reactorCoreList[index];
            while (!IsExitQosSet())
            {
                struct nvmfConnectEntry connectEntry;
                bool entry = SpdkConnection::GetNvmfReactorConnection(reactor, &connectEntry);
                if (false == entry)
                {
                    break;
                }
                else
                {
                    subsystemVolList = qosManager->GetVolumeFromActiveSubsystem(connectEntry.subsysId);
                    if (0 == subsystemVolList.size())
                    {
                        break;
                    }
                    changeDetected = true;
                    uint32_t volId = subsystemVolList[0]; // Currently Qos can handle only 1 NSID in subsystem
                    int count = 0;
                    if (connectEntry.connectType == NVMF_CONNECT)
                    {
                        reactorConnectionMap = volReactorMap[volId];
                        count = reactorConnectionMap[reactor];
                        count++;
                        reactorConnectionMap[reactor] = count;
                        volReactorMap[volId] = reactorConnectionMap;
                        volConnectionMap = reactorVolMap[reactor];
                        count = volConnectionMap[volId];
                        count++;
                        volConnectionMap[volId] = count;
                        reactorVolMap[reactor] = volConnectionMap;
                    }
                    else
                    {
                        reactorConnectionMap = volReactorMap[volId];
                        count = reactorConnectionMap[reactor];
                        if (count <= 1)
                        {
                            reactorConnectionMap.erase(reactor);
                        }
                        else
                        {
                            count--;
                            reactorConnectionMap[reactor] = count;
                        }
                        volReactorMap[volId] = reactorConnectionMap;
                        volConnectionMap = reactorVolMap[reactor];
                        count = volConnectionMap[volId];
                        if (count <= 1)
                        {
                            volConnectionMap.erase(volId);
                        }
                        else
                        {
                            count--;
                            volConnectionMap[volId] = count;
                        }
                        reactorVolMap[reactor] = volConnectionMap;
                    }
                }
            }
            activeConnectionCtr += reactorVolMap[reactor].size();
        }
        if (0 == activeConnectionCtr)
        {
            break;
        }
        for (uint32_t index = 0; index < reactorCoreList.size(); index++)
        {
            uint32_t reactor = reactorCoreList[index];
            for (map<uint32_t, uint32_t>::iterator it = reactorVolMap[reactor].begin(); it != reactorVolMap[reactor].end(); it++)
            {
                uint32_t volId = it->first;
                volParams[volId] = qosManager->DequeueVolumeParams(reactor, volId);
                if (volParams[volId].valid == M_VALID_ENTRY)
                {
                    _UpdateContextActiveVolumes(volId);
                    _UpdateContextActiveReactorVolumes(reactor, volId);
                    volParams[volId].valid = M_INVALID_ENTRY;
                    _UpdateVolumeReactorParameter(volId, reactor);
                }
            }
        }
        _UpdateContextActiveVolumeReactors(volReactorMap);
    } while (!((activeConnectionCtr <= qosContext->GetActiveReactorVolumeCount()) && activeConnectionCtr));

    return changeDetected;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosMonitoringManager::_UpdateEventParameter(BackendEvent event)
{
    QosParameters& qosParam = qosContext->GetQosParameters();
    AllEventParameter& allEventParam = qosParam.GetAllEventParameter();

    bool eventFound = allEventParam.EventExists(event);

    if (true == eventFound)
    {
        EventParameter& eventParam = allEventParam.GetEventParameter(event);
        eventParam.IncreaseBandwidth(eventParams[event].currentBW);
    }
    else
    {
        EventParameter eventParam;
        eventParam.SetBandwidth(eventParams[event].currentBW);
        allEventParam.InsertEventParameter(event, eventParam);
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
QosMonitoringManager::_GatherActiveEventParameters(void)
{
    QosManager* qosManager = QosManagerSingleton::Instance();
    cpu_set_t cpuSet = AffinityManagerSingleton::Instance()->GetCpuSet(CoreType::UDD_IO_WORKER);
    uint32_t cpuCount = CPU_COUNT(&cpuSet);

    for (uint32_t workerId = 0; workerId < cpuCount; workerId++)
    {
        for (uint32_t event = 0; (BackendEvent)event < BackendEvent_Count; event++)
        {
            do
            {
                eventParams[event].valid = M_INVALID_ENTRY;
                eventParams[event] = qosManager->DequeueEventParams(workerId, (BackendEvent)event);
                if (eventParams[event].valid == M_VALID_ENTRY)
                {
                    _UpdateEventParameter(static_cast<BackendEvent>(event));
                }
            } while ((eventParams[event].valid == M_VALID_ENTRY) && (!IsExitQosSet()));
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
QosMonitoringManager::_SetNextManagerType(void)
{
    nextManagerType = QosInternalManager_Processing;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosInternalManagerType
QosMonitoringManager::GetNextManagerType(void)
{
    return nextManagerType;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosMonitoringManager::_UpdateContextVolumeThrottle(uint32_t volId)
{
    QosCorrection& qosCorrection = qosContext->GetQosCorrection();
    AllVolumeThrottle& allVolumeThrottle = qosCorrection.GetVolumeThrottlePolicy();

    VolumeThrottle volThottle;
    volThottle.Reset();
    allVolumeThrottle.InsertVolumeThrottle(volId, volThottle);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosMonitoringManager::_UpdateContextVolumeParameter(uint32_t volId)
{
    QosParameters& qosParameters = qosContext->GetQosParameters();
    AllVolumeParameter& allVolumeParameter = qosParameters.GetAllVolumeParameter();

    VolumeParameter volParam;
    volParam.Reset();
    allVolumeParameter.InsertVolumeParameter(volId, volParam);
}
} // namespace pos

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

#include "src/qos/monitoring_manager.h"
#include "src/include/pos_event_id.hpp"
#include "src/qos/qos_context.h"
#include "src/qos/qos_manager.h"
#include "src/spdk_wrapper/event_framework_api.h"

#define VALID_ENTRY (1)
#define INVALID_ENTRY (0)
#define NVMF_CONNECT (0)
#define NVMF_DISCONNECT (1)

namespace pos
{
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosMonitoringManager::QosMonitoringManager(QosContext* qosCtx,
        QosManager* qosManager,
        SpdkPosNvmfCaller* spdkPosNvmfCaller,
        AffinityManager* affinityManager)
    : qosContext(qosCtx),
    qosManager(qosManager),
    spdkPosNvmfCaller(spdkPosNvmfCaller),
    affinityManager(affinityManager)
{
    nextManagerType = QosInternalManager_Unknown;
    for (uint32_t i = 0; i < MAX_ARRAY_COUNT; i++)
    {
        qosMonitoringManagerArray[i] = new QosMonitoringManagerArray(i, qosCtx, qosManager);
    }
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
    for (uint32_t i = 0; i < MAX_ARRAY_COUNT; i++)
    {
        delete qosMonitoringManagerArray[i];
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
QosMonitoringManager::Execute(void)
{
    if (qosManager->IsFeQosEnabled() == true)
    {
        _UpdateContextUserVolumePolicy();
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
QosMonitoringManager::_UpdateContextUserVolumePolicy(void)
{
    uint32_t maxArrays = qosManager->GetNumberOfArrays();
    for (uint32_t i = 0; i < maxArrays; i++)
    {
        qosMonitoringManagerArray[i]->UpdateContextUserVolumePolicy();
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
QosMonitoringManager::_UpdateContextUserRebuildPolicy(void)
{
    for (uint32_t i = 0; i < MAX_ARRAY_COUNT; i++)
    {
        qosMonitoringManagerArray[i]->UpdateContextUserRebuildPolicy();
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
    uint32_t maxArrays = qosManager->GetNumberOfArrays();
    for (uint32_t i = 0; i < maxArrays; i++)
    {
        qosMonitoringManagerArray[i]->UpdateContextResourceDetails();
    }
    QosResource& resourceDetails = qosContext->GetQosResource();
    ResourceCpu& resourceCpu = resourceDetails.GetResourceCpu();

    for (uint32_t event = 0; event < BackendEvent_Count; event++)
    {
        uint32_t pendingEventCount = qosManager->GetPendingBackendEvents(static_cast<BackendEvent>(event));
        resourceCpu.SetEventPendingCpuCount(static_cast<BackendEvent>(event), pendingEventCount);
        uint32_t generatedCpuEvents = qosManager->GetEventLog(static_cast<BackendEvent>(event));
        resourceCpu.SetTotalGeneratedEvents(static_cast<BackendEvent>(event), generatedCpuEvents);
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
QosMonitoringManager::_UpdateAllVolumeParameter(void)
{
    std::map<uint32_t, uint32_t> activeVolumeMap = qosContext->GetActiveVolumes();
    for (map<uint32_t, uint32_t>::iterator it = activeVolumeMap.begin(); it != activeVolumeMap.end(); it++)
    {
        uint32_t volId = it->first;
        uint32_t arrVolId = volId % MAX_VOLUME_COUNT;
        uint32_t arrayId = volId / MAX_VOLUME_COUNT;
        qosMonitoringManagerArray[arrayId]->UpdateVolumeParameter(arrVolId);
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
    return false;
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
    cpu_set_t cpuSet = affinityManager->GetCpuSet(CoreType::UDD_IO_WORKER);
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
} // namespace pos

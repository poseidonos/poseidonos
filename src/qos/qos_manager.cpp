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

#include "spdk/nvmf.h"
#include "spdk/thread.h"
#include "src/device/event_framework_api.h"
#include "src/device/ublock_device.h"
#include "src/include/ibof_event_id.hpp"
#include "src/io/frontend_io/aio.h"
#include "src/io/general_io/affinity_manager.h"
#include "src/io/general_io/ubio.h"
#include "src/io/general_io/volume_io.h"
#include "src/logger/logger.h"
#include "src/network/nvmf_volume_ibof.hpp"
#include "src/scheduler/io_worker.h"
#include "src/sys_event/volume_event_publisher.h"
namespace ibofos
{
#if defined QOS_ENABLED_FE
const uint16_t QosManager::M_INVALID_SUBSYSTEM = 0;
const uint16_t QosManager::M_VALID_SUBSYSTEM = 1;
std::atomic<bool> QosManager::registerQosPollerDone = false;
std::atomic<bool> QosManager::unregistrationComplete = false;
#endif

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosManager::_PollerUnregister(void* arg1, void* arg2)
{
#if defined QOS_ENABLED_FE
    QosManager* qosManager = static_cast<QosManager*>(arg1);
    uint32_t currentCoreIndex = EventFrameworkApi::GetCurrentReactor();
    if (qosManager->reactor_pollers[currentCoreIndex] != nullptr)
    {
        spdk_poller_unregister(&qosManager->reactor_pollers[currentCoreIndex]);
    }
    if ((EventFrameworkApi::IsLastReactorNow()))
    {
        unregistrationComplete = true;
    }
    else
    {
        EventFrameworkApi::SendSpdkEvent(EventFrameworkApi::GetNextReactor(),
            _PollerUnregister, qosManager, nullptr);
    }
#endif
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosManager::Finalize(void)
{
#if defined QOS_ENABLED_FE
    uint32_t firstReactorCore = EventFrameworkApi::GetFirstReactor();
    bool succeeded = EventFrameworkApi::SendSpdkEvent(firstReactorCore,
        _PollerUnregister, this, nullptr);
    if (unlikely(false == succeeded))
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::QOSMGR_REGISTER_POLLER_FAILED;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId), firstReactorCore);
        assert(0);
    }
    while (unregistrationComplete == false)
    {
        usleep(1);
    }
#endif
    quitQos = true;
    if (nullptr != qosThread)
    {
        qosThread->join();
    }
    delete qosThread;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosManager::_SetupQoSReactorPoller(void)
{
#if defined QOS_ENABLED_FE
    spdk_nvmf_initialize_reactor_subsystem_mapping();
    registerQosPollerDone = false;

    uint32_t firstReactorCore = EventFrameworkApi::GetFirstReactor();
    bool succeeded = EventFrameworkApi::SendSpdkEvent(firstReactorCore,
        _RegisterQosPoller, this, nullptr);
    if (unlikely(false == succeeded))
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::QOSMGR_REGISTER_POLLER_FAILED;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId), firstReactorCore);
    }
    else
    {
        while (false == registerQosPollerDone)
        {
            usleep(1);
        }
    }
#endif
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosManager::_RegisterQosPoller(void* arg1, void* arg2)
{
#if defined QOS_ENABLED_FE
    QosManager* qosManager = static_cast<QosManager*>(arg1);
    uint32_t currentCoreIndex = EventFrameworkApi::GetCurrentReactor();
    poller_structure* volPollStructure =
        &qosManager->volPollStructure[currentCoreIndex];

    volPollStructure->qosTimeSlice =
        IBOF_QOS_TIMESLICE_IN_USEC * spdk_get_ticks_hz() / SPDK_SEC_TO_USEC;
    volPollStructure->nextTimeStamp =
        spdk_get_ticks() + volPollStructure->qosTimeSlice;
    qosManager->reactor_pollers[currentCoreIndex] = spdk_poller_register(qosManager->_VolumeQosPoller, volPollStructure, 0);
    if (EventFrameworkApi::IsLastReactorNow())
    {
        registerQosPollerDone = true;
    }
    else
    {
        uint32_t nextCore = EventFrameworkApi::GetNextReactor();
        bool success = EventFrameworkApi::SendSpdkEvent(nextCore,
            _RegisterQosPoller, qosManager, nullptr);
        if (unlikely(false == success))
        {
            IBOF_EVENT_ID eventId =
                IBOF_EVENT_ID::QOSMGR_REGISTER_POLLER_FAILED;
            IBOF_TRACE_ERROR(static_cast<int>(eventId),
                IbofEventId::GetString(eventId), nextCore);

            registerQosPollerDone = true;
        }
    }
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
QosManager::EventRateLimit(uint32_t id, BackendEvent event)
{
    if (remainingEventLimit[id][event] > 0)
        return false;
    else
        return true;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

bool
QosManager::RateLimit(uint32_t reactor, int volId)
{
    if (remainingLimit[reactor][volId] > 0)
        return false;
    else
        return true;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosManager::UpdateEventRateLimit(uint32_t id, BackendEvent event, uint64_t size)
{
    remainingEventLimit[id][event] -= size;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosManager::UpdateRateLimit(uint32_t reactor, int volId, uint64_t size)
{
    remainingLimit[reactor][volId] -= size;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosManager::AioSubmitAsyncIO(VolumeIoSmartPtr volumeIo)
{
#if defined QOS_ENABLED_FE
    VolumeIoSmartPtr queuedVolumeIo = nullptr;
    uint32_t reactorId = EventFrameworkApi::GetCurrentReactor();
    uint32_t volId = volumeIo->GetVolumeId();
    uint64_t currentBw = 0;
    currentBw = volumeQosParam[reactorId][volId].currentBW;
    EnqueueVolumeUbio(reactorId, volId, volumeIo);
    while (1)
    {
        if (RateLimit(reactorId, volId) == true)
        {
            break;
        }
        queuedVolumeIo = PeekVolumeUbio(reactorId, volId);
        if (queuedVolumeIo == nullptr)
        {
            break;
        }
        currentBw = currentBw + queuedVolumeIo->GetMemSize();
        queuedVolumeIo = DequeueVolumeUbio(reactorId, volId);
        AIO aio;
        aio.SubmitAsyncIO(queuedVolumeIo);
        UpdateRateLimit(reactorId, volId, queuedVolumeIo->GetMemSize());
    }
    volumeQosParam[reactorId][volId].currentBW = currentBw;
#endif
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosManager::SubmitAsyncIO(IOWorker* ioWorker, UbioSmartPtr ubio)
{
    UbioSmartPtr queuedUbio = nullptr;
    int completionCount = 0;
    uint32_t id = ioWorker->GetWorkerId();
    BackendEvent event = IdentifyEventType(ubio);
    uint64_t currentBw = 0;
    if ((BackendEvent_Unknown == event) || (ubio->IsSyncMode() == true))
    {
        completionCount = ubio->GetUBlock()->SubmitAsyncIO(ubio);
        ioWorker->DecreaseCurrentOutstandingIoCount(completionCount);
        return;
    }
    currentBw = eventQosParam[id][event].currentBW;
    EnqueueEventUbio(id, event, ubio);
    while (1)
    {
        if (EventRateLimit(id, event) == true)
            break;
        queuedUbio = PeekEventUbio(id, event);
        if (queuedUbio == nullptr)
        {
            break;
        }
        currentBw = currentBw + queuedUbio->GetSize();
        queuedUbio = DequeueEventUbio(id, event);
        completionCount = queuedUbio->GetUBlock()->SubmitAsyncIO(ubio);
        ioWorker->DecreaseCurrentOutstandingIoCount(completionCount);
        UpdateEventRateLimit(id, event, queuedUbio->GetSize());
    }
    eventQosParam[id][event].currentBW = currentBw;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosManager::InitilizeEventRateLimit(BackendEvent event, uint64_t size)
{
    for (uint32_t id = 0; id < MAX_IO_WORKER; id++)
        remainingEventLimit[id][event] = size;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosManager::InitilizeRateLimit(uint32_t reactor, int volId, uint64_t size)
{
    remainingLimit[reactor][volId] = size;
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosManager::QosManager()
: VolumeEvent("QosManager"),
  volPollStructure{
      0,
  },
  eventPollStructure{
      0,
  },
  aio(nullptr),
  quitQos(false),
  qosThread(nullptr),
  pollerTime(UINT32_MAX)
{
    qosDataManager = new QosDataManager();
    policyManager = new QosPolicyManager();

    uint64_t maxBackendQosLimit = MAX_QOS_LIMIT;
    uint64_t maxVolumeQosLimit = MAX_QOS_LIMIT;
    InitialiseUsedStripeCnt();
    uint32_t reactorCount = M_MAX_REACTORS;
    uint32_t volumeCount = MAX_VOLUME_COUNT;
    for (uint32_t reactor = 0; reactor < reactorCount; reactor++)
    {
        for (uint32_t volId = 0; volId < volumeCount; volId++)
        {
            SetVolumeWeight(reactor, volId, maxVolumeQosLimit);
            uint64_t size = GetVolumeWeight(reactor, volId);
            InitilizeRateLimit(reactor, volId, size);
            wtUpdate[volId] = 0;
        }
    }
    uint32_t backendEventCount = BackendEvent_Count;
    for (uint32_t eventId = BackendEvent_Start; eventId < backendEventCount; eventId++)
    {
        SetEventWeight((BackendEvent)eventId, maxBackendQosLimit);
        SetEventWeightWRR((BackendEvent)eventId, M_EQUAL_WEIGHT);
        InitilizeEventRateLimit((BackendEvent)eventId, GetEventWeight((BackendEvent)eventId));
    }

    for (uint32_t reactor = 0; reactor < M_MAX_REACTORS; reactor++)
    {
        reactor_pollers[reactor] = nullptr;
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosManager::~QosManager()
{
    delete qosDataManager;
    delete policyManager;
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
    AffinityManager* affinityManager = AffinityManagerSingleton::Instance();
    cpuSet = affinityManager->GetCpuSet(CoreType::QOS);

    policyManager->Initialize();
    for (uint32_t event = 0; (BackendEvent)event < BackendEvent_Count; event++)
    {
        pendingEvents[event] = M_RESET_TO_ZERO;
        eventLog[event] = M_RESET_TO_ZERO;
        oldLog[event] = M_RESET_TO_ZERO;
    }
    qosThread = new std::thread(&QosManager::_QosManagerPoller, this);

#if defined QOS_ENABLED_FE
    _SetupQoSReactorPoller();
#endif
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::_QosManagerPoller(void)
{
    sched_setaffinity(0, sizeof(cpuSet), &cpuSet);
    pthread_setname_np(pthread_self(), "QoSWorker");
#if defined QOS_ENABLED_FE
    std::vector<int> volList;
    uint32_t activeConnectionCtr = 0;
    int triggerQos = 0;
    uint32_t id = M_RESET_TO_ZERO;
    uint64_t startTimeStamp = M_RESET_TO_ZERO;
    uint64_t endTimeStamp = M_RESET_TO_ZERO;
    double elapsedMs = M_RESET_TO_ZERO;
    uint64_t msTimeStamp = IBOF_QOS_TIMESLICE_IN_USEC * spdk_get_ticks_hz() / SPDK_SEC_TO_USEC;
    std::pair<uint32_t, uint32_t> reactorVolumePair;
    map<uint32_t, uint32_t> volCount;
    map<uint32_t, uint32_t> reactorCount;
#endif
    while (1)
    {
        if (quitQos == true)
        {
            break;
        }
        policyManager->CopyEventPolicy();
#if defined QOS_ENABLED_FE
        policyManager->CopyVolumePolicy();

        do
        {
            activeConnectionCtr = M_RESET_TO_ZERO;
            volReactorMap.clear();
            reactorVolMap.clear();
            // TODO : This for loop will be removed in future with a list for optimization
            for (uint32_t reactor = 0; reactor < M_MAX_REACTORS; reactor++)
            {
                for (uint32_t subsys = 0; subsys < M_MAX_SUBSYSTEMS; subsys++)
                {
                    if (spdk_nvmf_get_reactor_subsystem_mapping(reactor, subsys) != M_INVALID_SUBSYSTEM)
                    {
                        volList = GetVolumeFromActiveSubsystem(subsys);
                        for (uint32_t volId = 0; volId < volList.size(); volId++)
                        {
                            totalConnection[volList[volId]] = 0;
                            volCount = reactorVolMap[reactor];
                            volCount[volList[volId]] += spdk_nvmf_get_reactor_subsystem_mapping(reactor, subsys);
                            reactorVolMap[reactor] = volCount;

                            reactorCount = volReactorMap[volList[volId]];
                            reactorCount[reactor] += spdk_nvmf_get_reactor_subsystem_mapping(reactor, subsys);
                            volReactorMap[volList[volId]] = reactorCount;

                            reactorVolumePair = std::make_pair(reactor, volList[volId]);
                            activeConnectionCtr++;
                            policyManager->activeVolMap.insert(pair<int, int>(id, 1));
                            if (true == policyManager->VolumeMinimumPolicyInEffect())
                            {
                                policyManager->volParams[volList[volId]] = DequeueVolumeParams(reactor, volList[volId]);
                                if (policyManager->volParams[volList[volId]].valid == M_VALID_ENTRY)
                                {
                                    policyManager->volParams[volList[volId]].valid = M_INVALID_ENTRY;
                                    id = volList[volId];
                                    std::pair<std::pair<uint32_t, uint32_t>, uint32_t> reactorVolPair = std::make_pair(reactorVolumePair, 1);
                                    policyManager->activeConnections.insert(reactorVolPair);
                                    policyManager->FillVolumeState(id);
                                }
                            }
                        }
                    }
                }
            }
            if (activeConnectionCtr == 0)
            {
                break;
            }
            if (policyManager->VolumeMinimumPolicyInEffect() == false)
            {
                break;
            }
        } while (!((activeConnectionCtr <= policyManager->activeConnections.size()) && activeConnectionCtr));
        // updated based on dirty only
        for (map<int, int>::iterator it = policyManager->activeVolMap.begin(); it != policyManager->activeVolMap.end(); it++)
        {
            uint32_t volId = it->first;
            {
                std::unique_lock<std::mutex> uniqueLock(policyUpdateLock);
                if (wtUpdate[volId] == 1)
                {
                    wtUpdate[volId] = 0;
                    uint64_t value;
                    value = maxVolWeightCli[volId];
                    maxVolWeight[volId] = value;
                }
                else
                {
                    continue;
                }
                for (map<uint32_t, uint32_t>::iterator it = volReactorMap[volId].begin(); it != volReactorMap[volId].end(); ++it)
                {
                    totalConnection[volId] += it->second;
                }
                for (map<uint32_t, uint32_t>::iterator it = volReactorMap[volId].begin(); it != volReactorMap[volId].end(); ++it)
                {
                    SetVolumeWeight(it->first, volId, (maxVolWeight[volId] * (it->second)) / (totalConnection[volId]));
                }
            }
        }

        if (false == policyManager->VolumeMinimumPolicyInEffect())
        {
            continue;
        }
        if (activeConnectionCtr == 0)
        {
            policyManager->ResetAll();
            continue;
        }
        startTimeStamp = spdk_get_ticks();

        if (endTimeStamp != 0)
        {
            elapsedMs = (double)((startTimeStamp - endTimeStamp) / (1.0 * msTimeStamp));
        }

        if (elapsedMs - floor(elapsedMs) > 0.5)
        {
            elapsedMs = ceil(elapsedMs);
        }
        else
        {
            elapsedMs = floor(elapsedMs);
        }

        endTimeStamp = startTimeStamp;
        for (uint32_t workerId = 0; workerId < MAX_IO_WORKER; workerId++)
        {
            for (uint32_t event = 0; (BackendEvent)event < BackendEvent_Count; event++)
            {
                do
                {
                    policyManager->eventParams[event].valid = M_INVALID_ENTRY;
                    policyManager->eventParams[event] = DequeueEventParams(workerId, (BackendEvent)event);
                    if (policyManager->eventParams[event].valid == M_VALID_ENTRY)
                    {
                        policyManager->FillEventState((BackendEvent)event);
                    }
                } while (policyManager->eventParams[event].valid == M_VALID_ENTRY);
            }
        }
        // Trigger point needs to changed should be a function of read min guarantee or write min gurantee
        if (policyManager->volPolicy[policyManager->minGuranteeVolId].workLoad == QosWorkloadType_Read)
        {
            triggerQos = 1;
        }
        else
        {
            triggerQos = 0;
        }
        triggerQos = 1;
        if (triggerQos)
        {
            triggerQos = 1;
            if (elapsedMs == 0)
            {
                elapsedMs = 1;
            }
            policyManager->CheckSystem(elapsedMs);
        }

        for (std::map<int, int>::iterator it = policyManager->activeVolMap.begin(); it != policyManager->activeVolMap.end(); it++)
        {
            policyManager->UpdateOldParamVolumeState(it->first);
            policyManager->ResetVolumeState(it->first);
        }
        policyManager->activeVolMap.clear();
        policyManager->activeConnections.clear();
        for (uint32_t event = 0; (BackendEvent)event < BackendEvent_Count; event++)
        {
            policyManager->UpdateOldParamEventState((BackendEvent)event);
            policyManager->ResetEventState((BackendEvent)event);
        }
        for (uint16_t i = 0; i < M_MAX_REACTORS; i++)
        {
            policyManager->volParams[i].valid = M_INVALID_ENTRY;
        }
#endif
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
QosManager::EnqueueVolumeParams(uint32_t reactor, uint32_t volId, volume_qos_params& volume_param)
{
    qosDataManager->EnqueueVolumeParams(reactor, volId, volume_param);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
volume_qos_params
QosManager::DequeueVolumeParams(uint32_t reactor, uint32_t volId)
{
    return qosDataManager->DequeueVolumeParams(reactor, volId);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::EnqueueVolumeUbio(uint32_t reactorId, uint32_t volId, VolumeIoSmartPtr volIo)
{
    qosDataManager->EnqueueVolumeUbio(reactorId, volId, volIo);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
VolumeIoSmartPtr
QosManager::DequeueVolumeUbio(uint32_t reactorId, uint32_t volId)
{
    return (qosDataManager->DequeueVolumeUbio(reactorId, volId));
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
VolumeIoSmartPtr
QosManager::PeekVolumeUbio(uint32_t reactorId, uint32_t volId)
{
    return (qosDataManager->PeekVolumeUbio(reactorId, volId));
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
UbioSmartPtr
QosManager::DequeueEventUbio(uint32_t id, uint32_t event)
{
    return (qosDataManager->DequeueEventUbio(id, event));
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
UbioSmartPtr
QosManager::PeekEventUbio(uint32_t id, uint32_t event)
{
    return (qosDataManager->PeekEventUbio(id, event));
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::EnqueueEventParams(uint32_t workerId, BackendEvent event, event_qos_params& event_param)
{
    qosDataManager->EnqueueEventParams(workerId, event, event_param);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
event_qos_params
QosManager::DequeueEventParams(uint32_t workerId, BackendEvent event)
{
    return (qosDataManager->DequeueEventParams(workerId, event));
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::EnqueueEventUbio(uint32_t id, BackendEvent event, UbioSmartPtr ubio)
{
    qosDataManager->EnqueueEventUbio(id, event, ubio);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::SetVolumeWeight(uint32_t reactor, uint32_t volId, uint32_t weight)
{
    volReactorWeight[reactor][volId] = weight;
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
    return qosDataManager->SetEventWeightWRR(eventId, weight);
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
    return qosDataManager->GetEventWeightWRR(eventId);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
uint32_t
QosManager::GetVolumeWeight(uint32_t reactor, uint32_t volId)
{
    return volReactorWeight[reactor][volId];
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
uint32_t
QosManager::CalcEventWeight(uint32_t weight)
{
    return (weight * (M_KBYTES / IBOF_QOS_TIMESLICE_IN_USEC) * M_KBYTES);
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
void
QosManager::InitialiseUsedStripeCnt(void)
{
    usedStripeCnt = 0;
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
    return nqnVolumeMap[nqnId];
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

bool
QosManager::VolumeCreated(std::string volName, int volID, uint64_t volSizeByte,
    uint64_t maxiops, uint64_t maxbw)
{
    std::unique_lock<std::mutex> uniqueLock(policyUpdateLock);
    uint64_t newBw = maxbw;
    if (0 == newBw)
    {
        newBw = MAX_QOS_LIMIT;
    }
    else
    {
        newBw = (maxbw * M_KBYTES * M_KBYTES) / IBOF_QOS_TIMESLICE_IN_USEC;
    }
    maxVolWeightCli[volID] = newBw;
    wtUpdate[volID] = 1;
    return true;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

bool
QosManager::VolumeDeleted(std::string volName, int volID, uint64_t volSizeByte)
{
    return true;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

bool
QosManager::VolumeMounted(std::string volName, std::string subnqn, int volID, uint64_t volSizeByte,
    uint64_t maxiops, uint64_t maxbw)
{
    return true;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

bool
QosManager::VolumeUnmounted(std::string volName, int volID)
{
#if defined QOS_ENABLED_FE
    uint32_t nqnId = get_attached_subsystem_id(volName.c_str());
    uint32_t i = 0;
    for (i = 0; i < nqnVolumeMap[nqnId].size(); i++)
    {
        if (volID == nqnVolumeMap[nqnId][i])
        {
            nqnVolumeMap[nqnId].erase(nqnVolumeMap[nqnId].begin() + i);
        }
    }
#endif
    return true;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

bool
QosManager::VolumeLoaded(std::string volName, int id, uint64_t totalSize,
    uint64_t maxiops, uint64_t maxbw)
{
    return true;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

bool
QosManager::VolumeUpdated(std::string volName, int volID, uint64_t maxiops,
    uint64_t maxbw)
{
    std::unique_lock<std::mutex> uniqueLock(policyUpdateLock);
    uint64_t newBw = maxbw;
    if (0 == newBw)
    {
        newBw = MAX_QOS_LIMIT;
    }
    else
    {
        newBw = (maxbw * M_KBYTES * M_KBYTES) / IBOF_QOS_TIMESLICE_IN_USEC;
    }
    maxVolWeightCli[volID] = newBw;
    wtUpdate[volID] = 1;
    return true;
}

void
QosManager::VolumeDetached(vector<int> volList)
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
QosManager::UpdateSubsystemToVolumeMap(uint32_t nqnId, uint32_t volId)
{
    nqnVolumeMap[nqnId].push_back(volId);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosManager::ResetEventRateLimit(uint32_t id, BackendEvent event)
{
    if (remainingEventLimit[id][event] > 0)
    {
        remainingEventLimit[id][event] = GetEventWeight(event);
    }
    else
        remainingEventLimit[id][event] = remainingEventLimit[id][event] + GetEventWeight(event);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosManager::ResetRateLimit(uint32_t reactor, int volId)
{
    if (remainingLimit[reactor][volId] > 0)
    {
        remainingLimit[reactor][volId] = GetVolumeWeight(reactor, volId);
    }
    else
    {
        remainingLimit[reactor][volId] = remainingLimit[reactor][volId] + GetVolumeWeight(reactor, volId);
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
QosManager::_VolumeQosPoller(void* arg1)
{
    uint32_t retVal = 0;
#if defined QOS_ENABLED_FE
    QosManager* qosMgr = QosManagerSingleton::Instance();
    uint32_t reactor = spdk_env_get_current_core();
    uint64_t now = spdk_get_ticks();
    VolumeIoSmartPtr queuedVolumeIo;
    uint64_t currentBW = 0;
    struct poller_structure* param = (struct poller_structure*)arg1;

    uint64_t next_tick = param->nextTimeStamp;
    std::vector<int> volList;
    if (now < next_tick)
    {
        return 0;
    }
    param->nextTimeStamp = now + param->qosTimeSlice;
    for (uint32_t subsys = 0; subsys < M_MAX_SUBSYSTEMS; subsys++)
    {
        // Assumption is 1 Vol per Subsystem
        if (spdk_nvmf_get_reactor_subsystem_mapping(reactor, subsys) != M_INVALID_SUBSYSTEM)
        {
            volList = qosMgr->GetVolumeFromActiveSubsystem(subsys);
            for (uint32_t i = 0; i < volList.size(); i++)
            {
                uint32_t volId = volList[i];
                uint32_t oldValue = 0;
                if (true == qosMgr->policyManager->VolumeMinimumPolicyInEffect())
                {
                    qosMgr->volumeQosParam[reactor][volList[i]].valid = M_VALID_ENTRY;
                    oldValue = qosMgr->volumeQosParam[reactor][volId].currentBW;
                    qosMgr->EnqueueVolumeParams(reactor, volList[i], qosMgr->volumeQosParam[reactor][volList[i]]);
                    qosMgr->volumeQosParam[reactor][volId].currentBW = oldValue;
                }
                currentBW = 0;
                qosMgr->ResetRateLimit(reactor, volId);
                while (1)
                {
                    if (qosMgr->RateLimit(reactor, volId) == true)
                        break;
                    queuedVolumeIo = qosMgr->PeekVolumeUbio(reactor, volId);
                    if (queuedVolumeIo == nullptr)
                    {
                        break;
                    }
                    currentBW = currentBW + queuedVolumeIo->GetMemSize();
                    queuedVolumeIo = qosMgr->DequeueVolumeUbio(reactor, volId);
                    AIO aio;
                    aio.SubmitAsyncIO(queuedVolumeIo);
                    qosMgr->UpdateRateLimit(reactor, volId, queuedVolumeIo->GetMemSize());
                }
                qosMgr->volumeQosParam[reactor][volId].currentBW = currentBW;
            }
        }
    }
#endif
    return retVal;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int
QosManager::EventQosPoller(uint32_t id)
{
    int32_t retVal = 0;
    uint64_t now = spdk_get_ticks();
    UbioSmartPtr ubio;
    int completions = 0;
    uint64_t currentBW = 0;
    struct poller_structure* param = &eventPollStructure[id];
    uint32_t workerId = id;

    uint64_t next_tick = param->nextTimeStamp;
    uint32_t event = 0;
    if (now < next_tick)
    {
        return 0;
    }

    param->nextTimeStamp = now + param->qosTimeSlice;

    for (event = BackendEvent_Start; event < BackendEvent_End; event++)
    {
        if (true == policyManager->VolumeMinimumPolicyInEffect())
        {
            eventQosParam[workerId][event].valid = M_VALID_ENTRY;
            EnqueueEventParams(workerId, (BackendEvent)event, eventQosParam[workerId][event]);
        }
        currentBW = 0;
        ResetEventRateLimit(workerId, (BackendEvent)event);
        while (1)
        {
            if (EventRateLimit(workerId, (BackendEvent)event) == true)
                break;
            ubio = PeekEventUbio(workerId, event);
            if (ubio == nullptr)
            {
                break;
            }
            currentBW = currentBW + ubio->GetSize();
            ubio = DequeueEventUbio(workerId, event);
            assert(ubio != nullptr);
            UBlockDevice* device = ubio->GetUBlock();
            completions = device->SubmitAsyncIO(ubio);
            retVal += completions;
            UpdateEventRateLimit(workerId, (BackendEvent)event, ubio->GetSize());
        }
        eventQosParam[workerId][event].currentBW = currentBW;
    }
    return retVal;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
BackendEvent
QosManager::IdentifyEventType(UbioSmartPtr ubio)
{
    return (ubio->GetEventType());
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::EventParamterInit(uint32_t id)
{
    eventPollStructure[id].qosTimeSlice = IBOF_QOS_TIMESLICE_IN_USEC * spdk_get_ticks_hz() / SPDK_SEC_TO_USEC;
    eventPollStructure[id].nextTimeStamp = spdk_get_ticks() + eventPollStructure[id].qosTimeSlice;
    eventPollStructure[id].workerId = id;
    eventDeficietWeight[BackendEvent_Flush] = M_DEFAULT_DEFICIET_WEIGHT;
    eventDeficietWeight[BackendEvent_GC] = M_DEFAULT_DEFICIET_WEIGHT;
    eventDeficietWeight[BackendEvent_UserdataRebuild] = M_DEFAULT_DEFICIET_WEIGHT;
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
uint32_t
QosManager::GetEventWeight(uint32_t event)
{
    return qosDataManager->GetEventWeight(event);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::SetEventWeight(uint32_t event, uint32_t weight)
{
    return qosDataManager->SetEventWeight(event, weight);
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
    return policyManager->UpdateVolumePolicy(volId, policy);
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
    return policyManager->GetVolumePolicy(volId);
}

/* --------------------------------------------------------------------------*/
/**
  * @Synopsis
  *
  * @Returns
  */
/* --------------------------------------------------------------------------*/
EventPriority
QosManager::GetBackendEventPriority(BackendEvent event)
{
    return eventPriorityMap[event];
}

/* --------------------------------------------------------------------------*/
/**
  * @Synopsis
  *
  * @Returns
  */
/* --------------------------------------------------------------------------*/
void
QosManager::SetBackendEventPriority(BackendEvent event, EventPriority priority)
{
    eventPriorityMap[event] = priority;
}

/* --------------------------------------------------------------------------*/
/**
  * @Synopsis
  *
  * @Returns
  */
/* --------------------------------------------------------------------------*/
uint32_t
QosManager::GetPriorityWeight(EventPriority priority)
{
    return priorityWeightMap[priority];
}

/* --------------------------------------------------------------------------*/
/**
  * @Synopsis
  *
  * @Returns
  */
/* --------------------------------------------------------------------------*/
void
QosManager::SetPriorityWeight(EventPriority priority, uint32_t weight)
{
    priorityWeightMap[priority] = weight;
}

/* --------------------------------------------------------------------------*/
/**
  * @Synopsis
  *
  * @Returns
  */
/* --------------------------------------------------------------------------*/
int
QosManager::SetEventPolicy(BackendEvent event, EventPriority priority, uint32_t weight)
{
    return policyManager->SetEventPolicy(event, priority, weight);
}

int
QosManager::SetEventPolicy(std::string eventName, std::string perfImpact)
{
    BackendEvent bEvent = GetEventId(eventName);
    int ret = QosReturnCode::SUCCESS;

    if (perfImpact == "high")
    {
        ret = policyManager->SetEventPolicy(
            bEvent, EventPriority::EventPriority_Critical, 1);
    }
    else if (perfImpact == "medium")
    {
        ret = policyManager->SetEventPolicy(
            bEvent, EventPriority::EventPriority_High, 2);
    }
    else if (perfImpact == "low")
    {
        ret = policyManager->SetEventPolicy(
            bEvent, EventPriority::EventPriority_Low, 3);
    }
    else
    {
        return QosReturnCode::FAILURE;
    }

    IBOF_TRACE_INFO((int)IBOF_EVENT_ID::QOS_SET_EVENT_POLICY,
        "QoS SetEventPolicy for " + eventName + " to " + perfImpact + ", result: {} ", ret);
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
QosManager::ResetEventPolicy(void)
{
    policyManager->ResetEventPolicy();
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

uint64_t
QosManager::GetVolBWLog(int volId)
{
    uint64_t count = volBwLog[volId];
    uint64_t oldCount = oldvolBwLog[volId];
    oldvolBwLog[volId] = count;
    if (oldCount > count)
    {
        return (count + (ULLONG_MAX - oldCount));
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

void
QosManager::LogVolBw(uint32_t volId, uint64_t size)
{
    volBwLog[volId] += size;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::UpdateFreeSegmentCnt(uint32_t count)
{
    freeSegmentCnt = count;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

uint32_t
QosManager::GetFreeSegmentCnt(void)
{
    return freeSegmentCnt;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosManager::UpdateRemainingUserRebuildSegmentCnt(uint32_t count)
{
    rebuildPendingDataSegments = count;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosManager::UpdateRemainingMetaRebuildStripeCnt(uint32_t count)
{
    rebuildPendingMetaStripes = count;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

uint32_t
QosManager::GetRemainingUserRebuildSegmentCnt(void)
{
    return rebuildPendingDataSegments;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

uint32_t
QosManager::GetRemainingMetaRebuildStripeCnt(void)
{
    return rebuildPendingMetaStripes;
}
} // namespace ibofos

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
#include "src/include/pos_event_id.hpp"
#include "src/io/frontend_io/aio.h"
#include "src/io_scheduler/io_worker.h"
#include "src/logger/logger.h"
#include "src/network/nvmf_volume_ibof.hpp"
#include "src/spdk_wrapper/event_framework_api.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/io/frontend_io/aio_submission_adapter.h"
namespace pos
{
// SPDK MANAGER INITIALIZATIONS
#if defined QOS_ENABLED_FE
std::atomic<bool> QosSpdkManager::registerQosPollerDone(false);
std::atomic<bool> QosSpdkManager::unregistrationComplete(false);
#endif

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
spdk_poller*
QosSpdkManager::GetSpdkPoller(uint32_t reactor)
{
    return spdkPollers[reactor];
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosSpdkManager::PollerUnregister(void* arg1, void* arg2)
{
#if defined QOS_ENABLED_FE
    QosSpdkManager* spdkManager = static_cast<QosSpdkManager*>(arg1);
    uint32_t reactor = spdkManager->GetReactorId();
    spdk_poller* poller = spdkManager->GetSpdkPoller(reactor);
    EventFrameworkApi::SpdkPollerUnregister(&poller);
    if ((EventFrameworkApi::IsLastReactorNow()))
    {
        unregistrationComplete = true;
    }
    else
    {
        uint32_t nextReactor = EventFrameworkApi::GetNextReactor();
        spdkManager->SetReactorId(nextReactor);
        EventFrameworkApi::SendSpdkEvent(nextReactor, PollerUnregister, spdkManager, nullptr);
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
QosSpdkManager::Finalize(void)
{
#if defined QOS_ENABLED_FE
    reactorId = EventFrameworkApi::GetFirstReactor();
    bool succeeded = EventFrameworkApi::SendSpdkEvent(reactorId, QosSpdkManager::PollerUnregister, this, nullptr);
    if (unlikely(false == succeeded))
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::QOSMGR_REGISTER_POLLER_FAILED;
        POS_TRACE_ERROR(static_cast<int>(eventId), PosEventId::GetString(eventId), reactorId);
    }
    while (QosSpdkManager::unregistrationComplete == false)
    {
        usleep(1);
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
QosManager::_Finalize(void)
{
#if defined QOS_ENABLED_FE
    spdkManager->Finalize();
#endif
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
QosSpdkManager::_SetupQosReactorPoller(void)
{
#if defined QOS_ENABLED_FE
    EventFrameworkApi::SpdkNvmfInitializeReactorSubsystemMapping();
    registerQosPollerDone = false;
    reactorId = EventFrameworkApi::GetFirstReactor();
    bool succeeded = EventFrameworkApi::SendSpdkEvent(reactorId, RegisterQosPoller, this, nullptr);
    if (unlikely(false == succeeded))
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::QOSMGR_REGISTER_POLLER_FAILED;
        POS_TRACE_ERROR(static_cast<int>(eventId), PosEventId::GetString(eventId), reactorId);
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
QosSpdkManager::UpdateReactorData(uint32_t reactor, poller_structure data)
{
    reactorData[reactor] = data;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosSpdkManager::UpdateSpdkPoller(uint32_t reactor, spdk_poller* spdkPoller)
{
    spdkPollers[reactor] = spdkPoller;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
poller_structure&
QosSpdkManager::GetReactorData(uint32_t reactor)
{
    return reactorData[reactor];
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosSpdkManager::RegisterQosPoller(void* arg1, void* arg2)
{
#if defined QOS_ENABLED_FE
    QosSpdkManager* spdkManager = static_cast<QosSpdkManager*>(arg1);
    uint32_t reactor = spdkManager->GetReactorId();
    poller_structure& pollerData = spdkManager->GetReactorData(reactor);
    pollerData.qosTimeSlice = IBOF_QOS_TIMESLICE_IN_USEC * EventFrameworkApi::SpdkGetTicksHz() / SPDK_SEC_TO_USEC;
    pollerData.nextTimeStamp = EventFrameworkApi::SpdkGetTicks() + pollerData.qosTimeSlice;
    pollerData.id = reactor;

    std::string pollerName = "volume_qos_poller_" + std::to_string(reactor);
    spdk_poller* poller = static_cast<spdk_poller*> (EventFrameworkApi::SpdkPollerRegister(spdkManager->SpdkVolumeQosPoller, &pollerData, 0, pollerName));

    spdkManager->UpdateSpdkPoller(reactor, poller);
    if (EventFrameworkApi::IsLastReactorNow())
    {
        registerQosPollerDone = true;
    }
    else
    {
        uint32_t nextReactor = EventFrameworkApi::GetNextReactor();
        spdkManager->SetReactorId(nextReactor);
        bool success = EventFrameworkApi::SendSpdkEvent(nextReactor, RegisterQosPoller, spdkManager, nullptr);
        if (unlikely(false == success))
        {
            POS_EVENT_ID eventId = POS_EVENT_ID::QOSMGR_REGISTER_POLLER_FAILED;
            POS_TRACE_ERROR(static_cast<int>(eventId), PosEventId::GetString(eventId), nextReactor);
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
QosEventManager::_EventRateLimit(uint32_t id, BackendEvent event)
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
QosVolumeManager::_RateLimit(uint32_t reactor, int volId)
{
    if (remainingLimit[reactor][volId] > 0)
    {
        return false;
    }
    else
    {
        return true;
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
QosEventManager::_UpdateEventRateLimit(uint32_t id, BackendEvent event, uint64_t size)
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
QosVolumeManager::_UpdateRateLimit(uint32_t reactor, int volId, uint64_t size)
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
QosManager::AioSubmitAsyncIO(IbofIoSubmissionAdapter* aioSubmission, ibof_io* volIo)
{
    qosVolumeManager->AioSubmitAsyncIO(aioSubmission, volIo);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::AioSubmitAsyncIO(IbofIoSubmissionAdapter* aioSubmission, ibof_io* volIo)
{
#if defined QOS_ENABLED_FE
    uint32_t reactorId = EventFrameworkApi::GetCurrentReactor();
    uint32_t volId = volIo->volume_id;
    uint64_t currentBw = 0;
    currentBw = volumeQosParam[reactorId][volId].currentBW;
    if ((pendingVolumeIO[reactorId][volId] == 0) && (_RateLimit(reactorId, volId) == false))
    {
        currentBw = currentBw + volIo->length;
        aioSubmission->Do(volIo);
        _UpdateRateLimit(reactorId, volId, volIo->length);
    }
    else
    {
        pendingVolumeIO[reactorId][volId]++;
        _EnqueueVolumeUbio(reactorId, volId, volIo);
        while (1)
        {
            if (_RateLimit(reactorId, volId) == true)
            {
                break;
            }
            ibof_io* queuedVolumeIo = nullptr;
            queuedVolumeIo = _DequeueVolumeUbio(reactorId, volId);
            if (queuedVolumeIo == nullptr)
            {
                break;
            }
            currentBw = currentBw + queuedVolumeIo->length;
            pendingVolumeIO[reactorId][volId]--;
            aioSubmission->Do(queuedVolumeIo);
            _UpdateRateLimit(reactorId, volId, queuedVolumeIo->length);
        }
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
QosManager::SubmitAsyncIO(SubmissionAdapter* ioSubmission,
    SubmissionNotifier* submissionNotifier, uint32_t id, UbioSmartPtr ubio)
{
    qosEventManager->SubmitAsyncIO(ioSubmission,
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
QosEventManager::SubmitAsyncIO(SubmissionAdapter* ioSubmission,
    SubmissionNotifier* submissionNotifier, uint32_t id, UbioSmartPtr ubio)
{
    int completionCount = 0;
    BackendEvent event = _IdentifyEventType(ubio);
    uint64_t currentBw = 0;
    if ((BackendEvent_Unknown == event) || (ubio->IsSyncMode() == true))
    {
        completionCount = ioSubmission->Do(ubio);
        submissionNotifier->Do(completionCount);
        return;
    }
    currentBw = eventQosParam[id][event].currentBW;
    if ((0 == pendingEventIO[id][event]) && (false == _EventRateLimit(id, event)))
    {
        currentBw = currentBw + ubio->GetSize();
        completionCount = ioSubmission->Do(ubio);
        submissionNotifier->Do(completionCount);
        _UpdateEventRateLimit(id, event, ubio->GetSize());
    }
    else
    {
        pendingEventIO[id][event]++;
        _EnqueueEventUbio(id, event, ubio);
        while (1)
        {
            if (_EventRateLimit(id, event) == true)
            {
                break;
            }
            UbioSmartPtr queuedUbio = nullptr;
            queuedUbio = _DequeueEventUbio(id, event);
            if (queuedUbio == nullptr)
            {
                break;
            }
            currentBw = currentBw + queuedUbio->GetSize();
            pendingEventIO[id][event]--;
            completionCount = ioSubmission->Do(queuedUbio);
            submissionNotifier->Do(completionCount);
            _UpdateEventRateLimit(id, event, queuedUbio->GetSize());
        }
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
QosManager::QosManager()
: quitQos(false),
  qosThread(nullptr)
{
    spdkManager = new QosSpdkManager();
    policyManager = new QosPolicyManager();
    qosVolumeManager = new QosVolumeManager(policyManager);
    qosEventManager = new QosEventManager(policyManager);
    usedStripeCnt = 0;
    for (uint32_t volId = 0; volId < MAX_VOLUME_COUNT; volId++)
    {
        wtUpdate[volId] = 0;
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosManager::~QosManager()
{
    _Finalize();
    delete qosThread;
    delete spdkManager;
    delete policyManager;
    delete qosVolumeManager;
    delete qosEventManager;
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

    qosThread = new std::thread(&QosManager::_QosWorkerPoller, this);

#if defined QOS_ENABLED_FE
    spdkManager->Initialize();
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
QosSpdkManager::Initialize(void)
{
    _SetupQosReactorPoller();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::_QosWorkerPoller(void)
{
    sched_setaffinity(0, sizeof(cpuSet), &cpuSet);
    pthread_setname_np(pthread_self(), "QoSWorker");
#if defined QOS_ENABLED_FE
    std::vector<int> volList;
    uint32_t activeConnectionCtr = 0;
    uint64_t endTimeStamp = M_RESET_TO_ZERO;
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
            for (uint32_t reactor = 0; reactor < M_MAX_REACTORS; reactor++)
            {
                for (uint32_t subsys = 0; subsys < M_MAX_SUBSYSTEMS; subsys++)
                {
                    uint32_t reactorSubsystemMapping = EventFrameworkApi::SpdkNvmfGetReactorSubsystemMapping(reactor, subsys);
                    if (reactorSubsystemMapping != M_INVALID_SUBSYSTEM)
                    {
                        volList = _GetVolumeFromActiveSubsystem(subsys);
                        for (uint32_t index = 0; index < volList.size(); index++)
                        {
                            activeConnectionCtr++;
                            uint32_t volumeId = volList[index];
                            _UpdatePolicyManagerActiveVolumeData(reactor, subsys, volumeId);
                        }
                    }
                }
            }
            if ((activeConnectionCtr == 0) || (false == policyManager->VolumeMinimumPolicyInEffect()))
            {
                break;
            }
        } while (!((activeConnectionCtr <= policyManager->GetActiveConnectionsCount()) && activeConnectionCtr));
        _UpdateMaxVolumeWeightReactorVolume();
        if (activeConnectionCtr == 0)
        {
            policyManager->ResetAll();
            continue;
        }
        if (false == policyManager->VolumeMinimumPolicyInEffect())
        {
            continue;
        }
        else
        {
            _HandleVolumeMinimumPolicy(endTimeStamp);
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
QosManager::_UpdatePolicyManagerActiveVolumeData(uint32_t reactor, uint32_t subsys, uint32_t volumeId)
{
    std::pair<uint32_t, uint32_t> reactorVolumePair;
    map<uint32_t, uint32_t> volCount;
    map<uint32_t, uint32_t> reactorCount;
    uint32_t reactorSubsystemMapping = EventFrameworkApi::SpdkNvmfGetReactorSubsystemMapping(reactor, subsys);
    totalConnection[volumeId] = 0;
    volCount = reactorVolMap[reactor];
    volCount[volumeId] += reactorSubsystemMapping;
    reactorVolMap[reactor] = volCount;

    reactorCount = volReactorMap[volumeId];
    reactorCount[reactor] += reactorSubsystemMapping;
    volReactorMap[volumeId] = reactorCount;

    reactorVolumePair = std::make_pair(reactor, volumeId);
    policyManager->InsertActiveVolumeMap(pair<int, int>(volumeId, 1));
    if (true == policyManager->VolumeMinimumPolicyInEffect())
    {
        volume_qos_params volParam = DequeueVolumeParams(reactor, volumeId);
        if (volParam.valid == M_VALID_ENTRY)
        {
            volParam.valid = M_INVALID_ENTRY;
            std::pair<std::pair<uint32_t, uint32_t>, uint32_t> reactorVolPair = std::make_pair(reactorVolumePair, 1);
            policyManager->InsertActiveConnection(reactorVolPair);
            policyManager->UpdateVolumeParam(volumeId, volParam);
            policyManager->FillVolumeState(volumeId);
        }
        else
        {
            policyManager->UpdateVolumeParam(volumeId, volParam);
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
QosManager::_UpdateMaxVolumeWeightReactorVolume(void)
{
    std::map<int, int>& activeVolMap = policyManager->GetActiveVolumeMap();
    for (map<int, int>::iterator it = activeVolMap.begin(); it != activeVolMap.end(); it++)
    {
        uint32_t volId = it->first;
        {
            std::unique_lock<std::mutex> uniqueLock(policyUpdateLock);
            if (wtUpdate[volId] == 1)
            {
                wtUpdate[volId] = 0;
                uint64_t maxWeight = maxVolWeightCli[volId];
                maxVolWeight[volId] = maxWeight;
            }
            for (map<uint32_t, uint32_t>::iterator it = volReactorMap[volId].begin(); it != volReactorMap[volId].end(); ++it)
            {
                totalConnection[volId] += it->second;
            }
            for (map<uint32_t, uint32_t>::iterator it = volReactorMap[volId].begin(); it != volReactorMap[volId].end(); ++it)
            {
                uint64_t reactorVolWeight = (maxVolWeight[volId] * (it->second)) / totalConnection[volId];
                SetVolumeWeight(it->first, volId, reactorVolWeight);
            }
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
QosManager::_HandleVolumeMinimumPolicy(uint64_t& endTimeStamp)
{
    int triggerQos = 0;
    uint64_t startTimeStamp = M_RESET_TO_ZERO;
    double elapsedMs = M_RESET_TO_ZERO;
    uint64_t msTimeStamp = IBOF_QOS_TIMESLICE_IN_USEC * EventFrameworkApi::SpdkGetTicksHz() / SPDK_SEC_TO_USEC;

    startTimeStamp = EventFrameworkApi::SpdkGetTicks();

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
            event_qos_params eventParam;
            do
            {
                eventParam.valid = M_INVALID_ENTRY;
                eventParam = DequeueEventParams(workerId, (BackendEvent)event);
                if (eventParam.valid == M_VALID_ENTRY)
                {
                    policyManager->UpdateEventParam(event, eventParam);
                    policyManager->FillEventState((BackendEvent)event);
                }
            } while (eventParam.valid == M_VALID_ENTRY);
        }
    }
    if (policyManager->GetMinVolumeWorkloadType() == QosWorkloadType_Read)
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
    policyManager->ActiveVolumeClear();
    policyManager->ActiveEventClear();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::_EnqueueVolumeParams(uint32_t reactor, uint32_t volId, volume_qos_params& volume_param)
{
    std::unique_lock<std::mutex> uniqueLock(volQueueLock[reactor][volId]);
    volumesParamsQueue[reactor][volId].push(volume_param);
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
    return qosVolumeManager->DequeueVolumeParams(reactor, volId);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
volume_qos_params
QosVolumeManager::DequeueVolumeParams(uint32_t reactor, uint32_t volId)
{
    volume_qos_params ret;
    ret.valid = M_INVALID_ENTRY;
    std::unique_lock<std::mutex> uniqueLock(volQueueLock[reactor][volId]);
    if (volumesParamsQueue[reactor][volId].empty() == false)
    {
        ret = volumesParamsQueue[reactor][volId].front();
        volumesParamsQueue[reactor][volId].pop();
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
QosVolumeManager::_EnqueueVolumeUbio(uint32_t reactorId, uint32_t volId, ibof_io* io)
{
    volumesUbioQueue[reactorId][volId].push(io);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
ibof_io*
QosVolumeManager::_DequeueVolumeUbio(uint32_t reactorId, uint32_t volId)
{
    ibof_io* ret = nullptr;
    if (volumesUbioQueue[reactorId][volId].empty() == false)
    {
        ret = volumesUbioQueue[reactorId][volId].front();
        volumesUbioQueue[reactorId][volId].pop();
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
UbioSmartPtr
QosEventManager::_DequeueEventUbio(uint32_t id, uint32_t event)
{
    UbioSmartPtr ret = nullptr;
    if (eventsUbioQueue[id][event].empty() == false)
    {
        ret = eventsUbioQueue[id][event].front();
        eventsUbioQueue[id][event].pop();
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
QosEventManager::_EnqueueEventParams(uint32_t workerId, BackendEvent event, event_qos_params& event_param)
{
    std::unique_lock<std::mutex> uniqueLock(eventQueueLock[workerId][event]);
    eventsParamsQueue[workerId][event].push(event_param);
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
    return qosEventManager->DequeueEventParams(workerId, event);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
event_qos_params
QosEventManager::DequeueEventParams(uint32_t workerId, BackendEvent event)
{
    event_qos_params ret;
    ret.valid = M_INVALID_ENTRY;
    std::unique_lock<std::mutex> uniqueLock(eventQueueLock[workerId][event]);
    if (eventsParamsQueue[workerId][event].empty() == false)
    {
        ret = eventsParamsQueue[workerId][event].front();
        eventsParamsQueue[workerId][event].pop();
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
QosEventManager::_EnqueueEventUbio(uint32_t id, BackendEvent event, UbioSmartPtr ubio)
{
    eventsUbioQueue[id][event].push(ubio);
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
    qosVolumeManager->SetVolumeWeight(reactor, volId, weight);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::SetVolumeWeight(uint32_t reactor, uint32_t volId, uint32_t weight)
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
    qosEventManager->SetEventWeightWRR(eventId, weight);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosEventManager::SetEventWeightWRR(BackendEvent eventId, int64_t weight)
{
    eventWeightWRR[eventId] = weight;
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
int64_t
QosEventManager::GetEventWeightWRR(BackendEvent eventId)
{
    return eventWeightWRR[eventId];
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
    return qosVolumeManager->GetVolumeWeight(reactor, volId);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
uint32_t
QosVolumeManager::GetVolumeWeight(uint32_t reactor, uint32_t volId)
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
QosManager::_GetVolumeFromActiveSubsystem(uint32_t nqnId)
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
QosVolumeManager::_UpdateVolumeMaxQos(int volId, uint64_t maxiops, uint64_t maxbw)
{
    uint64_t newBw = maxbw;
    if (0 == newBw)
    {
        newBw = MAX_QOS_LIMIT;
    }
    else
    {
        newBw = (maxbw * M_KBYTES * M_KBYTES) / IBOF_QOS_TIMESLICE_IN_USEC;
    }
    QosManagerSingleton::Instance()->SetMaxVolWeightCli(volId, newBw);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bool
QosVolumeManager::VolumeCreated(std::string volName, int volID, uint64_t volSizeByte,
    uint64_t maxiops, uint64_t maxbw, std::string arrayName)
{
    _UpdateVolumeMaxQos(volID, maxiops, maxbw);
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
QosVolumeManager::VolumeDeleted(std::string volName, int volID, uint64_t volSizeByte, std::string arrayName)
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
QosVolumeManager::VolumeMounted(std::string volName, std::string subnqn, int volID, uint64_t volSizeByte,
    uint64_t maxiops, uint64_t maxbw, std::string arrayName)
{
    _UpdateVolumeMaxQos(volID, maxiops, maxbw);
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
QosVolumeManager::VolumeUnmounted(std::string volName, int volID, std::string arrayName)
{
#if defined QOS_ENABLED_FE
    uint32_t nqnId = EventFrameworkApi::GetAttachedSubsystemId(volName.c_str());
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
QosVolumeManager::VolumeLoaded(std::string volName, int id, uint64_t totalSize,
    uint64_t maxiops, uint64_t maxbw, std::string arrayName)
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
QosVolumeManager::VolumeUpdated(std::string volName, int volID, uint64_t maxiops,
    uint64_t maxbw, std::string arrayName)
{
    _UpdateVolumeMaxQos(volID, maxiops, maxbw);
    return true;
}

void
QosVolumeManager::VolumeDetached(vector<int> volList, std::string arrayName)
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
    qosVolumeManager->UpdateSubsystemToVolumeMap(nqnId, volId);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
QosEventManager::_ResetEventRateLimit(uint32_t id, BackendEvent event)
{
    if (remainingEventLimit[id][event] > 0)
    {
        remainingEventLimit[id][event] = eventWeight[event];
    }
    else
        remainingEventLimit[id][event] = remainingEventLimit[id][event] + eventWeight[event];
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::_ResetRateLimit(uint32_t reactor, int volId)
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
QosSpdkManager::SpdkVolumeQosPoller(void* arg1)
{
    poller_structure* param = static_cast<poller_structure*>(arg1);
    AioSubmissionAdapter aioSubmission;
    return QosManagerSingleton::Instance()->VolumeQosPoller(param, &aioSubmission);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int
QosManager::VolumeQosPoller(poller_structure* param, IbofIoSubmissionAdapter *aioSubmission)
{
#if defined QOS_ENABLED_FE
    return qosVolumeManager->VolumeQosPoller(param, aioSubmission);
#else
    return 0;
#endif
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int
QosVolumeManager::VolumeQosPoller(struct poller_structure* param, IbofIoSubmissionAdapter *aioSubmission)
{
    uint32_t retVal = 0;
#if defined QOS_ENABLED_FE
    uint32_t reactor = param->id;
    uint64_t now = EventFrameworkApi::SpdkGetTicks();
    ibof_io* queuedVolumeIo = nullptr;
    uint64_t currentBW = 0;
    uint64_t next_tick = param->nextTimeStamp;
    std::vector<int> volList;
    if (now < next_tick)
    {
        return 0;
    }
    param->nextTimeStamp = now + param->qosTimeSlice;
    for (uint32_t subsys = 0; subsys < M_MAX_SUBSYSTEMS; subsys++)
    {
        if (EventFrameworkApi::SpdkNvmfGetReactorSubsystemMapping(reactor, subsys) != M_INVALID_SUBSYSTEM)
        {
            volList = GetVolumeFromActiveSubsystem(subsys);
            for (uint32_t i = 0; i < volList.size(); i++)
            {
                uint32_t volId = volList[i];
                uint32_t oldValue = 0;
                if (true == qosPolicyManager->VolumeMinimumPolicyInEffect())
                {
                    volumeQosParam[reactor][volList[i]].valid = M_VALID_ENTRY;
                    oldValue = volumeQosParam[reactor][volId].currentBW;
                    _EnqueueVolumeParams(reactor, volList[i], volumeQosParam[reactor][volList[i]]);
                    volumeQosParam[reactor][volId].currentBW = oldValue;
                }
                currentBW = 0;
                _ResetRateLimit(reactor, volId);
                while (1)
                {
                    if (_RateLimit(reactor, volId) == true)
                    {
                        break;
                    }
                    queuedVolumeIo = _DequeueVolumeUbio(reactor, volId);
                    if (queuedVolumeIo == nullptr)
                    {
                        break;
                    }
                    currentBW = currentBW + queuedVolumeIo->length;
                    pendingVolumeIO[reactor][volId]--;
                    aioSubmission->Do(queuedVolumeIo);
                    _UpdateRateLimit(reactor, volId, queuedVolumeIo->length);
                }
                volumeQosParam[reactor][volId].currentBW = currentBW;
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
QosManager::EventQosPoller(uint32_t id, SubmissionAdapter* ioSubmission)
{
    return qosEventManager->EventQosPoller(id, ioSubmission);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int
QosEventManager::EventQosPoller(uint32_t id, SubmissionAdapter* ioSubmission)
{
    int32_t retVal = 0;
    uint64_t now = EventFrameworkApi::SpdkGetTicks();
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
        if (true == qosPolicyManager->VolumeMinimumPolicyInEffect())
        {
            eventQosParam[workerId][event].valid = M_VALID_ENTRY;
            _EnqueueEventParams(workerId, (BackendEvent)event, eventQosParam[workerId][event]);
        }
        currentBW = 0;
        _ResetEventRateLimit(workerId, (BackendEvent)event);
        while (1)
        {
            if (_EventRateLimit(workerId, (BackendEvent)event) == true)
            {
                break;
            }
            UbioSmartPtr queuedUbio = nullptr;
            queuedUbio = _DequeueEventUbio(workerId, event);
            if (queuedUbio == nullptr)
            {
                break;
            }
            currentBW = currentBW + queuedUbio->GetSize();
            pendingEventIO[workerId][event]--;
            completions = ioSubmission->Do(queuedUbio);
            retVal += completions;
            _UpdateEventRateLimit(workerId, (BackendEvent)event, queuedUbio->GetSize());
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
QosEventManager::_IdentifyEventType(UbioSmartPtr ubio)
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
QosEventManager::_EventParamterInit(uint32_t id)
{
    eventPollStructure[id].qosTimeSlice = IBOF_QOS_TIMESLICE_IN_USEC * EventFrameworkApi::SpdkGetTicksHz() / SPDK_SEC_TO_USEC;
    eventPollStructure[id].nextTimeStamp = EventFrameworkApi::SpdkGetTicks() + eventPollStructure[id].qosTimeSlice;
    eventPollStructure[id].id = id;
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
    return qosEventManager->GetEventWeight(event);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
uint32_t
QosEventManager::GetEventWeight(uint32_t event)
{
    return eventWeight[event];
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
    return qosEventManager->SetEventWeight(event, weight);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosEventManager::SetEventWeight(uint32_t event, uint32_t weight)
{
    eventWeight[event] = weight;
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

    POS_TRACE_INFO((int)POS_EVENT_ID::QOS_SET_EVENT_POLICY,
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

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
BackendEvent
QosManager::GetEventId(string eventName)
{
    return eventDict[eventName];
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosManager::SetMaxVolWeightCli(uint32_t volId, uint64_t weight)
{
    std::unique_lock<std::mutex> uniqueLock(policyUpdateLock);
    maxVolWeightCli[volId] = weight;
    wtUpdate[volId] = 1;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
uint64_t
QosManager::GetMaxVolumeWeight(uint32_t volId)
{
    return maxVolWeight[volId];
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
std::map<uint32_t, uint32_t>&
QosManager::GetVolumeReactorMap(uint32_t volId)
{
    return volReactorMap[volId];
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
uint32_t
QosManager::GetVolumeTotalConnection(uint32_t volId)
{
    return totalConnection[volId];
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosVolumeManager::QosVolumeManager(QosPolicyManager* policyMgr)
: VolumeEvent("QosManager", "")
{
    qosPolicyManager = policyMgr;
    for (uint32_t reactor = 0; reactor < M_MAX_REACTORS; reactor++)
    {
        for (uint32_t volId = 0; volId < MAX_VOLUME_COUNT; volId++)
        {
            pendingVolumeIO[reactor][volId] = 0;
            volReactorWeight[reactor][volId] = MAX_QOS_LIMIT;
            remainingLimit[reactor][volId] = MAX_QOS_LIMIT;
        }
    }

    VolumeEventPublisherSingleton::Instance()->RegisterSubscriber(this, "");
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosVolumeManager::~QosVolumeManager(void)
{
    VolumeEventPublisherSingleton::Instance()->RemoveSubscriber(this, "");
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::UpdateSubsystemToVolumeMap(uint32_t nqnId, uint32_t volId)
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
std::vector<int>
QosVolumeManager::GetVolumeFromActiveSubsystem(uint32_t nqnId)
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
QosEventManager::QosEventManager(QosPolicyManager* policyMgr)
{
    qosPolicyManager = policyMgr;

    for (uint32_t worker = 0; worker < MAX_IO_WORKER; worker++)
    {
        _EventParamterInit(worker);
        for (uint32_t eventId = BackendEvent_Start; eventId < BackendEvent_Count; eventId++)
        {
            pendingEventIO[worker][eventId] = 0;
            remainingEventLimit[worker][eventId] = MAX_QOS_LIMIT;
        }
    }
    for (uint32_t eventId = BackendEvent_Start; eventId < BackendEvent_Count; eventId++)
    {
        eventWeight[eventId] = MAX_QOS_LIMIT;
        eventWeightWRR[eventId] = M_EQUAL_WEIGHT;
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosEventManager::~QosEventManager(void)
{
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosSpdkManager::QosSpdkManager(void)
{
    for (int i = 0; i < M_MAX_REACTORS; i++)
    {
        spdkPollers[i] = nullptr;
    }
    reactorId = M_MAX_REACTORS + 1;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosSpdkManager::~QosSpdkManager(void)
{
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
uint32_t
QosSpdkManager::GetReactorId(void)
{
    return reactorId;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosSpdkManager::SetReactorId(uint32_t id)
{
    reactorId = id;
}

} // namespace pos

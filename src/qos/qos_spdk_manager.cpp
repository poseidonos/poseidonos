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

#include "src/qos/qos_spdk_manager.h"

#include "src/include/pos_event_id.hpp"
#include "src/include/branch_prediction.h"
#include "src/io/frontend_io/aio_submission_adapter.h"
#include "src/logger/logger.h"
#include "src/qos/qos_context.h"
#include "src/qos/qos_manager.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "src/spdk_wrapper/poller_management.h"
#include "src/spdk_wrapper/connection_management.h"

namespace pos
{
// SPDK MANAGER INITIALIZATIONS
std::atomic<bool> QosSpdkManager::registerQosPollerDone(false);
std::atomic<bool> QosSpdkManager::unregistrationComplete(false);

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosSpdkManager::QosSpdkManager(QosContext* qosCtx, bool feQos)
: qosContext(qosCtx),
  reactorId(M_MAX_REACTORS + 1),
  feQosEnabled(feQos)
{
    for (int i = 0; i < M_MAX_REACTORS; i++)
    {
        spdkPollers[i] = nullptr;
    }
    SpdkConnection::SpdkNvmfInitializeReactorSubsystemMapping();
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

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bool
QosSpdkManager::IsFeQosEnabled(void)
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
    QosSpdkManager* spdkManager = static_cast<QosSpdkManager*>(arg1);
    if (false == spdkManager->IsFeQosEnabled())
    {
        return;
    }
    uint32_t reactor = spdkManager->GetReactorId();
    spdk_poller* poller = spdkManager->GetSpdkPoller(reactor);
    PollerManager::SpdkPollerUnregister(&poller);
    if ((EventFrameworkApiSingleton::Instance()->IsLastReactorNow()))
    {
        unregistrationComplete = true;
    }
    else
    {
        uint32_t nextReactor = EventFrameworkApiSingleton::Instance()->GetNextReactor();
        spdkManager->SetReactorId(nextReactor);
        EventFrameworkApiSingleton::Instance()->SendSpdkEvent(nextReactor, PollerUnregister, spdkManager, nullptr);
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
QosSpdkManager::Finalize(void)
{
    if (false == feQosEnabled)
    {
        return;
    }
    reactorId = EventFrameworkApiSingleton::Instance()->GetFirstReactor();
    bool succeeded = EventFrameworkApiSingleton::Instance()->SendSpdkEvent(reactorId, QosSpdkManager::PollerUnregister, this, nullptr);
    if (unlikely(false == succeeded))
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::QOS_POLLER_UNREGISTRATION_FAILED;
        POS_TRACE_ERROR(static_cast<int>(eventId), PosEventId::GetString(eventId), reactorId);
    }
    while (QosSpdkManager::unregistrationComplete == false)
    {
        usleep(1);
    }
    POS_TRACE_INFO(POS_EVENT_ID::QOS_POLLER_UNREGISTRATION, "All the Qos pollers UnRegistered");
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
    if (false == feQosEnabled)
    {
        return;
    }
    registerQosPollerDone = false;
    reactorId = EventFrameworkApiSingleton::Instance()->GetFirstReactor();
    bool succeeded = EventFrameworkApiSingleton::Instance()->SendSpdkEvent(reactorId, RegisterQosPoller, this, nullptr);
    if (unlikely(false == succeeded))
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::QOS_POLLER_REGISTRATION_FAILED;
        POS_TRACE_ERROR(static_cast<int>(eventId), PosEventId::GetString(eventId), reactorId);
    }
    else
    {
        while (false == registerQosPollerDone)
        {
            usleep(1);
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
    _UpdateReactorInContext(reactor);
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
QosSpdkManager::_UpdateReactorInContext(uint32_t reactor)
{
    qosContext->UpdateReactorCoreList(reactor);
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
    QosSpdkManager* spdkManager = static_cast<QosSpdkManager*>(arg1);
    if (false == spdkManager->IsFeQosEnabled())
    {
        return;
    }
    uint32_t reactor = spdkManager->GetReactorId();
    poller_structure& pollerData = spdkManager->GetReactorData(reactor);
    pollerData.qosTimeSlice = IBOF_QOS_TIMESLICE_IN_USEC * SpdkConnection::SpdkGetTicksHz() / SPDK_SEC_TO_USEC;
    pollerData.nextTimeStamp = SpdkConnection::SpdkGetTicks() + pollerData.qosTimeSlice;
    pollerData.id = reactor;
    std::string pollerName = "volume_qos_poller_" + std::to_string(reactor);
    spdk_poller* poller = static_cast<spdk_poller*>(PollerManager::SpdkPollerRegister(spdkManager->SpdkVolumeQosPoller, &pollerData, 0, pollerName));
    if (unlikely(NULL == poller))
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::QOS_POLLER_REGISTRATION_FAILED;
        POS_TRACE_ERROR(static_cast<int>(eventId), PosEventId::GetString(eventId), reactor);
        registerQosPollerDone = true;
        return;
    }
    else
    {
        POS_TRACE_INFO(POS_EVENT_ID::QOS_POLLER_REGISTRATION, "Poller {} Registration Successful, ", pollerName);
    }
    spdkManager->UpdateSpdkPoller(reactor, poller);
    if (EventFrameworkApiSingleton::Instance()->IsLastReactorNow())
    {
        registerQosPollerDone = true;
    }
    else
    {
        uint32_t nextReactor = EventFrameworkApiSingleton::Instance()->GetNextReactor();
        spdkManager->SetReactorId(nextReactor);
        bool success = EventFrameworkApiSingleton::Instance()->SendSpdkEvent(nextReactor, RegisterQosPoller, spdkManager, nullptr);
        if (unlikely(false == success))
        {
            POS_EVENT_ID eventId = POS_EVENT_ID::QOS_POLLER_REGISTRATION_FAILED;
            POS_TRACE_ERROR(static_cast<int>(eventId), PosEventId::GetString(eventId), nextReactor);
            registerQosPollerDone = true;
        }
    }
}

} // namespace pos

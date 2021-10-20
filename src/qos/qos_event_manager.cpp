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

#include "src/qos/qos_event_manager.h"

#include "src/qos/io_queue.h"
#include "src/qos/parameter_queue.h"
#include "src/qos/qos_manager.h"
#include "src/qos/rate_limit.h"
#include "src/spdk_wrapper/event_framework_api.h"

namespace pos
{
#define SEC_TO_USEC 1000000ULL
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosEventManager::QosEventManager(SpdkEnvCaller* spdkEnvCaller)
: spdkEnvCaller(spdkEnvCaller)
{
    for (uint32_t eventId = BackendEvent_Start; eventId < BackendEvent_Count; eventId++)
    {
        eventWeightWRR[eventId] = M_DEFAULT_WEIGHT;
    }

    for (uint32_t worker = 0; worker < MAX_IO_WORKER; worker++)
    {
        _EventParamterInit(worker);
        for (uint32_t eventId = BackendEvent_Start; eventId < BackendEvent_Count; eventId++)
        {
            pendingEventIO[worker][eventId] = 0;
        }
    }
    try
    {
        bwIopsRateLimit = new BwIopsRateLimit;
        parameterQueue = new ParameterQueue;
        ioQueue = new IoQueue<UbioSmartPtr>;
    }
    catch (std::bad_alloc& ex)
    {
        assert(0);
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
    delete bwIopsRateLimit;
    delete parameterQueue;
    delete ioQueue;
    if (spdkEnvCaller != nullptr)
    {
        delete spdkEnvCaller;
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
QosEventManager::_RateLimit(uint32_t id, BackendEvent event)
{
    return bwIopsRateLimit->IsLimitExceeded(id, event);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosEventManager::_UpdateRateLimit(uint32_t id, BackendEvent event, uint64_t size)
{
    bwIopsRateLimit->UpdateRateLimit(id, event, size);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosEventManager::HandleEventUbioSubmission(SubmissionAdapter* ioSubmission,
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
    if ((0 == pendingEventIO[id][event]) && (false == _RateLimit(id, event)))
    {
        currentBw = currentBw + ubio->GetSize();
        completionCount = ioSubmission->Do(ubio);
        submissionNotifier->Do(completionCount);
        _UpdateRateLimit(id, event, ubio->GetSize());
    }
    else
    {
        pendingEventIO[id][event]++;
        _EnqueueEventUbio(id, event, ubio);
        while (!IsExitQosSet())
        {
            if (_RateLimit(id, event) == true)
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
            _UpdateRateLimit(id, event, queuedUbio->GetSize());
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
UbioSmartPtr
QosEventManager::_DequeueEventUbio(uint32_t id, uint32_t event)
{
    return ioQueue->DequeueIo(id, event);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosEventManager::_EnqueueParams(uint32_t workerId, BackendEvent event, bw_iops_parameter& event_param)
{
    parameterQueue->EnqueueParameter(workerId, event, event_param);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bw_iops_parameter
QosEventManager::DequeueParams(uint32_t workerId, BackendEvent event)
{
    return parameterQueue->DequeueParameter(workerId, event);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 * Throttling of event is not supported and excluding in coverage
 */
/* --------------------------------------------------------------------------*/
// LCOV_EXCL_START
void
QosEventManager::_EnqueueEventUbio(uint32_t id, BackendEvent event, UbioSmartPtr ubio)
{
    ioQueue->EnqueueIo(id, event, ubio);
}
// LCOV_EXCL_STOP
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
int64_t
QosEventManager::GetDefaultEventWeightWRR(BackendEvent eventId)
{
    return M_DEFAULT_WEIGHT;
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosEventManager::_ResetRateLimit(uint32_t id, BackendEvent event)
{
    bwIopsRateLimit->ResetRateLimit(id, event, 1, DEFAULT_MAX_BW_IOPS, DEFAULT_MAX_BW_IOPS);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int
QosEventManager::IOWorkerPoller(uint32_t id, SubmissionAdapter* ioSubmission)
{
    int32_t retVal = 0;
    uint64_t now = spdkEnvCaller->SpdkGetTicks();
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
        eventQosParam[workerId][event].valid = M_VALID_ENTRY;
        _EnqueueParams(workerId, (BackendEvent)event, eventQosParam[workerId][event]);
        currentBW = 0;
        _ResetRateLimit(workerId, (BackendEvent)event);
        while (!IsExitQosSet())
        {
            if (_RateLimit(workerId, (BackendEvent)event) == true)
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
            _UpdateRateLimit(workerId, (BackendEvent)event, queuedUbio->GetSize());
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
    eventPollStructure[id].qosTimeSlice = IBOF_QOS_TIMESLICE_IN_USEC * spdkEnvCaller->SpdkGetTicksHz() / SEC_TO_USEC;
    eventPollStructure[id].nextTimeStamp = spdkEnvCaller->SpdkGetTicks() + eventPollStructure[id].qosTimeSlice;
    eventPollStructure[id].id = id;
}
} // namespace pos

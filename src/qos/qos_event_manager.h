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

#pragma once

#include <atomic>
#include <queue>

#include "src/spdk_wrapper/caller/spdk_env_caller.h"
#include "src/qos/exit_handler.h"
#include "src/qos/qos_common.h"

namespace pos
{
class SubmissionAdapter;
class SubmissionNotifier;
class BwIopsRateLimit;
class ParameterQueue;
template<class T>
class IoQueue;
class QosEventManager : public ExitQosHandler
{
public:
    explicit QosEventManager(SpdkEnvCaller* spdkEnvCaller = new SpdkEnvCaller());
    ~QosEventManager(void);
    int IOWorkerPoller(uint32_t id, SubmissionAdapter* ioSubmission);
    void HandleEventUbioSubmission(SubmissionAdapter* ioSubmission,
        SubmissionNotifier* submissionNotifier, uint32_t id, UbioSmartPtr ubio);
    bw_iops_parameter DequeueParams(uint32_t workerId, BackendEvent event);
    int64_t GetEventWeightWRR(BackendEvent eventId);
    int64_t GetDefaultEventWeightWRR(BackendEvent eventId);
    void SetEventWeightWRR(BackendEvent eventId, int64_t weight);

private:
    void _EventParamterInit(uint32_t id);
    void _EnqueueParams(uint32_t workerId, BackendEvent event, bw_iops_parameter& event_param);
    void _EnqueueEventUbio(uint32_t id, BackendEvent event, UbioSmartPtr ubio);
    UbioSmartPtr _DequeueEventUbio(uint32_t id, uint32_t event);
    void _ResetRateLimit(uint32_t id, BackendEvent event);
    bool _RateLimit(uint32_t id, BackendEvent event);
    void _UpdateRateLimit(uint32_t id, BackendEvent event, uint64_t size);
    BackendEvent _IdentifyEventType(UbioSmartPtr ubio);

    bw_iops_parameter eventQosParam[MAX_IO_WORKER][BackendEvent_Count];
    poller_structure eventPollStructure[MAX_IO_WORKER];
    std::atomic<int64_t> eventWeightWRR[BackendEvent_Count];
    uint64_t pendingEventIO[MAX_IO_WORKER][BackendEvent_Count];
    BwIopsRateLimit* bwIopsRateLimit;
    ParameterQueue* parameterQueue;
    IoQueue<UbioSmartPtr>* ioQueue;
    SpdkEnvCaller* spdkEnvCaller;
};
} // namespace pos

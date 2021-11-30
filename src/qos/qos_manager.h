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

#pragma once

#include <atomic>
#include <iostream>
#include <map>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/bio/volume_io.h"
#include "src/event_scheduler/event.h"
#include "src/include/backend_event.h"
#include "src/include/event_priority.h"
#include "src/io/frontend_io/aio.h"
#include "src/lib/singleton.h"
#include "src/qos/exit_handler.h"
#include "src/qos/qos_common.h"
#include "submission_adapter.h"
#include "submission_notifier.h"
namespace pos
{
class Ubio;
class VolumeIo;
class IOWorker;
class AIO;
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Interface Class for event scheduling policy
 *
 */
/* --------------------------------------------------------------------------*/
class QosVolumeManager;
class QosEventManager;
class QosSpdkManager;
class QosContext;
class QosInternalManager;

class QosManager : public ExitQosHandler
{
public:
    QosManager(void);
    ~QosManager(void);
    void Initialize(void);
    int IOWorkerPoller(uint32_t id, SubmissionAdapter* ioSubmission);
    void HandleEventUbioSubmission(SubmissionAdapter* ioSubmission,
        SubmissionNotifier* submissionNotifier, uint32_t id, UbioSmartPtr ubio);
    int UpdateVolumePolicy(uint32_t volId, qos_vol_policy policy);
    qos_vol_policy GetVolumePolicy(uint32_t volId);
    bw_iops_parameter DequeueVolumeParams(uint32_t reactor, uint32_t volId);
    bw_iops_parameter DequeueEventParams(uint32_t workerId, BackendEvent event);
    void SetEventWeightWRR(BackendEvent event, int64_t weight);
    int64_t GetEventWeightWRR(BackendEvent event);
    uint32_t GetUsedStripeCnt(void);
    void IncreaseUsedStripeCnt(void);
    void DecreaseUsedStripeCnt(void);
    void UpdateSubsystemToVolumeMap(uint32_t nqnId, uint32_t volId);
    void IncreasePendingEvents(BackendEvent event);
    void DecreasePendingEvents(BackendEvent event);
    void LogEvent(BackendEvent event);
    uint32_t GetEventLog(BackendEvent event);
    uint32_t GetPendingEvents(BackendEvent event);
    void CopyEventPolicy(void);
    void HandlePosIoSubmission(IbofIoSubmissionAdapter* aioSubmission, pos_io* io);
    int VolumeQosPoller(poller_structure* param, IbofIoSubmissionAdapter* aioSubmission);
    bool IsFeQosEnabled(void);
    qos_rebuild_policy GetRebuildPolicy(void);
    int UpdateRebuildPolicy(qos_rebuild_policy rebuildPolicy);
    void SetVolumeLimit(uint32_t reactor, uint32_t volId, int64_t weight, bool iops);
    int64_t GetVolumeLimit(uint32_t reactor, uint32_t volId, bool iops);
    bool IsVolumePolicyUpdated(void);
    void SetGcFreeSegment(uint32_t count);
    uint32_t GetGcFreeSegment(void);
    void GetVolumePolicyMap(std::map<uint32_t, qos_vol_policy>& volumePolicyMapCopy);
    std::vector<int> GetVolumeFromActiveSubsystem(uint32_t nqnId);
    bool IsMinimumPolicyInEffectInSystem(void);
    void GetSubsystemVolumeMap(std::unordered_map<int32_t, std::vector<int>>& subSysVolMap);

private:
    void _Finalize(void);
    void _QosWorker(void);
    void _QosTimeChecker(void);
    QosInternalManager* _GetNextInternalManager(QosInternalManagerType internalManagerType);
    std::thread* qosThread;
    std::thread* qosTimeThrottling;
    cpu_set_t cpuSet;
    volatile uint64_t eventWeight[BackendEvent_Count];
    uint32_t oldLog[BackendEvent_Count];
    std::mutex policyUpdateLock;
    bool feQosEnabled;
    bool initialized;
    uint32_t pollerTime;
    std::atomic<uint32_t> pendingEvents[BackendEvent_Count];
    std::atomic<uint32_t> eventLog[BackendEvent_Count];
    std::atomic<uint32_t> usedStripeCnt;
    std::atomic<uint32_t> minGuaranteeVolume;
    std::atomic<bool> volMinPolicyInEffect;
    std::atomic<bool> minBwGuarantee;
    std::atomic<bool> volumePolicyUpdated;
    uint32_t gcFreeSegments;
    qos_vol_policy volPolicyCli[MAX_VOLUME_COUNT];
    qos_rebuild_policy rebuildPolicyCli;
    std::map<uint32_t, qos_vol_policy> volumePolicyMapCli;
    QosVolumeManager* qosVolumeManager;
    QosEventManager* qosEventManager;
    QosSpdkManager* spdkManager;
    QosContext* qosContext;
    QosInternalManager* monitoringManager;
    QosInternalManager* policyManager;
    QosInternalManager* processingManager;
    QosInternalManager* correctionManager;
};

using QosManagerSingleton = Singleton<QosManager>;

} // namespace pos

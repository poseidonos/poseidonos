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
#include <iostream>
#include <map>
#include <mutex>
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
#include "src/spdk_wrapper/caller/spdk_env_caller.h"
#include "src/spdk_wrapper/caller/spdk_pos_nvmf_caller.h"
#include "src/qos/exit_handler.h"
#include "src/qos/qos_array_manager.h"
#include "src/qos/qos_common.h"
#include "src/bio/volume_io.h"
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
class AffinityManager;
class ConfigManager;
class EventFrameworkApi;
class QosManager : public ExitQosHandler
{
public:
    explicit QosManager(SpdkEnvCaller* spdkEnvCaller = new SpdkEnvCaller(),
        SpdkPosNvmfCaller* spdkPosNvmfCaller = new SpdkPosNvmfCaller(),
        ConfigManager* configManager = ConfigManagerSingleton::Instance(),
        EventFrameworkApi* eventFrameworkApi = EventFrameworkApiSingleton::Instance(),
        AffinityManager* affinityManager = AffinityManagerSingleton::Instance());
    virtual ~QosManager(void);
    void Initialize(void);
    void InitializeSpdkManager(void);
    virtual int IOWorkerPoller(uint32_t id, SubmissionAdapter* ioSubmission);
    virtual void HandleEventUbioSubmission(SubmissionAdapter* ioSubmission,
        SubmissionNotifier* submissionNotifier, uint32_t id, UbioSmartPtr ubio);
    int UpdateVolumePolicy(uint32_t volId, qos_vol_policy policy, uint32_t arrayId);
    qos_vol_policy GetVolumePolicy(uint32_t volId, std::string arrayName);
    virtual bw_iops_parameter DequeueVolumeParams(uint32_t reactor, uint32_t volId, uint32_t arrayId);
    virtual bw_iops_parameter DequeueEventParams(uint32_t workerId, BackendEvent event);
    void SetEventWeightWRR(BackendEvent event, int64_t weight);
    virtual int64_t GetEventWeightWRR(BackendEvent event);
    virtual int64_t GetDefaultEventWeightWRR(BackendEvent event);
    uint32_t GetUsedStripeCnt(uint32_t arrayId);
    void IncreaseUsedStripeCnt(uint32_t arrayId);
    void DecreaseUsedStripeCnt(std::string arrayName);
    void UpdateSubsystemToVolumeMap(uint32_t nqnId, uint32_t volId, std::string arrayName);
    void DeleteVolumeFromSubsystemMap(uint32_t nqnId, uint32_t volId, std::string arrayName);
    virtual void IncreasePendingBackendEvents(BackendEvent event);
    virtual void DecreasePendingBackendEvents(BackendEvent event);
    virtual void LogEvent(BackendEvent event);
    uint32_t GetEventLog(BackendEvent event);
    uint32_t GetPendingBackendEvents(BackendEvent event);
    void CopyEventPolicy(void);
    void HandlePosIoSubmission(IbofIoSubmissionAdapter* aioSubmission, VolumeIoSmartPtr io);
    int VolumeQosPoller(poller_structure* param, IbofIoSubmissionAdapter* aioSubmission);
    virtual bool IsFeQosEnabled(void);
    qos_rebuild_policy GetRebuildPolicy(std::string arrayName);
    int UpdateRebuildPolicy(qos_rebuild_policy rebuildPolicy);
    void SetVolumeLimit(uint32_t reactor, uint32_t volId, int64_t weight, bool iops, uint32_t arrayId);
    int64_t GetVolumeLimit(uint32_t reactor, uint32_t volId, bool iops, uint32_t arrayId);
    bool IsVolumePolicyUpdated(uint32_t arrayId);
    void SetGcFreeSegment(uint32_t count, uint32_t arrayId);
    uint32_t GetGcFreeSegment(uint32_t arrayId);
    void GetVolumePolicyMap(uint32_t arrayId, std::map<uint32_t, qos_vol_policy>& volumePolicyMapCopy);
    std::vector<int> GetVolumeFromActiveSubsystem(uint32_t nqnId, uint32_t arrayId);
    int32_t GetArrayIdFromMap(std::string arrayName);
    std::string GetArrayNameFromMap(uint32_t arrayId);
    uint32_t GetNumberOfArrays(void);
    void UpdateArrayMap(string arrayName);
    void DeleteEntryArrayMap(std::string arrayName);
    void GetSubsystemVolumeMap(std::unordered_map<int32_t, std::vector<int>>& subsysVolMap, uint32_t arrayId);
    uint32_t GetNoContentionCycles(void);
    virtual bool IsMinimumPolicyInEffectInSystem(void);
    void ResetCorrection(void);
    void FinalizeSpdkManager(void);

private:
    virtual void _Finalize(void);
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
    std::atomic<uint32_t> pendingBackendEvents[BackendEvent_Count];
    std::atomic<uint32_t> eventLog[BackendEvent_Count];
    QosEventManager* qosEventManager;
    QosArrayManager* qosArrayManager[MAX_ARRAY_COUNT];
    std::map<std::string, uint32_t> arrayNameMap;
    std::map<uint32_t, std::string> arrayIdMap;
    QosSpdkManager* spdkManager;
    QosContext* qosContext;
    QosInternalManager* monitoringManager;
    QosInternalManager* policyManager;
    QosInternalManager* processingManager;
    QosInternalManager* correctionManager;
    uint32_t currentNumberOfArrays;
    std::vector<uint32_t> prevIndexDeleted;
    std::mutex mapUpdateLock;
    bool systemMinPolicy;
    SpdkEnvCaller* spdkEnvCaller;
    SpdkPosNvmfCaller* spdkPosNvmfCaller;
    ConfigManager* configManager;
    EventFrameworkApi* eventFrameworkApi;
    AffinityManager* affinityManager;

    uint64_t previousDelay[M_MAX_REACTORS];
};

using QosManagerSingleton = Singleton<QosManager>;

} // namespace pos

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

#ifndef __IBOFOS_QOS_MANAGER_H__
#define __IBOFOS_QOS_MANAGER_H__

#include <atomic>
#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>
#include <queue>

#include "spdk/thread.h"
#include "src/bio/volume_io.h"
#include "src/event_scheduler/event.h"
#include "src/include/backend_event.h"
#include "src/include/event_priority.h"
#include "src/io/frontend_io/aio.h"
#include "src/lib/singleton.h"
#include "src/qos/qos_common.h"
#include "src/qos/qos_policy_manager.h"
#include "src/sys_event/volume_event.h"
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
class QosPolicyManager;
class QosVolumeManager;
class QosEventManager;
class QosSpdkManager;
class QosManager
{
public:
    QosManager(void);
    ~QosManager(void);
    void Initialize(void);
    int EventQosPoller(uint32_t id, SubmissionAdapter* ioSubmission);
    void SubmitAsyncIO(SubmissionAdapter* ioSubmission,
        SubmissionNotifier* submissionNotifier, uint32_t id, UbioSmartPtr ubio);
    int UpdateVolumePolicy(uint32_t volId, qos_vol_policy policy);
    qos_vol_policy GetVolumePolicy(uint32_t volId);
    volume_qos_params DequeueVolumeParams(uint32_t reactor, uint32_t volId);
    event_qos_params DequeueEventParams(uint32_t workerId, BackendEvent event);
    void SetVolumeWeight(uint32_t reactor, uint32_t volId, uint32_t weight);
    void SetEventWeight(uint32_t event, uint32_t weight);
    void SetEventWeightWRR(BackendEvent event, int64_t weight);
    int64_t GetEventWeightWRR(BackendEvent event);
    uint32_t GetVolumeWeight(uint32_t reactor, uint32_t volId);
    uint32_t GetEventWeight(uint32_t event);
    uint32_t GetUsedStripeCnt(void);
    void IncreaseUsedStripeCnt(void);
    void DecreaseUsedStripeCnt(void);
    void UpdateSubsystemToVolumeMap(uint32_t nqnId, uint32_t volId);
    uint32_t GetRemainingUserRebuildSegmentCnt(void);
    uint32_t GetRemainingMetaRebuildStripeCnt(void);
    uint32_t GetFreeSegmentCnt(void);
    void IncreasePendingEvents(BackendEvent event);
    void DecreasePendingEvents(BackendEvent event);
    void LogEvent(BackendEvent event);
    uint32_t GetEventLog(BackendEvent event);
    uint32_t GetPendingEvents(BackendEvent event);
    uint64_t GetVolBWLog(int volId);
    void LogVolBw(uint32_t volId, uint64_t size);
    int SetEventPolicy(BackendEvent event, EventPriority priority, uint32_t weight);
    int SetEventPolicy(string eventName, string perfImpact);
    void ResetEventPolicy(void);
    void CopyEventPolicy(void);
    void AioSubmitAsyncIO(IbofIoSubmissionAdapter* aioSubmission, pos_io* io);
    BackendEvent GetEventId(string eventName);
    void SetMaxVolWeightCli(uint32_t volId, uint64_t weight);
    uint64_t GetMaxVolumeWeight(uint32_t volId);
    std::map<uint32_t, uint32_t>& GetVolumeReactorMap(uint32_t volId);
    uint32_t GetVolumeTotalConnection(uint32_t volId);
    int VolumeQosPoller(poller_structure* param, IbofIoSubmissionAdapter* aioSubmission);
    bool IsFeQosEnabled(void);

private:
    void _Finalize(void);
    void _QosWorkerPoller(void);
    void _UpdatePolicyManagerActiveVolumeData(uint32_t reactorId, uint32_t subsystemId, uint32_t volId);
    void _UpdateMaxVolumeWeightReactorVolume(void);
    void _HandleVolumeMinimumPolicy(uint64_t& endTimeStamp);
    std::vector<int> _GetVolumeFromActiveSubsystem(uint32_t nqnId);
    std::atomic<uint64_t> volBwLog[MAX_VOLUME_COUNT];
    uint64_t oldvolBwLog[MAX_VOLUME_COUNT];
    volatile bool quitQos;
    std::thread* qosThread;
    cpu_set_t cpuSet;
    QosPolicyManager* policyManager;
    volatile uint64_t eventWeight[BackendEvent_Count];
    std::atomic<uint32_t> freeSegmentCnt;
    std::atomic<uint32_t> rebuildPendingDataSegments;
    std::atomic<uint32_t> rebuildPendingMetaStripes;
    std::atomic<uint32_t> pendingEvents[BackendEvent_Count];
    std::atomic<uint32_t> eventLog[BackendEvent_Count];
    std::atomic<uint32_t> usedStripeCnt;
    uint32_t oldLog[BackendEvent_Count];
    std::unordered_map<BackendEvent, EventPriority> eventPriorityMap;
    std::unordered_map<EventPriority, uint32_t> priorityWeightMap;
    std::map<string, BackendEvent> eventDict = {
        {"flush", BackendEvent_Flush},
        {"gc", BackendEvent_GC},
        {"rebuild", BackendEvent_UserdataRebuild},
        {"meta_rebuild", BackendEvent_MetadataRebuild},
        {"metaio", BackendEvent_MetaIO},
        {"fe_rebuild", BackendEvent_FrontendIO}};
    std::mutex policyUpdateLock;
    std::atomic<uint16_t> wtUpdate[MAX_VOLUME_COUNT];
    std::atomic<uint64_t> maxVolWeight[MAX_VOLUME_COUNT];
    std::map<uint32_t, map<uint32_t, uint32_t>> volReactorMap;
    std::map<uint32_t, map<uint32_t, uint32_t>> reactorVolMap;
    uint32_t totalConnection[MAX_VOLUME_COUNT];
    std::atomic<uint64_t> maxVolWeightCli[MAX_VOLUME_COUNT];
    QosVolumeManager* qosVolumeManager;
    QosEventManager* qosEventManager;
    QosSpdkManager* spdkManager;
    bool feQosEnabled;
    bool initialized;
};

using QosManagerSingleton = Singleton<QosManager>;

class QosVolumeManager : public VolumeEvent
{
public:
    explicit QosVolumeManager(QosPolicyManager* policyMgr, bool feQos);
    ~QosVolumeManager(void);
    bool VolumeCreated(std::string volName, int volID, uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw, std::string arrayName) override;
    bool VolumeDeleted(std::string volName, int volID, uint64_t volSizeByte, std::string arrayName) override;
    bool VolumeMounted(std::string volName, std::string subnqn, int volID, uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw, std::string arrayName) override;
    bool VolumeUnmounted(std::string volName, int volID, std::string arrayName) override;
    bool VolumeLoaded(std::string name, int id, uint64_t totalSize, uint64_t maxiops, uint64_t maxbw, std::string arrayName) override;
    bool VolumeUpdated(std::string volName, int volID, uint64_t maxiops, uint64_t maxbw, std::string arrayName) override;
    void VolumeDetached(vector<int> volList, std::string arrayName) override;
    void UpdateSubsystemToVolumeMap(uint32_t nqnId, uint32_t volId);
    std::vector<int> GetVolumeFromActiveSubsystem(uint32_t nqnId);
    void AioSubmitAsyncIO(IbofIoSubmissionAdapter* aioSubmission, pos_io* io);
    void SetVolumeWeight(uint32_t reactor, uint32_t volId, uint32_t weight);
    uint32_t GetVolumeWeight(uint32_t reactor, uint32_t volId);
    volume_qos_params DequeueVolumeParams(uint32_t reactor, uint32_t volId);
    int VolumeQosPoller(struct poller_structure* param, IbofIoSubmissionAdapter* aioSubmission);

private:
    void _EnqueueVolumeParams(uint32_t reactor, uint32_t volId, volume_qos_params& volume_param);
    void _ResetRateLimit(uint32_t reactor, int volId);
    bool _RateLimit(uint32_t reactor, int volId);
    void _UpdateRateLimit(uint32_t reactor, int volId, uint64_t size);
    void _EnqueueVolumeUbio(uint32_t rectorId, uint32_t volId, pos_io* io);
    void _UpdateVolumeMaxQos(int volId, uint64_t maxiops, uint64_t maxbw);
    pos_io* _DequeueVolumeUbio(uint32_t reactorId, uint32_t volId);
    QosPolicyManager* qosPolicyManager;
    std::unordered_map<int32_t, std::vector<int>> nqnVolumeMap;
    volume_qos_params volumeQosParam[M_MAX_REACTORS][MAX_VOLUME_COUNT];
    int64_t remainingLimit[M_MAX_REACTORS][MAX_VOLUME_COUNT];
    std::atomic<uint64_t> volReactorWeight[M_MAX_REACTORS][MAX_VOLUME_COUNT];
    std::queue<volume_qos_params> volumesParamsQueue[M_MAX_REACTORS][MAX_VOLUME_COUNT];
    std::queue<pos_io*> volumesUbioQueue[M_MAX_REACTORS][MAX_VOLUME_COUNT];
    std::mutex volQueueLock[M_MAX_REACTORS][MAX_VOLUME_COUNT];
    uint64_t pendingVolumeIO[M_MAX_REACTORS][MAX_VOLUME_COUNT];
    bool feQosEnabled;
};

class QosEventManager
{
public:
    explicit QosEventManager(QosPolicyManager* policyMgr);
    ~QosEventManager(void);
    int EventQosPoller(uint32_t id, SubmissionAdapter* ioSubmission);
    void SubmitAsyncIO(SubmissionAdapter* ioSubmission,
        SubmissionNotifier* submissionNotifier, uint32_t id, UbioSmartPtr ubio);
    event_qos_params DequeueEventParams(uint32_t workerId, BackendEvent event);
    uint32_t GetEventWeight(uint32_t event);
    void SetEventWeight(uint32_t event, uint32_t weight);
    int64_t GetEventWeightWRR(BackendEvent eventId);
    void SetEventWeightWRR(BackendEvent eventId, int64_t weight);

private:
    void _EventParamterInit(uint32_t id);
    void _EnqueueEventParams(uint32_t workerId, BackendEvent event, event_qos_params& event_param);
    void _EnqueueEventUbio(uint32_t id, BackendEvent event, UbioSmartPtr ubio);
    UbioSmartPtr _DequeueEventUbio(uint32_t id, uint32_t event);
    void _ResetEventRateLimit(uint32_t id, BackendEvent event);
    bool _EventRateLimit(uint32_t id, BackendEvent event);
    void _UpdateEventRateLimit(uint32_t id, BackendEvent event, uint64_t size);
    BackendEvent _IdentifyEventType(UbioSmartPtr ubio);

    QosPolicyManager* qosPolicyManager;
    int64_t remainingEventLimit[MAX_IO_WORKER][BackendEvent_Count];
    event_qos_params eventQosParam[MAX_IO_WORKER][BackendEvent_Count];
    poller_structure eventPollStructure[MAX_IO_WORKER];
    std::queue<event_qos_params> eventsParamsQueue[MAX_IO_WORKER][BackendEvent_Count];
    std::queue<UbioSmartPtr> eventsUbioQueue[MAX_IO_WORKER][BackendEvent_Count];
    std::mutex eventQueueLock[MAX_IO_WORKER][BackendEvent_Count];
    uint64_t eventWeight[BackendEvent_Count];
    std::atomic<int64_t> eventWeightWRR[BackendEvent_Count];
    uint64_t pendingEventIO[MAX_IO_WORKER][BackendEvent_Count];
};

class QosSpdkManager
{
public:
    explicit QosSpdkManager(bool feQos);
    ~QosSpdkManager(void);
    void Initialize(void);
    void Finalize(void);
    poller_structure& GetReactorData(uint32_t reactor);
    spdk_poller* GetSpdkPoller(uint32_t reactor);
    void UpdateReactorData(uint32_t reactor, poller_structure data);
    void UpdateSpdkPoller(uint32_t reactor, spdk_poller* spdkPoller);
    static void RegisterQosPoller(void* arg1, void* arg2);
    static void PollerUnregister(void* arg1, void* arg2);
    static int SpdkVolumeQosPoller(void* arg1);
    uint32_t GetReactorId(void);
    void SetReactorId(uint32_t id);
    static std::atomic<bool> registerQosPollerDone;
    static std::atomic<bool> unregistrationComplete;
    bool IsFeQosEnabled(void);

private:
    void _SetupQosReactorPoller(void);
    poller_structure reactorData[M_MAX_REACTORS];
    spdk_poller* spdkPollers[M_MAX_REACTORS];
    uint32_t reactorId;
    bool feQosEnabled;
};

} // namespace pos
#endif // __IBOFOS_QOS_MANAGER_H__

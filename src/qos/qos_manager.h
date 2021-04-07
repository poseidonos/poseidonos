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

#include "spdk/thread.h"
#include "src/io/frontend_io/aio.h"
#include "src/io/general_io/volume_io.h"
#include "src/lib/singleton.h"
#include "src/qos/qos_common.h"
#include "src/qos/qos_data_manager.h"
#include "src/qos/qos_policy_manager.h"
#include "src/scheduler/event.h"
#include "src/sys_event/volume_event.h"
namespace ibofos
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
class QosManager : public VolumeEvent
{
public:
    QosManager();
    ~QosManager();

    void Initialize(void);
    void Finalize(void);
    int EventQosPoller(uint32_t id);
    void SubmitAsyncIO(IOWorker* ioWorker, UbioSmartPtr ubio);
    void SendVolumeIoDirect(VolumeIoSmartPtr volumeIo);
    BackendEvent IdentifyEventType(UbioSmartPtr ubio);
    QosPolicyManager* policyManager;
    int UpdateVolumePolicy(uint32_t volId, qos_vol_policy policy);
    qos_vol_policy GetVolumePolicy(uint32_t volId);
    void EnqueueVolumeParams(uint32_t reactor, uint32_t volId, volume_qos_params& volume_param);
    volume_qos_params DequeueVolumeParams(uint32_t reactor, uint32_t volId);
    void EnqueueVolumeUbio(uint32_t rectorId, uint32_t volId, VolumeIoSmartPtr ubio);
    VolumeIoSmartPtr DequeueVolumeUbio(uint32_t reactorId, uint32_t volId);
    VolumeIoSmartPtr PeekVolumeUbio(uint32_t reactorId, uint32_t volId);
    UbioSmartPtr DequeueEventUbio(uint32_t id, uint32_t event);
    UbioSmartPtr PeekEventUbio(uint32_t id, uint32_t event);
    // Event specific qos
    void EnqueueEventParams(uint32_t workerId, BackendEvent event, event_qos_params& event_param);
    event_qos_params DequeueEventParams(uint32_t workerId, BackendEvent event);
    void EnqueueEventUbio(uint32_t id, BackendEvent event, UbioSmartPtr ubio);
    // Inherited functions from VolumeEvent class
    bool VolumeCreated(std::string volName, int volID, uint64_t volSizeByte,
        uint64_t maxiops, uint64_t maxbw) override;
    bool VolumeDeleted(std::string volName,
        int volID, uint64_t volSizeByte) override;
    bool VolumeMounted(std::string volName, std::string subnqn, int volID, uint64_t volSizeByte,
        uint64_t maxiops, uint64_t maxbw) override;
    bool VolumeUnmounted(std::string volName, int volID) override;
    bool VolumeLoaded(std::string name, int id, uint64_t totalSize,
        uint64_t maxiops, uint64_t maxbw) override;
    bool VolumeUpdated(std::string volName, int volID,
        uint64_t maxiops, uint64_t maxbw) override;
    void VolumeDetached(vector<int> volList) override;

    void SetVolumeWeight(uint32_t reactor, uint32_t volId, uint32_t weight);
    void SetEventWeight(uint32_t event, uint32_t weight);
    void SetEventWeightWRR(BackendEvent event, int64_t weight);
    int64_t GetEventWeightWRR(BackendEvent event);
    uint32_t GetVolumeWeight(uint32_t reactor, uint32_t volId);
    uint32_t GetEventWeight(uint32_t event);
    uint32_t CalcEventWeight(uint32_t event);
    // Volume parameter and event level param struture
    volume_qos_params volumeQosParam[M_MAX_REACTORS][MAX_VOLUME_COUNT];
    event_qos_params eventQosParam[MAX_IO_WORKER][BackendEvent_Count];
    poller_structure volPollStructure[MAX_VOLUME_COUNT];
    poller_structure eventPollStructure[MAX_IO_WORKER];
    uint32_t eventDeficietWeight[BackendEvent_Count];
    uint32_t volDeficietWeight[MAX_VOLUME_COUNT];
    void EventParamterInit(uint32_t id);
    uint32_t GetUsedStripeCnt(void);
    void InitialiseUsedStripeCnt(void);
    void IncreaseUsedStripeCnt(void);
    void DecreaseUsedStripeCnt(void);
    void UpdateSubsystemToVolumeMap(uint32_t nqnId, uint32_t volId);
    void PrintNqnVolMapping(void);

    void UpdateRemainingUserRebuildSegmentCnt(uint32_t count);
    void UpdateRemainingMetaRebuildStripeCnt(uint32_t count);
    uint32_t GetRemainingUserRebuildSegmentCnt(void);
    uint32_t GetRemainingMetaRebuildStripeCnt(void);
    uint32_t GetFreeSegmentCnt(void);
    void UpdateFreeSegmentCnt(uint32_t count);
    void SendGCInfo(void);
    void SendRebuildUserDataInfo(void);
    void SendRebuildMetaDataInfo(void);
    void IncreasePendingEvents(BackendEvent event);
    void DecreasePendingEvents(BackendEvent event);
    void LogEvent(BackendEvent event);
    uint32_t GetEventLog(BackendEvent event);
    uint32_t GetPendingEvents(BackendEvent event);
    uint64_t GetVolBWLog(int volId);
    void LogVolBw(uint32_t volId, uint64_t size);

    EventPriority GetBackendEventPriority(BackendEvent event);
    void SetBackendEventPriority(BackendEvent event, EventPriority priority);
    uint32_t GetPriorityWeight(EventPriority priority);
    void SetPriorityWeight(EventPriority priority, uint32_t weight);
    int SetEventPolicy(BackendEvent event, EventPriority priority, uint32_t weight);
    int SetEventPolicy(string eventName, string perfImpact);
    void ResetEventPolicy(void);
    void CopyEventPolicy(void);
    void AioSubmitAsyncIO(VolumeIoSmartPtr volumeIO);
    void ResetRateLimit(uint32_t reactor, int volId);
    bool RateLimit(uint32_t reactor, int volId);
    void UpdateRateLimit(uint32_t reactor, int volId, uint64_t size);
    void InitilizeRateLimit(uint32_t reactor, int volId, uint64_t size);
    void ResetEventRateLimit(uint32_t id, BackendEvent event);
    bool EventRateLimit(uint32_t id, BackendEvent event);
    void UpdateEventRateLimit(uint32_t id, BackendEvent event, uint64_t size);
    void InitilizeEventRateLimit(BackendEvent event, uint64_t size);
    BackendEvent
    GetEventId(string eventName)
    {
        return eventDict[eventName];
    }
    std::atomic<uint64_t> maxVolWeight[MAX_VOLUME_COUNT];
    std::mutex policyUpdateLock;
    std::atomic<uint64_t> maxVolWeightCli[MAX_VOLUME_COUNT];
    std::atomic<uint16_t> wtUpdate[MAX_VOLUME_COUNT];
    std::atomic<uint64_t> volReactorWeight[M_MAX_REACTORS][MAX_VOLUME_COUNT];
    std::map<uint32_t, map<uint32_t, uint32_t>> volReactorMap;
    std::map<uint32_t, map<uint32_t, uint32_t>> reactorVolMap;
    uint32_t totalConnection[MAX_VOLUME_COUNT];

private:
    int64_t remainingLimit[M_MAX_REACTORS][MAX_VOLUME_COUNT];
    int64_t remainingEventLimit[MAX_IO_WORKER][BackendEvent_Count];
    AIO* aio;
    std::atomic<uint64_t> volBwLog[MAX_VOLUME_COUNT];
    uint64_t oldvolBwLog[MAX_VOLUME_COUNT];
    static const uint16_t M_INVALID_SUBSYSTEM;
    static const uint16_t M_VALID_SUBSYSTEM;
    struct spdk_poller* reactor_pollers[M_MAX_REACTORS];
    void _QosManagerPoller();
    void _SetupQoSReactorPoller(void);
    static int _VolumeQosPoller(void* arg1);
    static void _RegisterQosPoller(void* arg1, void* arg2);
    static void _PollerUnregister(void* arg1, void* arg2);
    // Generic Functions
    std::vector<int> GetVolumeFromActiveSubsystem(uint32_t nqnId);
    // Member Variables
    volatile bool quitQos;
    std::thread* qosThread;
    uint32_t pollerTime;
    cpu_set_t cpuSet;
    std::unordered_map<int32_t, std::vector<int>> nqnVolumeMap;
    QosDataManager* qosDataManager;
    volatile uint64_t eventWeight[BackendEvent_Count];
    static std::atomic<bool> registerQosPollerDone;
    static std::atomic<bool> unregistrationComplete;
    // All these counts should be set to 0 during init
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
};

using QosManagerSingleton = Singleton<QosManager>;

} // namespace ibofos
#endif // __IBOFOS_QOS_MANAGER_H__

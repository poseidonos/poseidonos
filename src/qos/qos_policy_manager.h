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

#ifndef __IBOFOS_QOS_POLICY_MANAGER_H__
#define __IBOFOS_QOS_POLICY_MANAGER_H__

#include <iostream>
#include <list>
#include <map>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/array/array.h"
#include "src/bio/ubio.h"
#include "src/event_scheduler/event.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/include/backend_event.h"
#include "src/include/event_priority.h"
#include "src/lib/singleton.h"
#include "src/qos/qos_common.h"
#include "src/sys_event/volume_event.h"

using namespace std;
namespace pos
{
class MovingAvgCompute;
class QosManager;
class QosVolumePolicyManager;
class QosEventPolicyManager;
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Interface Class for event scheduling policy
 *
 */
/* --------------------------------------------------------------------------*/
class QosPolicyManager
{
public:
    QosPolicyManager(void);
    ~QosPolicyManager(void);
    void Initialize(void);
    void CheckSystem(uint64_t timeSpent);
    int UpdateVolumePolicy(int volId, qos_vol_policy volPolicy);
    qos_vol_policy GetVolumePolicy(int volId);
    void CopyVolumePolicy(void);
    void FillVolumeState(int volId);
    void FillEventState(BackendEvent event);
    int SetEventPolicy(BackendEvent event, EventPriority priority, uint32_t weight);
    void ResetEventPolicy(void);
    void CopyEventPolicy(void);
    bool VolumeMinimumPolicyInEffect(void);
    void ResetAll(void);
    void InsertActiveVolumeMap(std::pair<int, int> volPair);
    void UpdateVolumeParam(uint32_t volId, volume_qos_params volParam);
    void InsertActiveConnection(std::pair<std::pair<uint32_t, uint32_t>, uint32_t> reactorVolPair);
    uint32_t GetActiveConnectionsCount(void);
    std::map<int, int>& GetActiveVolumeMap(void);
    void UpdateEventParam(uint32_t eventId, event_qos_params eventParam);
    void ActiveVolumeClear(void);
    void ActiveEventClear(void);
    QosWorkloadType GetMinVolumeWorkloadType(void);
    eventState GetCurrentEventState(BackendEvent event);

private:
    QosVolumePolicyManager* qosVolumePolicyManager;
    QosEventPolicyManager* qosEventPolicyManager;
};

class QosVolumePolicyManager
{
public:
    QosVolumePolicyManager(void);
    ~QosVolumePolicyManager(void);
    void Initialize(QosPolicyManager* qosPolicyMgr);
    void InsertActiveVolumeMap(std::pair<int, int> volPair);
    void UpdateVolumeParam(uint32_t volId, volume_qos_params volParam);
    void InsertActiveConnection(std::pair<std::pair<uint32_t, uint32_t>, uint32_t> reactorVolPair);
    uint32_t GetActiveConnectionsCount(void);
    std::map<int, int>& GetActiveVolumeMap(void);
    bool VolumeMinimumPolicyInEffect(void);
    QosWorkloadType GetMinVolumeWorkloadType(void);
    void ActiveVolumeClear(void);
    void ResetAll(void);
    void CheckSystem(uint64_t timeSpent);
    void FillVolumeState(int volId);
    qos_vol_policy GetVolumePolicy(int volId);
    int UpdateVolumePolicy(int volId, qos_vol_policy volPolicy);
    void CopyVolumePolicy(void);

private:
    bool _IsQosCompromized(int volId, qos_state_ctx* ctx);
    void _ResetVolumeState(int volId);
    void _UpdateOldParamVolumeState(int volId);
    bool _AvgBwCompute(int timeIndex);
    bool _MonitorParams(qos_state_ctx* ctx);
    void _ApplyCorrection(qos_state_ctx* context);
    void _IncreaseThrottling(uint32_t volId, uint64_t correction);
    void _ApplyThrottling(uint64_t throttleValue, bool increase);

    volume_qos_params volParams[MAX_VOLUME_COUNT];
    std::map<int, int> activeVolMap;
    std::map<std::pair<uint32_t, uint32_t>, uint32_t> activeConnections;
    std::list<int> highConsumerVolList;
    std::list<int> lowConsumerVolList;
    std::list<int> intermediateConsumerVolList;
    uint16_t bufIndex;
    uint32_t qosCycle;
    std::atomic<bool> policyDirty[MAX_VOLUME_COUNT];
    std::mutex volumePolicyLock;
    struct qos_vol_policy volPolicy[MAX_VOLUME_COUNT];
    struct qos_vol_policy volPolicyCli[MAX_VOLUME_COUNT];
    int32_t minGuranteeVolId;
    uint64_t avgVolBw[M_PING_PONG_BUFFER][M_MAX_REACTORS];
    volState curVolState[MAX_VOLUME_COUNT];
    bool volMinPolicyInEffect;
    QosState currentState;
    uint64_t volBwDeficit;
    uint32_t consumerCount;
    struct qos_state_ctx context;
    MovingAvgCompute* movAvg;
    QosPolicyManager* qosPolicyManager;
};

class QosEventPolicyManager
{
public:
    QosEventPolicyManager(void);
    ~QosEventPolicyManager(void);
    void Initialize(QosPolicyManager* qosPolicyMgr);
    int SetEventPolicy(BackendEvent event, EventPriority priority, uint32_t weight);
    bool EventWrrPolicyInEffect(void);
    void ResetEventPolicy(void);
    void ResetAll(void);
    void UpdateEventParam(uint32_t eventId, event_qos_params eventParam);
    void ActiveEventClear(void);
    void FillEventState(BackendEvent event);
    eventState GetCurrentEventState(BackendEvent event);
    void CopyEventPolicy(void);

private:
    void _ApplyEventPolicy(void);
    void _ResetEventState(BackendEvent event);
    void _UpdateOldParamEventState(BackendEvent event);
    void _FillFlushEventState(BackendEvent event);
    void _FillGCEventState(BackendEvent event);
    void _FillUserRebuildEventState(BackendEvent event);
    void _FillMetaRebuildEventState(BackendEvent event);
    void _FillMetaEventState(BackendEvent event);
    void _FillFrontEventState(BackendEvent event);

    event_qos_params eventParams[BackendEvent_Count];
    std::atomic<bool> eventDirty;
    std::mutex eventPolicyLock;
    std::map<BackendEvent, std::pair<EventPriority, uint32_t>> eventPolicyMapCli;
    std::map<BackendEvent, std::pair<EventPriority, uint32_t>> eventPolicyMapQos;
    std::map<EventPriority, std::vector<int32_t>> eventPriorityWeights;
    eventState curEventState[BackendEvent_Count];
    bool eventWrrPolicyInEffect;
    QosPolicyManager* qosPolicyManager;
};

} // namespace pos

#endif // __IBOFOS_QOS_POLICY_MANAGER_H__

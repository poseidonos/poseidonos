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
#include "src/io/general_io/ubio.h"
#include "src/lib/singleton.h"
#include "src/qos/qos_common.h"
#include "src/qos/qos_data_manager.h"
#include "src/scheduler/event.h"
#include "src/scheduler/event_scheduler.h"
#include "src/sys_event/volume_event.h"

using namespace std;
namespace ibofos
{
class MovingAvgCompute;
class QosManager;
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Interface Class for event scheduling policy
 *
 */
/* --------------------------------------------------------------------------*/
class QosPolicyManager
{
public:
    QosPolicyManager();
    ~QosPolicyManager();
    void Initialize();
    void CheckSystem(uint64_t timeSpent);
    QosState GetCurrentState();
    void SetCurrentState(QosState state);
    bool MonitorParams(qos_state_ctx* ctx);
    uint32_t GetMinBWPolicy(int volId);
    uint32_t GetMaxBWPolicy(int volId);
    volume_qos_params volParams[MAX_VOLUME_COUNT];
    event_qos_params eventParams[BackendEvent_Count];
    QosWorkloadType GetWorkloadType(int volId);
    void InitilizeVolumePolicy(int volId);
    int UpdateVolumePolicy(int volId, qos_vol_policy volPolicy);
    qos_vol_policy GetVolumePolicy(int volId);
    void CopyVolumePolicy(void);
    void FillVolumeState(int volId);
    void ResetVolumeState(int volId);
    void ResetEventState(BackendEvent event);
    void FillEventState(BackendEvent event);
    void FillFlushEventState(BackendEvent event);
    void FillGCEventState(BackendEvent event);
    void FillUserRebuildEventState(BackendEvent event);
    void FillMetaRebuildEventState(BackendEvent event);
    void FillMetaEventState(BackendEvent event);
    void FillFrontEventState(BackendEvent event);
    void UpdateOldParamEventState(BackendEvent event);
    void UpdateOldParamVolumeState(int volId);
    bool IsQosCompromized(int volId, qos_state_ctx* ctx);
    void SelectQosPolicy(int voidId);
    void CorrectionManager(qos_state_ctx* ctx);
    bool ResetThrottling(qos_state_ctx* ctx);
    bool MonitorCorrection(qos_state_ctx* ctx);
    bool AvgBwCompute(int timeIndex);
    void IncreaseThrottling(uint32_t volId, uint64_t correction);
    void DecreaseThrottling(int volId, uint64_t correction);
    void ApplyThrottling(uint64_t throttleValue, bool increase);
    void ApplyCorrection(qos_state_ctx* context);
    void ApplyEventPolicy(void);
    std::map<int, int> activeVolMap;
    std::map<std::pair<uint32_t, uint32_t>, uint32_t> activeConnections;
    std::list<int> highConsumerVolList;
    std::list<int> lowConsumerVolList;
    std::list<int> intermediateConsumerVolList;
    uint16_t bufIndex;
    std::atomic<bool> policyDirty[MAX_VOLUME_COUNT];
    std::atomic<bool> eventDirty;
    std::mutex policyUpdateLock;
    struct qos_vol_policy volPolicy[MAX_VOLUME_COUNT];
    struct qos_vol_policy volPolicyCli[MAX_VOLUME_COUNT];
    int32_t minGuranteeVolId;
    int SetEventPolicy(BackendEvent event, EventPriority priority, uint32_t weight);
    void ResetEventPolicy(void);
    void CopyEventPolicy(void);
    void InitializeEventPolicy(void);
    bool VolumeMinimumPolicyInEffect(void);
    void ResetAll();

private:
    uint32_t qosCycle = 0;
    void _InitializeEventPriorityWeights();
    uint64_t avgVolBw[M_PING_PONG_BUFFER][M_MAX_REACTORS] = {0};
    QosState currentState;
    uint64_t volBwDeficit;
    uint32_t consumerCount;
    volState curVolState[MAX_VOLUME_COUNT];
    uint64_t volCycleExcess[MAX_VOLUME_COUNT];
    eventState curEventState[BackendEvent_Count];
    struct qos_state_ctx context;
    QosManager* qosMgr;
    MovingAvgCompute* movAvg;
    std::map<BackendEvent, std::pair<EventPriority, uint32_t>> eventPolicyMapCli;
    std::map<BackendEvent, std::pair<EventPriority, uint32_t>> eventPolicyMapQos;
    std::map<EventPriority, std::vector<int32_t>> eventPriorityWeights;
    bool volMinPolicyInEffect;
    bool eventWrrPolicyInEffect;
};

} // namespace ibofos

#endif // __IBOFOS_QOS_POLICY_MANAGER_H__

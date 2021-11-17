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

#include <utility>
#include <map>
#include <vector>

#include "src/qos/correction.h"
#include "src/qos/event_wrr_weight.h"
#include "src/qos/parameters.h"
#include "src/qos/parameters_all_events.h"
#include "src/qos/parameters_all_volumes.h"
#include "src/qos/resource.h"
#include "src/qos/resource_array.h"
#include "src/qos/resource_cpu.h"
#include "src/qos/resource_nvram.h"
#include "src/qos/throttle_all_volumes.h"
#include "src/qos/user_policy.h"
#include "src/qos/user_policy_all_volumes.h"
#include "src/qos/user_policy_rebuild.h"
#include "src/qos/qos_common.h"

namespace pos
{
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 */
/* --------------------------------------------------------------------------*/
class QosContext
{
public:
    QosContext(void);
    ~QosContext(void);
    void Reset(void);
    QosUserPolicy& GetQosUserPolicy(void);
    QosParameters& GetQosParameters(void);
    QosCorrection& GetQosCorrection(void);
    QosResource& GetQosResource(void);
    void ResetActiveVolume(void);
    void InsertActiveVolume(uint32_t volId);
    std::map<uint32_t, uint32_t>& GetActiveVolumes(void);
    void ResetActiveReactorVolume(void);
    void InsertActiveReactorVolume(uint32_t reactor, uint32_t volId);
    std::map<uint32_t, map<uint32_t, uint32_t>>& GetActiveVolumeReactors(void);
    void InsertActiveVolumeReactor(std::map<uint32_t, map<uint32_t, uint32_t>> map);
    std::map<uint32_t, map<uint32_t, uint32_t>>& GetActiveVolReactorMap(void);
    uint32_t GetActiveReactorVolumeCount(void);
    bool IsVolumeMinPolicyInEffect(void);
    void SetApplyCorrection(bool apply);
    bool GetApplyCorrection(void);
    void IncrementCorrectionCycle(void);
    bool IsCorrectionCycleOver(void);
    void SetTotalConnection(uint32_t volId, uint32_t value);
    uint32_t GetTotalConnection(uint32_t volId);
    void UpdateReactorCoreList(uint32_t reactorCore);
    std::vector<uint32_t>& GetReactorCoreList(void);
    void SetReactorProcessed(uint32_t reactorId, bool value);
    bool AllReactorsProcessed(void);
    void ResetAllReactorsProcessed(void);
    void InsertInactiveReactors(std::vector<uint32_t> inactiveReactors);
    std::vector<uint32_t> GetInactiveReactorsList(void);

private:
    QosUserPolicy userPolicy;
    QosParameters parameters;
    QosCorrection correction;
    QosResource resourceState;
    std::map<uint32_t, uint32_t> activeVolumeMap;
    std::map<pair<uint32_t, uint32_t>, uint32_t> activeReactorVolumeMap;
    std::map<uint32_t, map<uint32_t, uint32_t>> activeVolReactorMap;
    uint64_t timestamp;
    double elapsedTime;
    bool applyCorrection;
    bool volMinPolicy;
    bool userPolicyChange;
    bool correctionChange;
    bool resourceStateChange;
    uint32_t qosCorrectionCycle;
    uint32_t totalConnection[MAX_VOLUME_COUNT];
    std::vector<uint32_t> reactorCoreList;
    std::vector<uint32_t> inactiveReactorsList;
    std::atomic<bool> reactorProcessed[M_MAX_REACTORS];
};
} // namespace pos

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

#include <map>
#include <vector>

#include "src/spdk_wrapper/caller/spdk_pos_nvmf_caller.h"
#include "src/qos/internal_manager.h"
#include "src/qos/monitoring_manager_array.h"
#include "src/qos/qos_common.h"
#include "src/cpu_affinity/affinity_manager.h"

namespace pos
{
class QosContext;
class QosMonitoringManagerArray;
class QosManager;
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 */
/* --------------------------------------------------------------------------*/
class QosMonitoringManager : public QosInternalManager
{
public:
    explicit QosMonitoringManager(QosContext* qosCtx, QosManager* qosManager,
        SpdkPosNvmfCaller* spdkPosNvmfCaller = new SpdkPosNvmfCaller(),
        AffinityManager* affinityManager = AffinityManagerSingleton::Instance());
    ~QosMonitoringManager(void);
    void Execute(void) override;
    QosInternalManagerType GetNextManagerType(void) override;

private:
    void _SetNextManagerType(void);
    void _UpdateContextUserVolumePolicy(void);
    void _UpdateContextUserRebuildPolicy(void);
    void _UpdateContextResourceDetails(void);
    void _UpdateAllVolumeParameter(void);
    bool _GatherActiveVolumeParameters(void);
    void _GatherActiveEventParameters(void);
    void _UpdateEventParameter(BackendEvent event);
    QosContext* qosContext;
    QosInternalManagerType nextManagerType;
    std::map<uint32_t, map<uint32_t, uint32_t>> volReactorMap;
    std::map<uint32_t, map<uint32_t, uint32_t>> reactorVolMap;
    bw_iops_parameter eventParams[BackendEvent_Count];
    QosMonitoringManagerArray* qosMonitoringManagerArray[MAX_ARRAY_COUNT];
    std::map<uint32_t, uint32_t> prevActiveVolumeMap;
    QosManager* qosManager;
    SpdkPosNvmfCaller* spdkPosNvmfCaller;
    AffinityManager* affinityManager;
    std::map<uint32_t, vector<uint32_t>> inactiveReactors;
};
} // namespace pos

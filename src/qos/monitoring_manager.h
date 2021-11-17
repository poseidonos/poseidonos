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

#include <map>
#include <vector>

#include "src/qos/internal_manager.h"
#include "src/qos/qos_common.h"

namespace pos
{
class QosContext;
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 */
/* --------------------------------------------------------------------------*/
class QosMonitoringManager : public QosInternalManager
{
public:
    explicit QosMonitoringManager(QosContext* qosCtx);
    ~QosMonitoringManager(void);
    void Execute(void) override;
    QosInternalManagerType GetNextManagerType(void) override;

private:
    void _SetNextManagerType(void);
    void _UpdateContextUserVolumePolicy(void);
    void _UpdateContextUserRebuildPolicy(void);
    void _UpdateContextVolumeThrottle(uint32_t volId);
    void _UpdateContextVolumeParameter(uint32_t volId);
    void _UpdateContextResourceDetails(void);
    bool _GatherActiveVolumeParameters(void);
    void _UpdateContextActiveVolumes(uint32_t volId);
    void _UpdateContextActiveReactorVolumes(uint32_t reactor, uint32_t volId);
    void _UpdateContextActiveVolumeReactors(std::map<uint32_t, map<uint32_t, uint32_t>>& activeReactors, std::vector<uint32_t>& inactiveReactors);
    void _UpdateVolumeParameter(uint32_t volId);
    void _UpdateAllVolumeParameter(void);
    void _UpdateVolumeReactorParameter(uint32_t volId, uint32_t reactor);
    void _GatherActiveEventParameters(void);
    void _UpdateEventParameter(BackendEvent event);
    void _ComputeTotalActiveConnection(void);
    bool _VolParamActivities(uint32_t volId, uint32_t rector);
    QosContext* qosContext;
    QosInternalManagerType nextManagerType;
    std::map<uint32_t, uint32_t> volumeMap;
    std::map<uint32_t, map<uint32_t, uint32_t>> volReactorMap;
    std::map<uint32_t, map<uint32_t, uint32_t>> reactorVolMap;
    uint32_t totalConnection[MAX_VOLUME_COUNT];
    bw_iops_parameter volParams[MAX_VOLUME_COUNT];
    bw_iops_parameter eventParams[BackendEvent_Count];
    std::vector<uint32_t> inactiveReactors;
};
} // namespace pos

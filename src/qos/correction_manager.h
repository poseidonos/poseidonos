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
#include <utility>
#include <vector>

#include "src/qos/internal_manager.h"
#include "src/qos/qos_common.h"

namespace pos
{
class QosContext;
class QosManager;
class VolumeParameter;
class VolumeUserPolicy;
class IThrottlingLogic;
class ReactorHeap;
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 */
/* --------------------------------------------------------------------------*/
class QosCorrectionManager : public QosInternalManager
{
public:
    explicit QosCorrectionManager(QosContext* qosCtx, QosManager* qosManager);
    ~QosCorrectionManager(void);
    void Execute(void) override;
    QosInternalManagerType GetNextManagerType(void) override;
    void Reset(void) override;

private:
    void _SetNextManagerType(void);
    void ControlVolumeThrottling(void);
    void _HandleVolumeCorrection(void);
    void _HandleMaxThrottling(bool minPolicy);
    void _HandleMinThrottling(std::vector<std::pair<uint32_t, uint32_t>> volId);
    void _HandleWrrCorrection(void);
    void _DetermineWeight(std::vector<std::pair<uint32_t, uint32_t>>& minVolId,
        std::list<std::pair<uint32_t, uint32_t>> allMountedVolumeList, bool iops,
        uint64_t globalMaxWeight, uint64_t (*maxDeterminedWeight)[MAX_VOLUME_COUNT]);
    uint64_t _InitialValueCheck(uint64_t value, bool iops, VolumeParameter& volParameter, VolumeUserPolicy& volUserPolicy);
    void _ApplyCorrection(uint64_t value, bool iops, uint64_t volId, uint64_t reactor, uint64_t count, uint64_t totalConnection);
    uint64_t _GetUserMaxWeight(uint32_t arrayId, uint32_t volId, bool iops);
    uint64_t _GetUserMinWeight(uint32_t arrayId, uint32_t volId, bool iops);

    QosContext* qosContext;
    QosManager* qosManager;
    QosInternalManagerType nextManagerType;
    IThrottlingLogic *throttlingLogic;
    uint64_t INVALID_WEIGHT = 0xffffffff;
    uint64_t volumeBwThrottling[MAX_ARRAY_COUNT][MAX_VOLUME_COUNT];
    uint64_t volumeIopsThrottling[MAX_ARRAY_COUNT][MAX_VOLUME_COUNT];
    
};
} // namespace pos

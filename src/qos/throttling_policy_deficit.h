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

#include "src/qos/parameters_volume.h"
#include "src/qos/qos_common.h"
#include "src/qos/qos_context.h"
#include "src/qos/throttling_logic.h"
#include "src/qos/user_policy_volume.h"

namespace pos
{
class VolumeThrottle;
class QosManager;

class ThrottlingPolicyDeficit : public IThrottlingLogic
{
public:
    explicit ThrottlingPolicyDeficit(QosContext* qosCtx, QosManager* qosManager);
// LCOV_EXCL_START
    virtual ~ThrottlingPolicyDeficit(void)
    {
    }
// LCOV_EXCL_STOP
    virtual unsigned int GetNewWeight(uint32_t volId, uint32_t arrayId, VolumeThrottle* volumeThrottle) override;
    virtual bool GetCorrectionType(uint32_t volId, uint32_t arrayId) override;
    virtual void Reset(void) override;
    virtual void IncrementCycleCount(void) override;

private:
    void _GetBwIopsCorrection(int64_t& bwCorrection, int64_t& iopsCorrection, uint32_t volId, uint32_t arrayId, VolumeThrottle* volthrottle);
    uint64_t _InitialValueCheck(uint64_t value, bool iops, VolumeParameter& volParameter, VolumeUserPolicy& volUserPolicy);
    uint64_t _MinimumVolumeCorrection(VolumeThrottle* volumeThrottle, VolumeUserPolicy* volumeUserPolicy);
    bool _CheckValidInput(void);
    void _CalculateDeficit(int64_t& bwCorrection, int64_t& iopsCorrection);
    AllVolumeParameter& _GetAllVolumeParameters(void);
    AllVolumeUserPolicy& _GetUserPolicy(void);
    QosContext* qosContext;
    QosManager* qosManager;
    std::map<std::pair<uint32_t, uint32_t>, bool> correctionType;
    uint64_t storeCorrection;
    bool resetFlag;
    bool beginAgain;
    int noOfCycles;
    int64_t prevDeficit;
    int64_t totalDeficit;
    uint32_t totalCycleIter;
};
} // namespace pos

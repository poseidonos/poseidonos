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

#include "src/qos/user_policy_volume.h"

namespace pos
{
class VolumeUserPolicy;
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 */
/* --------------------------------------------------------------------------*/
class AllVolumeUserPolicy
{
public:
    AllVolumeUserPolicy(void);
    ~AllVolumeUserPolicy(void);
    void Reset(void);
    bool operator==(AllVolumeUserPolicy allVolPolicy);
    void InsertVolumeUserPolicy(uint32_t array, uint32_t vol, const VolumeUserPolicy& userPolicy);
    VolumeUserPolicy* GetVolumeUserPolicy(uint32_t array, uint32_t vol);
    void SetMinimumGuaranteeVolume(uint32_t volId, uint32_t arrayId);
    std::vector<std::pair<uint32_t, uint32_t>> GetMinimumGuaranteeVolume(void);
    void SetMinimumPolicyInEffect(bool value);
    void SetMinimumPolicyType(bool minBw);
    bool IsMinPolicyInEffect(void);
    bool IsMinBwPolicyInEffect(void);
    void SetMaxThrottlingChanged(bool value);
    bool IsMaxThrottlingChanged(void);

private:
    std::map<std::pair<uint32_t, uint32_t>, VolumeUserPolicy> volumeUserPolicyMap;
    std::vector<std::pair<uint32_t, uint32_t>> minGuaranteeVolume;
    bool maxThrottlingChanged;
    bool minInEffect;
    bool minBwGuarantee;
};
} // namespace pos

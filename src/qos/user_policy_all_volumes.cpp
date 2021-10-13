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
#include "src/qos/user_policy_all_volumes.h"

#include <utility>

#include "src/qos/helper_templates.h"
#include "src/qos/qos_common.h"

namespace pos
{
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
AllVolumeUserPolicy::AllVolumeUserPolicy(void)
{
    Reset();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
AllVolumeUserPolicy::~AllVolumeUserPolicy(void)
{
}

/* --------------------------------------------------------------------------*/
/**
 * @synopsis
 *
 * @returns
 */
/* --------------------------------------------------------------------------*/
void
AllVolumeUserPolicy::Reset(void)
{
    volumeUserPolicyMap.clear();
    minInEffect = false;
    minBwGuarantee = false;
    maxThrottlingChanged = false;
}

/* --------------------------------------------------------------------------*/
/**
 * @synopsis
 *
 * @returns
 */
/* --------------------------------------------------------------------------*/
void
AllVolumeUserPolicy::SetMaxThrottlingChanged(bool value)
{
    maxThrottlingChanged = value;
}

/* --------------------------------------------------------------------------*/
/**
 * @synopsis
 *
 * @returns
 */
/* --------------------------------------------------------------------------*/
bool
AllVolumeUserPolicy::IsMaxThrottlingChanged(void)
{
    return maxThrottlingChanged;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bool
AllVolumeUserPolicy::operator==(AllVolumeUserPolicy allVolPolicy)
{
    if (volumeUserPolicyMap == allVolPolicy.volumeUserPolicyMap)
    {
        return true;
    }

    return false;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
AllVolumeUserPolicy::InsertVolumeUserPolicy(uint32_t array, uint32_t vol, const VolumeUserPolicy& userPolicy)
{
    volumeUserPolicyMap[make_pair(array, vol)] = userPolicy;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
VolumeUserPolicy*
AllVolumeUserPolicy::GetVolumeUserPolicy(uint32_t array, uint32_t vol)
{
    auto search = volumeUserPolicyMap.find(make_pair(array, vol));
    if (search != volumeUserPolicyMap.end())
    {
        return &(search->second);
    }
    return nullptr;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
AllVolumeUserPolicy::SetMinimumGuaranteeVolume(uint32_t volId, uint32_t arrayId)
{
    for (auto i : minGuaranteeVolume)
    {
        if (i.first == arrayId && i.second == volId)
            return;
    }
    minGuaranteeVolume.push_back(std::make_pair(arrayId, volId));
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
std::vector<std::pair<uint32_t, uint32_t>>
AllVolumeUserPolicy::GetMinimumGuaranteeVolume(void)
{
    return minGuaranteeVolume;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
AllVolumeUserPolicy::SetMinimumPolicyInEffect(bool value)
{
    minInEffect = value;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
AllVolumeUserPolicy::SetMinimumPolicyType(bool minBw)
{
    minBwGuarantee = minBw;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bool
AllVolumeUserPolicy::IsMinPolicyInEffect(void)
{
    return minInEffect;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bool
AllVolumeUserPolicy::IsMinBwPolicyInEffect(void)
{
    return minBwGuarantee;
}
} // namespace pos

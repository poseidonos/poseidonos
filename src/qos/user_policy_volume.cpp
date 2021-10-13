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

#include "src/qos/user_policy_volume.h"

namespace pos
{
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
VolumeUserPolicy::VolumeUserPolicy(void)
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
VolumeUserPolicy::~VolumeUserPolicy(void)
{
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
VolumeUserPolicy::Reset(void)
{
    minBandwidth = 0;
    maxBandwidth = 0;
    minIops = 0;
    maxIops = 0;
    isMinVol = false;
    bwPolicy = false;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bool
VolumeUserPolicy::operator==(VolumeUserPolicy& volPolicy)
{
    if (minBandwidth != volPolicy.minBandwidth)
    {
        return false;
    }
    if (maxBandwidth != volPolicy.maxBandwidth)
    {
        return false;
    }
    if (minIops != volPolicy.minIops)
    {
        return false;
    }
    if (maxIops != volPolicy.maxIops)
    {
        return false;
    }

    return true;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
VolumeUserPolicy::SetMinBandwidth(uint64_t minBw)
{
    minBandwidth = minBw;
    if (minBw != DEFAULT_MIN_BW_PCS)
    {
        isMinVol = true;
        bwPolicy = true;
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
uint64_t
VolumeUserPolicy::GetMinBandwidth(void)
{
    return minBandwidth;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
VolumeUserPolicy::SetMaxBandwidth(uint64_t maxBw)
{
    maxBandwidth = maxBw;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
uint64_t
VolumeUserPolicy::GetMaxBandwidth(void)
{
    return maxBandwidth;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
VolumeUserPolicy::SetMaxIops(uint64_t iops)
{
    maxIops = iops;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
VolumeUserPolicy::SetMinIops(uint64_t iops)
{
    minIops = iops;
    if (minIops != DEFAULT_MIN_IOPS)
    {
        isMinVol = true;
        bwPolicy = false;
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
uint64_t
VolumeUserPolicy::GetMaxIops(void)
{
    return maxIops;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
uint64_t
VolumeUserPolicy::GetMinIops(void)
{
    return minIops;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bool
VolumeUserPolicy::IsBwPolicySet(void)
{
    return bwPolicy;
}
bool
VolumeUserPolicy::IsMinimumVolume(void)
{
    return isMinVol;
}
} // namespace pos

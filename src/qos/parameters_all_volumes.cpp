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

#include "src/qos/parameters_all_volumes.h"

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
AllVolumeParameter::AllVolumeParameter(void)
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
AllVolumeParameter::~AllVolumeParameter(void)
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
AllVolumeParameter::Reset(void)
{
    volumeParameterMap.clear();
    totalBwMap.clear();
    totalMinVolBwMap.clear();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
AllVolumeParameter::InsertVolumeParameter(uint32_t arrayId, uint32_t volId, const VolumeParameter& volParam)
{
    volumeParameterMap[make_pair(arrayId, volId)] = volParam;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bool
AllVolumeParameter::VolumeExists(uint32_t arrayId, uint32_t volId)
{
    auto search = volumeParameterMap.find(make_pair(arrayId, volId));
    if (search != volumeParameterMap.end())
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
VolumeParameter&
AllVolumeParameter::GetVolumeParameter(uint32_t arrayId, uint32_t volId)
{
    auto search = volumeParameterMap.find(make_pair(arrayId, volId));
    if (search != volumeParameterMap.end())
    {
        return search->second;
    }
    throw QosReturnCode::VOLUME_NOT_PRESENT;
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
AllVolumeParameter::IncrementMinVolumesBw(uint32_t arrayId, uint64_t bw)
{
    uint64_t bandwidth = 0;
    auto search = totalMinVolBwMap.find(arrayId);
    if (search!=  totalMinVolBwMap.end())
    {
        bandwidth = search->second;
    }
    bandwidth += bw;
    totalMinVolBwMap[arrayId] = bandwidth;
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
AllVolumeParameter::IncrementTotalBw(uint32_t arrayId, uint64_t totalBw)
{
    uint64_t bw = 0;
    auto search = totalBwMap.find(arrayId);
    if (search != totalBwMap.end())
    {
        bw = search->second;
    }
    bw = bw + totalBw;
    totalBwMap[arrayId] = bw;
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
uint64_t
AllVolumeParameter::GetTotalBw(uint32_t arrayId)
{
    auto search = totalBwMap.find(arrayId);
    if (search != totalBwMap.end())
    {
        return search->second;
    }
    return 0;
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

uint64_t
AllVolumeParameter::GetMinVolBw(uint32_t arrayId)
{
    auto search = totalMinVolBwMap.find(arrayId);
    if (search!= totalMinVolBwMap.end())
    {
        return search->second;
    }
    else
    {
        return 0;
    }
}
} // namespace pos

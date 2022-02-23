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

#include "src/qos/rate_limit.h"

namespace pos
{
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
BwIopsRateLimit::BwIopsRateLimit(void)
{
    for (uint32_t i = 0; i < MAX_REACTOR_WORKER; i++)
    {
        for (uint32_t j = 0; j < MAX_VOLUME_EVENT; j++)
        {
            iopsRateLimit[i][j] = DEFAULT_MAX_BW_IOPS;
            bwRateLimit[i][j] = DEFAULT_MAX_BW_IOPS;
        }
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
BwIopsRateLimit::~BwIopsRateLimit(void)
{
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bool
BwIopsRateLimit::IsLimitExceeded(uint32_t id1, uint32_t id2)
{
    bool rateExceeded = false;
    if (bwRateLimit[id1][id2] <= 0)
    {
        rateExceeded = true;
    }
    if (iopsRateLimit[id1][id2] <= 0)
    {
        rateExceeded = true;
    }
    return rateExceeded;
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
BwIopsRateLimit::ResetRateLimit(uint32_t id1, uint32_t id2, double offset,
        const int64_t bwLimit, const int64_t iopsLimit)
{
    int64_t remainingLimit = bwRateLimit[id1][id2];
    if (remainingLimit > 0)
    {
        remainingLimit = offset * bwLimit;
    }
    else
    {
        remainingLimit = remainingLimit + (offset * bwLimit);
    }
    bwRateLimit[id1][id2] = remainingLimit;
    remainingLimit = iopsRateLimit[id1][id2];
    if (remainingLimit > 0)
    {
        remainingLimit = offset * iopsLimit;
    }
    else
    {
        remainingLimit = remainingLimit + (offset * iopsLimit);
    }
    iopsRateLimit[id1][id2] = remainingLimit;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
BwIopsRateLimit::UpdateRateLimit(uint32_t id1, uint32_t id2, uint64_t bwVal)
{
    bwRateLimit[id1][id2] -= bwVal * POLLING_FREQ_PER_QOS_SLICE;
    iopsRateLimit[id1][id2] -= POLLING_FREQ_PER_QOS_SLICE;
}
} // namespace pos


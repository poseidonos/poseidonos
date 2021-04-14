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

#include "src/qos/qos_avg_compute.h"
namespace pos
{
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
MovingAvgCompute::MovingAvgCompute(int len)
: movingAvgWindowLength(len)
{
    for (int i = 0; i < MAX_VOLUME_COUNT; i++)
    {
        dataReadyToProcess[i] = false;
    }
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
MovingAvgCompute::~MovingAvgCompute(void)
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
MovingAvgCompute::ResetMovingAvg(int volId)
{
    uint64_t value = 0;
    dataReadyToProcess[volId] = false;
    buffer[volId] = 0;
    count[volId] = 0;
    averageBW[volId] = 0;
    do
    {
        value = DequeueAvgData(volId);
    } while (value != 0);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

void
MovingAvgCompute::ComputeMovingAvg(int volId, uint64_t data)
{
    uint64_t oldData = 0;
    if (dataReadyToProcess[volId] == false)
    {
        averageBW[volId] = (buffer[volId] + data) / movingAvgWindowLength;
        dataReadyToProcess[volId] = true;
    }
    else
    {
        oldData = DequeueAvgData(volId);
        averageBW[volId] = ((averageBW[volId] * (movingAvgWindowLength)) - oldData + data) / movingAvgWindowLength;
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
MovingAvgCompute::GetMovingAvg(int volId)
{
    return averageBW[volId];
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

bool
MovingAvgCompute::EnqueueAvgData(int volId, uint64_t data)
{
    avgQueue[volId].push(data);
    count[volId]++;
    if (count[volId] < (movingAvgWindowLength))
    {
        buffer[volId] += data;
    }
    else
    {
        ComputeMovingAvg(volId, data);
        count[volId]--;
    }
    return (dataReadyToProcess[volId]);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

uint64_t
MovingAvgCompute::DequeueAvgData(int volId)
{
    uint64_t ret = 0;
    if (avgQueue[volId].size() != 0)
    {
        ret = avgQueue[volId].front();
        avgQueue[volId].pop();
    }
    return ret;
}

}; // namespace pos

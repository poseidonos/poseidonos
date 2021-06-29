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

#include <cmath>
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
MovingAvgCompute::Initilize(void)
{
    std::queue<struct avgData> emptyQueue;
    for (uint32_t volId = 0; volId < MAX_VOLUME_COUNT; volId++)
    {
        buffer[volId] = 0;
        count[volId] = 0;
        averageBW[volId] = 0;
        bufferIops[volId] = 0;
        averageIops[volId] = 0;
        dataReadyToProcess[volId] = false;
        std::swap(avgQueue[volId], emptyQueue);
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
// Store the Max value for other volumes, only do Avg for min Gurantee Vol
void
MovingAvgCompute::_ComputeMovingAvg(int volId, avgData data)
{
    avgData oldData;
    if (dataReadyToProcess[volId] == false)
    {
        averageBW[volId] = (buffer[volId] + data.bw) / movingAvgWindowLength;
        averageIops[volId] = (bufferIops[volId] + data.iops) / movingAvgWindowLength;
        dataReadyToProcess[volId] = true;
    }
    else
    {
        oldData = _DequeueAvgData(volId);
        double value = ((averageBW[volId] * (movingAvgWindowLength)) - oldData.bw + data.bw);
        value = round(value / movingAvgWindowLength);
        averageBW[volId] = value;

        value = ((averageIops[volId] * (movingAvgWindowLength)) - oldData.iops + data.iops);
        value = round(value / movingAvgWindowLength);
        averageIops[volId] = value;
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
MovingAvgCompute::GetMovingAvgBw(int volId)
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
uint64_t
MovingAvgCompute::GetMovingAvgIops(int volId)
{
    return averageIops[volId];
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bool
MovingAvgCompute::EnqueueAvgData(int volId, uint64_t bw, uint64_t iops)
{
    avgData data;
    data.bw = bw;
    data.iops = iops;
    avgQueue[volId].push(data);
    count[volId]++;
    if (count[volId] < (movingAvgWindowLength))
    {
        buffer[volId] += bw;
        bufferIops[volId] += iops;
    }
    else
    {
        _ComputeMovingAvg(volId, data);
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
avgData
MovingAvgCompute::_DequeueAvgData(int volId)
{
    avgData ret;
    if (avgQueue[volId].size() != 0)
    {
        ret = avgQueue[volId].front();
        avgQueue[volId].pop();
    }
    return ret;
}
}; // namespace pos

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

#ifndef __IBOFOS_QOS_AVG_COMPUTE_H__
#define __IBOFOS_QOS_AVG_COMPUTE_H__

#include <iostream>
#include <list>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>

#include "src/lib/singleton.h"
#include "src/qos/qos_common.h"

using namespace std;
namespace pos
{
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Interface Class for event scheduling policy
 *
 */
/* --------------------------------------------------------------------------*/
class MovingAvgCompute
{
public:
    explicit MovingAvgCompute(int len);
    ~MovingAvgCompute(void);
    bool EnqueueAvgData(int volId, uint64_t data);
    void ComputeMovingAvg(int volId, uint64_t data);
    uint64_t DequeueAvgData(int voldId);
    uint64_t GetMovingAvg(int volId);
    void ResetMovingAvg(int volId);

private:
    std::queue<uint64_t> avgQueue[MAX_VOLUME_COUNT];
    uint64_t buffer[MAX_VOLUME_COUNT];
    int count[MAX_VOLUME_COUNT];
    uint64_t averageBW[MAX_VOLUME_COUNT];
    int movingAvgWindowLength;
    bool dataReadyToProcess[MAX_VOLUME_COUNT];
};

} // namespace pos

#endif // __IBOFOS_QOS_AVG_COMPUTE_H__

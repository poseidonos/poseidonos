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

#include <atomic>
#include <functional>
#include <string>
#include <mutex>

#include "rebuild_result.h"
#include "rebuild_progress.h"
#include "rebuild_logger.h"
#include "src/include/rebuild_state.h"
#include "src/include/address_type.h"
#include "src/include/raid_type.h"
#include "src/logger/logger.h"

using namespace std;

namespace pos
{
class PartitionPhysicalSize;

using F2PTranslator = function<PhysicalBlkAddr(const FtBlkAddr&)>;
using RebuildComplete = function<void(RebuildResult)>;

class RebuildContext
{
public:
    string array;
    string part;
    RaidType raidType;
    uint32_t arrayIndex = 0;
    uint32_t faultIdx = 0;
    ArrayDevice* faultDev = nullptr;
    uint64_t stripeCnt = 0;
    atomic<uint32_t> taskCnt;
    const PartitionPhysicalSize* size = nullptr;
    RebuildProgress* prog = nullptr;
    RebuildLogger* logger = nullptr;
    F2PTranslator translate;
    RebuildComplete rebuildComplete;
    RebuildState GetResult()
    {
        unique_lock<mutex> lock(mtx);
        return result;
    }
    void SetResult(RebuildState reqState)
    {
        unique_lock<mutex> lock(mtx);
        if (reqState == RebuildState::REBUILDING)
        {
            if (result == RebuildState::READY)
            {
                result = RebuildState::REBUILDING;
            }
        }
        else
        {
            result = reqState;
        }
    }

private:
    RebuildState result = RebuildState::READY;
    mutex mtx;
};
} // namespace pos

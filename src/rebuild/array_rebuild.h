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

#pragma once

#include "src/include/rebuild_state.h"
#include "src/array/rebuild/rebuild_progress.h"
#include "src/array/rebuild/rebuild_context.h"

#include <list>
#include <mutex>
#include <string>
#include <condition_variable>

using namespace std;

namespace pos
{
class PartitionRebuild;
class ArrayDevice;
class RebuildTarget;

class ArrayRebuild
{
public:
    ArrayRebuild(string array, ArrayDevice* dev, RebuildComplete cb,
            list<RebuildTarget*> tgt);
    void Start(void);
    void Discard(void);
    void Stop(void);
    RebuildState GetState(void);
    uint64_t GetProgress(void);

private:
    void _RebuildNext(void);
    void _RebuildDone(RebuildResult res);
    void _RebuildCompleted(RebuildResult res);
    void _WaitRebuildStop(void);
    string arrayName = "";
    ArrayDevice* targetDev = nullptr;
    RebuildState state = RebuildState::READY;
    RebuildProgress* progress = nullptr;
    RebuildComplete rebuildComplete;
    RebuildComplete rebuildDoneCb;
    list<PartitionRebuild*> tasks;
    mutex mtx;
    condition_variable cv;
};
} // namespace pos

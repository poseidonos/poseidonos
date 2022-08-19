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

#include "array_rebuild.h"
#include "src/array/rebuild/i_array_rebuilder.h"
#include "src/rebuild/interface/i_rebuild_notification.h"

#include <string>
#include <map>
#include <list>
#include <vector>
#include <mutex>
#include <utility>
#include <condition_variable>

using namespace std;

namespace pos
{
class ArrayRebuilder : public IArrayRebuilder
{
public:
    ArrayRebuilder(IRebuildNotification* noti);
    virtual ~ArrayRebuilder() {}
    void Rebuild(string array, uint32_t arrayId, vector<IArrayDevice*> dst,
        RebuildComplete cb, list<RebuildTarget*>& tgt) override;
    void QuickRebuild(string array, uint32_t arrayId, QuickRebuildPair rebuildPair,
        RebuildComplete cb, list<RebuildTarget*>& tgt) override;
    void StopRebuild(string arrayname) override;
    void RebuildDone(RebuildResult result) override;
    void WaitRebuildDone(string arrayname) override;
    bool IsRebuilding(string arrayname) override;
    uint32_t GetRebuildProgress(string arrayname) override;

private:
    int _PrepareRebuild(string arrayname, list<RebuildTarget*>& tgt);
    ArrayRebuild* _Find(string arrayname);
    map<string, ArrayRebuild*> jobsInProgress;
    IRebuildNotification* iRebuildNoti = nullptr;
    std::mutex mtxStart;
    std::condition_variable cv;
};
} // namespace pos

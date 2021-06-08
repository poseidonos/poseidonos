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

#include "array_rebuilder.h"
#include "array_rebuild.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
ArrayRebuilder::ArrayRebuilder(IRebuildNotification* noti)
: iRebuildNoti(noti)
{
}

void
ArrayRebuilder::Rebuild(string array, ArrayDevice* dev,
    RebuildComplete cb, list<RebuildTarget*> tgt)
{
    POS_TRACE_INFO((int)POS_EVENT_ID::REBUILD_DEBUG_MSG,
        "ArrayRebuilder::Rebuild {}", array);

    mtxStart.lock();
    StopRebuild(array);
    WaitRebuildDone(array);
    POS_TRACE_INFO((int)POS_EVENT_ID::REBUILD_DEBUG_MSG,
        "ArrayRebuilder::Rebuild, start job");

    bool resume = false;
    int ret = iRebuildNoti->PrepareRebuild(array, resume);

    if (resume)
    {
        // remove meta parttition frome tgt
        tgt.pop_front();
    }

    ArrayRebuild* job = new ArrayRebuild(array, dev, cb, tgt);
    jobsInProgress.emplace(array, job);

    if (ret == 0)
    {
        mtxStart.unlock();
        job->Start();
    }
    else
    {
        job->Discard();
    }
}

void
ArrayRebuilder::StopRebuild(string array)
{
    ArrayRebuild* jobInProg = _Find(array);
    if (jobInProg != nullptr)
    {
        POS_TRACE_INFO((int)POS_EVENT_ID::REBUILD_DEBUG_MSG,
            "ArrayRebuilder::StopRebuild, {}", array);
        jobInProg->Stop();
    }
}

void
ArrayRebuilder::RebuildDone(string array)
{
    iRebuildNoti->RebuildDone(array);
    unique_lock<mutex> lock(mtxWait);
    ArrayRebuild* job = _Find(array);
    if (job != nullptr)
    {
        delete job;
        jobsInProgress.erase(array);
    }
    POS_TRACE_INFO((int)POS_EVENT_ID::REBUILD_DEBUG_MSG,
        "ArrayRebuilder::RebuildDone {}, inProgressCnt:{}",
        array, jobsInProgress.size());
    cv.notify_all();
}

void
ArrayRebuilder::WaitRebuildDone(string array)
{
    unique_lock<mutex> lock(mtxWait);
    while (_Find(array) != nullptr)
    {
        cv.wait(lock);
    }
}

uint32_t
ArrayRebuilder::GetRebuildProgress(string array)
{
    ArrayRebuild* jobInProg = _Find(array);
    if (jobInProg != nullptr)
    {
        return jobInProg->GetProgress();
    }
    return 0;
}

ArrayRebuild*
ArrayRebuilder::_Find(string array)
{
    if (array == "" && jobsInProgress.size() == 1)
    {
        return jobsInProgress.begin()->second;
    }

    auto it = jobsInProgress.find(array);
    if (it == jobsInProgress.end())
    {
        return nullptr;
    }

    return it->second;
}
} // namespace pos

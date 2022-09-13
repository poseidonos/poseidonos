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

#include "array_rebuilder.h"
#include "array_rebuild.h"
#include "rebuild_behavior_factory.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
ArrayRebuilder::ArrayRebuilder(IRebuildNotification* noti)
: iRebuildNoti(noti)
{
}

void
ArrayRebuilder::Rebuild(string array, uint32_t arrayId, vector<IArrayDevice*> dst,
    RebuildComplete cb, list<RebuildTarget*>& tgt)
{
    POS_TRACE_INFO(EID(REBUILD_DEBUG_MSG),
        "Rebuild requested, array:{}, dstCnt:{}, targetPartCnt:{}", array, dst.size(), tgt.size());

    mtxStart.lock();
    if (_Find(array) != nullptr)
    {
        POS_TRACE_ERROR(EID(REBUILD_DEBUG_MSG),
            "The rebuild is already in progress, array:{}", array);
        mtxStart.unlock();
        return;
    }
    int ret = _PrepareRebuild(array, tgt);
    RebuildBehaviorFactory factory(AllocatorServiceSingleton::Instance()->GetIContextManager(array));
    ArrayRebuild* job = new ArrayRebuild(array, arrayId, dst, cb, tgt, &factory);
    jobsInProgress.emplace(array, job);
    mtxStart.unlock();

    if (ret == 0)
    {
        job->Start();
    }
    else
    {
        job->Discard();
    }
}

void
ArrayRebuilder::QuickRebuild(string array, uint32_t arrayId, QuickRebuildPair rebuildPair,
    RebuildComplete cb, list<RebuildTarget*>& tgt)
{
    // todo quick rebuild implementation
    POS_TRACE_INFO(EID(REBUILD_DEBUG_MSG),
        "QuickRebuild requested, array:{}, pair_cnt:{}, targetPartCnt:{}", array, rebuildPair.size(), tgt.size());

    mtxStart.lock();
    if (_Find(array) != nullptr)
    {
        POS_TRACE_ERROR(EID(REBUILD_DEBUG_MSG),
            "The rebuild is already in progress, array:{}", array);
        mtxStart.unlock();
        return;
    }

    int ret = _PrepareRebuild(array, tgt);
    RebuildBehaviorFactory factory(AllocatorServiceSingleton::Instance()->GetIContextManager(array));
    ArrayRebuild* job = new ArrayRebuild(array, arrayId, rebuildPair, cb, tgt, &factory);
    jobsInProgress.emplace(array, job);
    mtxStart.unlock();

    if (ret == 0)
    {
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
    mtxStart.lock();
    POS_TRACE_INFO(EID(REBUILD_DEBUG_MSG),
        "ArrayRebuilder::StopRebuild, Try");
    ArrayRebuild* jobInProg = _Find(array);
    if (jobInProg != nullptr)
    {
        jobInProg->Stop();
        POS_TRACE_INFO(EID(REBUILD_DEBUG_MSG),
            "ArrayRebuilder::StopRebuild, {}", array);
    }
    mtxStart.unlock();
}

void
ArrayRebuilder::RebuildDone(RebuildResult result)
{
    string array = result.array;
    iRebuildNoti->RebuildDone(array);
    unique_lock<mutex> lock(mtxStart);
    ArrayRebuild* job = _Find(array);
    if (job != nullptr)
    {
        delete job;
        jobsInProgress.erase(array);
    }
    POS_TRACE_INFO(EID(REBUILD_DEBUG_MSG),
        "ArrayRebuilder::RebuildDone {}, inProgressCnt:{}",
        array, jobsInProgress.size());
    cv.notify_all();
}

void
ArrayRebuilder::WaitRebuildDone(string array)
{
    unique_lock<mutex> lock(mtxStart);
    while (_Find(array) != nullptr)
    {
        cv.wait(lock);
    }
}

bool
ArrayRebuilder::IsRebuilding(string array)
{
    if ( _Find(array) != nullptr)
    {
        return true;
    }

    return false;
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

int
ArrayRebuilder::_PrepareRebuild(string arrayname, list<RebuildTarget*>& tgt)
{
    POS_TRACE_INFO(EID(REBUILD_DEBUG_MSG), "{} is preparing rebuild", arrayname);
    bool resume = false;
    int ret = iRebuildNoti->PrepareRebuild(arrayname, resume);
    if (ret == 0)
    {
        POS_TRACE_INFO(EID(REBUILD_DEBUG_MSG),
            "{} is ready to rebuild. isResume: {}", arrayname, resume);
    }
    else
    {
         POS_TRACE_WARN(EID(REBUILD_DEBUG_MSG),
            "{} is failed to prepare rebuild and will be discarded, isResume: {}, prepare_result: {}",
            arrayname, resume, ret);
    }
    if (resume)
    {
        for (auto it : tgt)
        {
            if (it->IsResumable() == false)
            {
                tgt.remove(it);
                break;
            }
        }
    }
    return ret;
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

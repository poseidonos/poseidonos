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

#include "array_rebuild.h"

#include <functional>

#include "src/rebuild/partition_rebuild.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
namespace pos
{
ArrayRebuild::ArrayRebuild(string arrayName, uint32_t arrayId, vector<IArrayDevice*>& dst,
    RebuildComplete cb, list<RebuildTarget*>& tgt, RebuildBehaviorFactory* factory)
: arrayName(arrayName),
  rebuildComplete(cb)
{
    POS_TRACE_INFO(EID(ARRAY_REBUILD_INIT),
        "array_name:{}, taskCnt:{}, dstCnt:{}",
        arrayName, tgt.size(), dst.size());

    RebuildProgress* prog = new RebuildProgress(arrayName);
    RebuildLogger* rLogger = new RebuildLogger(arrayName);

    for (RebuildTarget* tar : tgt)
    {
        unique_ptr<RebuildContext> ctx = tar->GetRebuildCtx(dst);
        if (ctx && factory != nullptr)
        {
            ctx->array = arrayName;
            ctx->arrayIndex = arrayId;
            POS_TRACE_INFO(EID(ARRAY_REBUILD_INIT),
                "Trying to create partition rebuild, part:{}", PARTITION_TYPE_STR[ctx->part]);
            RebuildBehavior* bhvr = factory->CreateRebuildBehavior(move(ctx));
            if (bhvr != nullptr)
            {
                bhvr->GetContext()->prog = prog;
                bhvr->GetContext()->logger = rLogger;
                bhvr->UpdateProgress(0);
                PartitionRebuild* ptnRbd = new PartitionRebuild(bhvr);
                if (ptnRbd->TotalStripes() > 0)
                {
                    tasks.push_back(ptnRbd);
                }
                else
                {
                    delete ptnRbd;
                }
            }
        }
    }
    progress = prog;
    rebuildLogger = rLogger;
    rebuildLogger->SetArrayRebuildStart();
    rebuildDoneCb = bind(&ArrayRebuild::_PartitionRebuildDone, this, placeholders::_1);
}

ArrayRebuild::ArrayRebuild(string arrayName, uint32_t arrayId, QuickRebuildPair& rebuildPair,
    RebuildComplete cb, list<RebuildTarget*>& tgt, RebuildBehaviorFactory* factory)
: arrayName(arrayName),
  rebuildComplete(cb)
{
    POS_TRACE_INFO(EID(ARRAY_REBUILD_INIT),
        "QuickRebuild, array_name:{}, taskCnt:{}, pairCnt:{}",
        arrayName, tgt.size(), rebuildPair.size());
    RebuildProgress* prog = new RebuildProgress(arrayName);
    RebuildLogger* rLogger = new RebuildLogger(arrayName);

    for (RebuildTarget* tar : tgt)
    {
        unique_ptr<RebuildContext> ctx = tar->GetQuickRebuildCtx(rebuildPair);
        if (ctx && factory != nullptr)
        {
            ctx->array = arrayName;
            ctx->arrayIndex = arrayId;
            POS_TRACE_INFO(EID(ARRAY_REBUILD_INIT),
                "Trying to create partition rebuild, part:{}, pairCnt:{}", PARTITION_TYPE_STR[ctx->part], ctx->rp.size());
            RebuildBehavior* bhvr = factory->CreateRebuildBehavior(move(ctx));
            if (bhvr != nullptr)
            {
                bhvr->GetContext()->prog = prog;
                bhvr->GetContext()->logger = rLogger;
                bhvr->UpdateProgress(0);
                PartitionRebuild* ptnRbd = new PartitionRebuild(bhvr);
                if (ptnRbd->TotalStripes() > 0)
                {
                    tasks.push_back(ptnRbd);
                }
                else
                {
                    delete ptnRbd;
                }
            }
        }
    }
    progress = prog;
    rebuildLogger = rLogger;
    rebuildLogger->SetArrayRebuildStart();
    rebuildDoneCb = bind(&ArrayRebuild::_PartitionRebuildDone, this, placeholders::_1);
}

ArrayRebuild::~ArrayRebuild(void)
{
    delete progress;
    delete rebuildLogger;
}

void
ArrayRebuild::Start(void)
{
    POS_TRACE_INFO(EID(ARRAY_REBUILD_START),
        "array_name:{}, taskCnt:{}", arrayName, tasks.size());

    if (tasks.empty())
    {
        RebuildResult res;
        res.array = arrayName;
        res.result = RebuildState::READY;
        _RebuildCompleted(res);
    }
    else
    {
        _RebuildNextPartition();
    }
}

void
ArrayRebuild::Discard(void)
{
    POS_TRACE_ERROR(EID(ARRAY_REBUILD_DISCARD), "array_name:{}", arrayName);
    tasks.clear();
    RebuildResult res;
    res.array = arrayName;
    res.result = RebuildState::FAIL;
    _RebuildCompleted(res);
}

void
ArrayRebuild::Stop(void)
{
    for (PartitionRebuild* task : tasks)
    {
        task->Stop();
    }
}

RebuildState
ArrayRebuild::GetState(void)
{
    return state;
}

uint64_t
ArrayRebuild::GetProgress(void)
{
    if (progress != nullptr)
    {
        return progress->Current();
    }
    return 0;
}

void
ArrayRebuild::_RebuildNextPartition(void)
{
    POS_TRACE_INFO(EID(REBUILD_NEXT_PARTITION),
        "array_name:{}, remainingTaskCnt:{}", arrayName, tasks.size());
    if (tasks.empty() == false)
    {
        PartitionRebuild* task = tasks.front();
        state = RebuildState::REBUILDING;
        task->Start(rebuildDoneCb);
    }
}

void
ArrayRebuild::_PartitionRebuildDone(RebuildResult res)
{
    POS_TRACE_INFO(EID(PARTITION_REBUILD_END),
        "array_name:{}, remainingTaskCnt:{}", arrayName, tasks.size());
    RebuildState taskResult = res.result;
    state = taskResult;

    mtx.lock();
    PartitionRebuild* task = tasks.front();
    delete task;
    tasks.pop_front();
    if (taskResult == RebuildState::CANCELLED ||
        taskResult == RebuildState::FAIL)
    {
        while (tasks.empty() == false)
        {
            PartitionRebuild* t = tasks.front();
            delete t;
            tasks.pop_front();
        }
    }
    mtx.unlock();

    if (tasks.empty() == true)
    {
        _RebuildCompleted(res);
    }
    else
    {
        _RebuildNextPartition();
    }
}

void
ArrayRebuild::_RebuildCompleted(RebuildResult res)
{
    POS_TRACE_INFO(EID(ARRAY_REBUILD_END),
        "array_name:{}, result:{}", arrayName,res.result);
    state = res.result;
    switch (state)
    {
        case RebuildState::PASS:
        {
            POS_TRACE_INFO(EID(REBUILD_RESULT_PASS),
                "array_name:{}", arrayName);
            rebuildLogger->SetResult(REBUILD_STATE_STR[(int)state]);
            break;
        }
        case RebuildState::FAIL:
        {
            POS_TRACE_INFO(EID(REBUILD_RESULT_FAILED),
                "array_name:{}", arrayName);
            rebuildLogger->SetResult(REBUILD_STATE_STR[(int)state]);
            break;
        }
        case RebuildState::CANCELLED:
        {
            POS_TRACE_INFO(EID(REBUILD_RESULT_CANCELLED),
                "array_name:{}", arrayName);
            rebuildLogger->SetResult(REBUILD_STATE_STR[(int)state]);
            break;
        }
        default:
            break;
    }
    rebuildLogger->WriteLog();
    if (rebuildComplete != nullptr)
    {
        rebuildComplete(res);
    }
}
} // namespace pos

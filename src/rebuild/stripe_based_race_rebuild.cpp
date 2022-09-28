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
#include "stripe_based_race_rebuild.h"
#include "rebuilder.h"
#include "rebuild_completed.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/include/branch_prediction.h"
#include "src/include/backend_event.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/array/service/array_service_layer.h"
#include "src/include/address_type.h"

namespace pos
{
StripeBasedRaceRebuild::StripeBasedRaceRebuild(unique_ptr<RebuildContext> c)
: RebuildBehavior(move(c))
{
    POS_TRACE_DEBUG(EID(STRIPES_REBUILD_INIT), "StripeBasedRaceRebuild");
    locker = ArrayService::Instance()->Getter()->GetIOLocker(ctx->part);
    assert(locker != nullptr);
}

StripeBasedRaceRebuild::~StripeBasedRaceRebuild(void)
{
}

bool
StripeBasedRaceRebuild::Rebuild(void)
{
    if (isInitialized == false)
    {
        bool ret = _Init();
        if (ret == true)
        {
            return _Recover();
        }
        return false;
    }
    return _Recover();
}

void StripeBasedRaceRebuild::UpdateProgress(uint32_t val)
{
    ctx->prog->Update(PARTITION_TYPE_STR[ctx->part], val, ctx->stripeCnt);
}

bool
StripeBasedRaceRebuild::_Init(void)
{
    if (isInitialized == false)
    {
        uint32_t idx = 1;
        for (RebuildMethod* rm : ctx->rm)
        {
            bool ret = rm->Init("StripeBasedRaceRebuild_" + ctx->array + "_" + PARTITION_TYPE_STR[ctx->part] + "_" + to_string(idx));
            if (ret == false)
            {
                if (initRetryCnt >= INIT_REBUILD_MAX_RETRY)
                {
                    POS_TRACE_ERROR(EID(REBUILD_BUFFER_INIT_FAIL), "part_type:{}, retried:{}",
                        PARTITION_TYPE_STR[ctx->part], initRetryCnt);
                    ctx->SetResult(RebuildState::FAIL);
                    return _Finish();
                }
                int logInterval = INIT_REBUILD_MAX_RETRY / 10;
                if (initRetryCnt % logInterval == 0)
                {
                    POS_TRACE_WARN(EID(REBUILD_BUFFER_INIT_RETRY), "array:{}, part:{}, retried:{}",
                        ctx->array, PARTITION_TYPE_STR[ctx->part], initRetryCnt);
                }
                initRetryCnt++;
                return false;
            }
            idx++;
        }
        for (auto rp : ctx->rp)
        {
            for (IArrayDevice* dev : rp->dsts)
            {
                targetDevs.insert(dev);
            }
        }
        POS_TRACE_INFO(EID(REBUILD_BUFFER_INIT_OK), "part:{}, targetDevsCnt:{}, rmCnt:{}",
            PARTITION_TYPE_STR[ctx->part], targetDevs.size(), ctx->rm.size());
        isInitialized = true;
        initRetryCnt = 0;
    }
    return true;
}

bool
StripeBasedRaceRebuild::_Recover(void)
{
    uint32_t strPerSeg = ctx->size->stripesPerSegment;
    uint32_t maxStripeId = ctx->size->totalSegments * strPerSeg - 1;
    if (baseStripe >= maxStripeId ||
        ctx->GetResult() >= RebuildState::CANCELLED)
    {
        return _Finish();
    }
    uint32_t from = baseStripe;
    uint32_t to = baseStripe + strPerSeg - 1;
    if (to > maxStripeId)
    {
        to = maxStripeId;
    }
    int ret = _TryLock(from, to);
    if (ret != 0)
    {
        if (ret == EID(REBUILD_TRY_LOCK_FAILED))
        {
            return true;
        }
        return false;
    }
    uint32_t currWorkload = to - from + 1;
    uint32_t callbackCnt = ctx->rm.size();
    ctx->taskCnt = currWorkload * callbackCnt;
    POS_TRACE_INFO(EID(STRIPES_REBUILD_BEGIN),
        "from:{}, to:{}, taskCnt:{}", from, to, ctx->taskCnt);
    for (uint32_t offset = 0; offset < currWorkload; offset++)
    {
        uint32_t stripeId = baseStripe + offset;
        for (RebuildMethod* rm : ctx->rm)
        {
            StripeRebuildDoneCallback callback = bind(&StripeBasedRaceRebuild::_RecoverCompleted, this, stripeId, placeholders::_1);
            int ret = rm->Recover(ctx->arrayIndex, stripeId, ctx->size, callback);
            if (ret != 0)
            {
                POS_TRACE_WARN(ret,
                    "stripeId:{} part:{}, maxStripes:{}",
                    stripeId, PARTITION_TYPE_STR[ctx->part], maxStripeId);
                ctx->SetResult(RebuildState::FAIL);
                return _Finish();
            }
        }
    }
    UpdateProgress(baseStripe);
    baseStripe += currWorkload;
    return true;
}

void
StripeBasedRaceRebuild::_RecoverCompleted(uint32_t targetId, int result)
{
    for (IArrayDevice* dev : targetDevs)
    {
        locker->Unlock(dev, targetId);
    }

    if (result != 0)
    {
        ctx->SetResult(RebuildState::FAIL);
        POS_TRACE_WARN(result,
            "stripeId:{}, result:{}", targetId, result);
    }

    uint32_t currentTaskCnt = ctx->taskCnt -= 1;
    if (currentTaskCnt == 0)
    {
        POS_TRACE_INFO(EID(STRIPES_REBUILD_DONE),
            "result:{}", (int)(ctx->GetResult()));
        EventSmartPtr nextEvent(new Rebuilder(this));
        nextEvent->SetEventType(BackendEvent_MetadataRebuild);
        EventSchedulerSingleton::Instance()->EnqueueEvent(nextEvent);
    }
}

bool
StripeBasedRaceRebuild::_Finish(void)
{
    UpdateProgress(baseStripe);
    POS_TRACE_DEBUG(EID(STRIPES_REBUILD_FINISH), "part:{}", PARTITION_TYPE_STR[ctx->part]);
    for (IArrayDevice* targetDev : targetDevs)
    {
        if (locker->ResetBusyLock(targetDev) == false)
        {
            int logInterval = TRY_LOCK_MAX_RETRY / 10;
            if (resetLockRetryCnt % logInterval == 0)
            {
                locker->WriteBusyLog(targetDev);
                POS_TRACE_WARN(EID(REBUILD_RESET_LOCK_RETRY),
                    "part:{}, retried:{}, target:{}",
                    PARTITION_TYPE_STR[ctx->part], resetLockRetryCnt, targetDev->GetName());
                int sleep = resetLockRetryCnt * 10;
                usleep(sleep);
            }
            resetLockRetryCnt++;
            if (resetLockRetryCnt >= TRY_LOCK_MAX_RETRY)
            {
                POS_TRACE_WARN(EID(REBUILD_FORCED_RESET_LOCK), "part:{}, retried:{}, target:{}",
                    PARTITION_TYPE_STR[ctx->part], resetLockRetryCnt, targetDev->GetName());
                bool forceReset = true;
                locker->ResetBusyLock(targetDev, forceReset);
                resetLockRetryCnt = 0;
                return true;
            }
            return false;
        }
        else
        {
            resetLockRetryCnt = 0;
            POS_TRACE_INFO(EID(REBUILD_RESET_LOCK_OK), "target:{}",
                targetDev->GetName());
        }
    }
    if (ctx->GetResult() == RebuildState::CANCELLED)
    {
        POS_TRACE_WARN(EID(REBUILD_RESULT_CANCELLED),
            "Partition {} ({}) rebuilding stopped",
            PARTITION_TYPE_STR[ctx->part]);
    }
    else if (ctx->GetResult() == RebuildState::FAIL)
    {
        POS_TRACE_WARN(EID(REBUILD_RESULT_FAILED),
            "Partition {} ({}) rebuilding failed",
            PARTITION_TYPE_STR[ctx->part]);
    }
    else
    {
        POS_TRACE_INFO(EID(REBUILD_RESULT_PASS),
            "Partition {} ({}) rebuilding complete successfully",
            PARTITION_TYPE_STR[ctx->part]);
        ctx->SetResult(RebuildState::PASS);
        UpdateProgress(ctx->stripeCnt);
    }

    EventSmartPtr complete(new RebuildCompleted(this));
    complete->SetEventType(BackendEvent_MetadataRebuild);
    EventSchedulerSingleton::Instance()->EnqueueEvent(complete);
    baseStripe = 0;
    return true;
}

int
StripeBasedRaceRebuild::_TryLock(uint32_t from, uint32_t to)
{
    IArrayDevice* failedDev;
    if (locker->TryBusyLock(targetDevs, from, to, failedDev) == false)
    {
        int logInterval = TRY_LOCK_MAX_RETRY / 10;
        if (tryLockRetryCnt % logInterval == 0)
        {
            locker->WriteBusyLog(failedDev);
            POS_TRACE_WARN(EID(REBUILD_TRY_LOCK_RETRY),
                "array_name:{}, part:{}, dev_name:{}, stripe_from:{}, stripe_to:{}, retried:{}",
                ctx->array, PARTITION_TYPE_STR[ctx->part], failedDev->GetName(), from, to, tryLockRetryCnt);
            int sleep = tryLockRetryCnt * 10;
            usleep(sleep);
        }

        tryLockRetryCnt++;
        if (tryLockRetryCnt >= TRY_LOCK_MAX_RETRY)
        {
            locker->WriteBusyLog(failedDev);
            POS_TRACE_ERROR(EID(REBUILD_TRY_LOCK_FAILED),
                "array_name:{}, part:{}, dev_name:{}, stripe_from:{}, stripe_to:{}, retried:{}",
                ctx->array, PARTITION_TYPE_STR[ctx->part], failedDev->GetName(), from, to, tryLockRetryCnt);
            ctx->SetResult(RebuildState::FAIL);
            EventSmartPtr complete(new RebuildCompleted(this));
            complete->SetEventType(BackendEvent_MetadataRebuild);
            EventSchedulerSingleton::Instance()->EnqueueEvent(complete);
            baseStripe = 0;
            return EID(REBUILD_TRY_LOCK_FAILED);
        }
        return EID(REBUILD_TRY_LOCK_RETRY);
    }
    tryLockRetryCnt = 0;
    return 0;
}


} // namespace pos

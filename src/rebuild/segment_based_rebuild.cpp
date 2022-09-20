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

#include "segment_based_rebuild.h"
#include "rebuilder.h"
#include "rebuild_completed.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/include/branch_prediction.h"
#include "src/include/backend_event.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/include/address_type.h"

namespace pos
{
SegmentBasedRebuild::SegmentBasedRebuild(unique_ptr<RebuildContext> c, IContextManager* allocatorSvc)
: RebuildBehavior(move(c)),
  allocatorSvc(allocatorSvc)
{
    POS_TRACE_DEBUG(EID(STRIPES_REBUILD_INIT), "SegmentBasedRebuild");
}

SegmentBasedRebuild::~SegmentBasedRebuild(void)
{
}

bool
SegmentBasedRebuild::Rebuild(void)
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

void SegmentBasedRebuild::UpdateProgress(uint32_t val)
{
    // the val is always 0, but the progress is increased due to reduced remaining.
    uint32_t remainingStripe =
        allocatorSvc->GetRebuildTargetSegmentCount() * ctx->size->stripesPerSegment;
    ctx->prog->Update(PARTITION_TYPE_STR[ctx->part], val, remainingStripe);
}

bool
SegmentBasedRebuild::_Init(void)
{
    if (isInitialized == false)
    {
        uint32_t idx = 1;
        for (RebuildMethod* rm : ctx->rm)
        {
            bool ret = rm->Init("SegmentBasedRebuild_" + ctx->array + "_" + PARTITION_TYPE_STR[ctx->part] + "_" + to_string(idx));
            if (ret == false)
            {
                if (initRetryCnt >= INIT_REBUILD_MAX_RETRY)
                {
                    POS_TRACE_ERROR(EID(REBUILD_BUFFER_INIT_FAIL), "part_type:{}, retried:{}",
                        PARTITION_TYPE_STR[ctx->part], initRetryCnt);
                    ctx->SetResult(RebuildState::FAIL);
                    _Finish(RebuildState::FAIL);
                    return true;
                }
                int logInterval = INIT_REBUILD_MAX_RETRY / 10;
                if (initRetryCnt % logInterval == 0)
                {
                    POS_TRACE_WARN(EID(REBUILD_BUFFER_INIT_RETRY), "Failed to initialize buffers for segment based rebuild, array:{}, part:{}, retried:{}",
                        ctx->array, PARTITION_TYPE_STR[ctx->part], initRetryCnt);
                }
                initRetryCnt++;
                return false;
            }
            idx++;
        }
        POS_TRACE_INFO(EID(REBUILD_BUFFER_INIT_OK), "part:{}, rmCnt:{}",
            PARTITION_TYPE_STR[ctx->part], ctx->rm.size());
        isInitialized = true;
        initRetryCnt = 0;
    }
    return true;
}

bool
SegmentBasedRebuild::_Recover(void)
{
    SegmentId segId = _GetNextSegment();
    if (segId == NEED_TO_RETRY)
    {
        return false;
    }
    UpdateProgress(0);

    RebuildState state = ctx->GetResult();
    if (segId == ctx->size->totalSegments ||
        state >= RebuildState::CANCELLED)
    {
        _Finish(state);
        return true;
    }

    uint32_t strCnt = ctx->size->stripesPerSegment;
    uint32_t callbackCnt = ctx->rm.size();
    ctx->taskCnt = strCnt * callbackCnt;
    StripeId baseStripe = segId * strCnt;
    POS_TRACE_INFO(EID(STRIPES_REBUILD_BEGIN),
        "segID:{}, from:{}, taskCnt:{}", segId, baseStripe, ctx->taskCnt);
    for (uint32_t offset = 0; offset < strCnt; offset++)
    {
        StripeId stripeId = baseStripe + offset;
        for (RebuildMethod* rm : ctx->rm)
        {
            StripeRebuildDoneCallback callback = bind(&SegmentBasedRebuild::_RecoverCompleted, this, segId, stripeId, placeholders::_1);
            int ret = rm->Recover(ctx->arrayIndex, stripeId, ctx->size, callback);
            if (ret != 0)
            {
                POS_TRACE_ERROR(ret,
                    "stripeId:{}, part:{}",
                    stripeId, PARTITION_TYPE_STR[ctx->part]);
                ctx->SetResult(RebuildState::FAIL);
                _Finish(RebuildState::FAIL);
                return true;
            }
        }
    }
    return true;
}

void SegmentBasedRebuild::_RecoverCompleted(SegmentId segmentId, StripeId stripeId, int result)
{
    if (result != 0)
    {
        ctx->SetResult(RebuildState::FAIL);
        POS_TRACE_ERROR(result,
            "segID:{}, stripeID:{}, result:{}", segmentId, stripeId, result);
    }
    uint32_t currentTaskCnt = ctx->taskCnt -= 1;
    if (currentTaskCnt == 0)
    {
        POS_TRACE_INFO(EID(STRIPES_REBUILD_DONE),
            "segID:{}, result:{}", segmentId, (int)(ctx->GetResult()));
        allocatorSvc->ReleaseRebuildSegment(segmentId);
        EventSmartPtr nextEvent(new Rebuilder(this));
        nextEvent->SetEventType(BackendEvent_UserdataRebuild);
        EventSchedulerSingleton::Instance()->EnqueueEvent(nextEvent);
    }
}

void
SegmentBasedRebuild::_Finish(RebuildState state)
{
    POS_TRACE_DEBUG(EID(STRIPES_REBUILD_FINISH), "part:{}", PARTITION_TYPE_STR[ctx->part]);
    if (state == RebuildState::CANCELLED)
    {
        POS_TRACE_WARN(EID(REBUILD_RESULT_CANCELLED),
            "Partition {} ({}) rebuilding stopped",
            PARTITION_TYPE_STR[ctx->part]);
    }
    else if (state == RebuildState::FAIL)
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
    }

    EventSmartPtr complete(new RebuildCompleted(this));
    complete->SetEventType(BackendEvent_UserdataRebuild);
    EventSchedulerSingleton::Instance()->EnqueueEvent(complete);
}

SegmentId
SegmentBasedRebuild::_GetNextSegment(void)
{
    SegmentId segId = allocatorSvc->AllocateRebuildTargetSegment();
    if (segId == UINT32_MAX)
    {
        return ctx->size->totalSegments;
    }
    ctx->logger->rebuiltSegCnt++;
    return segId;
}

} // namespace pos

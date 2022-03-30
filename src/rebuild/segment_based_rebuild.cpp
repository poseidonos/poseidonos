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

#include <typeinfo>

#include "rebuilder.h"
#include "rebuild_completed.h"
#include "update_data_handler.h"
#include "update_data_complete_handler.h"
#include "src/include/array_config.h"
#include "src/include/pos_event_id.h"
#include "src/include/branch_prediction.h"
#include "src/include/backend_event.h"
#include "src/array_models/dto/partition_physical_size.h"
#include "src/bio/ubio.h"
#include "src/logger/logger.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/event_scheduler/io_completer.h"
#include "src/io_scheduler/io_dispatcher.h"
#include "src/io/backend_io/rebuild_io/rebuild_read.h"
#include "src/resource_manager/buffer_pool.h"

#include "Air.h"

namespace pos
{
SegmentBasedRebuild::SegmentBasedRebuild(unique_ptr<RebuildContext> c, IContextManager* allocatorSvc)
: RebuildBehavior(move(c)),
  allocatorSvc(allocatorSvc)
{
    POS_TRACE_DEBUG(POS_EVENT_ID::REBUILD_DEBUG_MSG, "SegmentBasedRebuild");
    bool ret = _InitBuffers();
    assert(ret);
}

SegmentBasedRebuild::~SegmentBasedRebuild(void)
{
    POS_TRACE_DEBUG(POS_EVENT_ID::REBUILD_DEBUG_MSG, "~SegmentBasedRebuild");
}

string
SegmentBasedRebuild::_GetClassName(void)
{
    return typeid(this).name();
}

int
SegmentBasedRebuild::_GetTotalReadChunksForRecovery(void)
{
    return ctx->size->chunksPerStripe - 1; // Except faulty device
}

SegmentId
SegmentBasedRebuild::_NextSegment(void)
{
    SegmentId segId = allocatorSvc->AllocateRebuildTargetSegment();

    POS_TRACE_INFO(EID(REBUILD_DEBUG_MSG),
                "SegmentBasedRebuild::_NextSegment is {}",
                segId);
    if (segId == UINT32_MAX)
    {
        return ctx->size->totalSegments;
    }
    ctx->logger->rebuiltSegCnt++;
    return segId;
}

bool
SegmentBasedRebuild::Read(void)
{
    uint32_t strCnt = ctx->size->stripesPerSegment;
    uint32_t blkCnt = ctx->size->blksPerChunk;
    uint64_t key = (((uint64_t)strCnt) << 32) + blkCnt;
    airlog("LAT_SegmentBasedRebuildRead", "AIR_BEGIN", 0, key);

    SegmentId segId = _NextSegment();
    if (segId == NEED_TO_RETRY)
    {
        airlog("LAT_SegmentBasedRebuildRead", "AIR_END", 0, key);
        return false;
    }
    UpdateProgress(0);

    RebuildState state = ctx->GetResult();
    if (segId == ctx->size->totalSegments ||
        state >= RebuildState::CANCELLED)
    {
        if (state == RebuildState::CANCELLED)
        {
            POS_TRACE_WARN((int)POS_EVENT_ID::REBUILD_STOPPED,
                "Partition {} ({}) rebuilding stopped",
                PARTITION_TYPE_STR[ctx->part], ctx->raidType.ToString());
        }
        else if (state == RebuildState::FAIL)
        {
            POS_TRACE_WARN((int)POS_EVENT_ID::REBUILD_FAILED,
                "Partition {} ({}) rebuilding failed",
                PARTITION_TYPE_STR[ctx->part], ctx->raidType.ToString());
        }
        else
        {
            POS_TRACE_DEBUG(EID(REBUILD_DEBUG_MSG),
                "Partition {} ({}) rebuilding done",
                PARTITION_TYPE_STR[ctx->part], ctx->raidType.ToString());
            ctx->SetResult(RebuildState::PASS);
        }

        EventSmartPtr complete(new RebuildCompleted(this));
        complete->SetEventType(BackendEvent_UserdataRebuild);
        EventSchedulerSingleton::Instance()->EnqueueEvent(complete);

        airlog("LAT_SegmentBasedRebuildRead", "AIR_END", 0, key);
        return true;
    }

    ctx->taskCnt = strCnt;
    StripeId baseStripe = segId * strCnt;
    POS_TRACE_DEBUG(EID(REBUILD_DEBUG_MSG),
        "SegmentBasedRebuild - segID:{}, from:{}, cnt:{}", segId, baseStripe, strCnt);
    for (uint32_t offset = 0; offset < strCnt; offset++)
    {
        StripeId stripeId = baseStripe + offset;
        void* buffer = recoverBuffers->TryGetBuffer();
        assert(buffer != nullptr);

        UbioSmartPtr ubio(new Ubio(buffer, blkCnt * Ubio::UNITS_PER_BLOCK, ctx->arrayIndex));
        ubio->dir = UbioDir::Write;
        FtBlkAddr fta = {.stripeId = stripeId,
            .offset = ctx->faultIdx * blkCnt};
        PhysicalBlkAddr addr = ctx->translate(fta);
        ubio->SetPba(addr);
        CallbackSmartPtr callback(new UpdateDataHandler(segId, ubio, this));
        callback->SetEventType(BackendEvent_UserdataRebuild);
        ubio->SetEventType(BackendEvent_UserdataRebuild);
        ubio->SetCallback(callback);
        RebuildRead rebuildRead;
        int res = rebuildRead.Recover(ubio, rebuildReadBuffers);
        if (res != 0)
        {
            POS_TRACE_ERROR((int)POS_EVENT_ID::REBUILD_FAILED,
                "Failed to recover stripe {} in Partition {} ({})",
                stripeId, PARTITION_TYPE_STR[ctx->part], ctx->raidType.ToString());
            ctx->SetResult(RebuildState::FAIL);
        }
    }

    airlog("LAT_SegmentBasedRebuildRead", "AIR_END", 0, key);
    return true;
}

bool SegmentBasedRebuild::Write(uint32_t targetId, UbioSmartPtr ubio)
{
    uint64_t objAddr = reinterpret_cast<uint64_t>(ubio.get());
    airlog("LAT_SegmentBasedRebuildWrite", "AIR_BEGIN", 0, objAddr);

    CallbackSmartPtr event(
        new UpdateDataCompleteHandler(targetId, ubio, this));
    event->SetEventType(BackendEvent_UserdataRebuild);
    IODispatcher* ioDisp = IODispatcherSingleton::Instance();

    ubio->ClearCallback();
    ubio->SetCallback(event);

    if (likely(ctx->GetResult() == RebuildState::REBUILDING))
    {
        ioDisp->Submit(ubio);
    }
    else
    {
        IoCompleter ioCompleter(ubio);
        ioCompleter.CompleteUbio(IOErrorType::GENERIC_ERROR, true);
    }

    airlog("LAT_SegmentBasedRebuildWrite", "AIR_END", 0, objAddr);
    ubio = nullptr;
    return true;
}

bool SegmentBasedRebuild::Complete(uint32_t targetId, UbioSmartPtr ubio)
{
    uint32_t currentTaskCnt = ctx->taskCnt -= 1;

    if (currentTaskCnt == 0)
    {
        allocatorSvc->ReleaseRebuildSegment(targetId);
        EventSmartPtr nextEvent(new Rebuilder(this));
        nextEvent->SetEventType(BackendEvent_UserdataRebuild);
        EventSchedulerSingleton::Instance()->EnqueueEvent(nextEvent);
    }
    recoverBuffers->ReturnBuffer(ubio->GetBuffer());
    ubio = nullptr;

    return true;
}

void SegmentBasedRebuild::UpdateProgress(uint32_t val)
{
    uint32_t remainingStripe =
        allocatorSvc->GetRebuildTargetSegmentCount() * ctx->size->stripesPerSegment;
    POS_TRACE_DEBUG(POS_EVENT_ID::REBUILD_DEBUG_MSG,
        "SegmentBasedRebuild::UpdateProgress, reamining:{}", remainingStripe);
    ctx->prog->Update(PARTITION_TYPE_STR[ctx->part], val, remainingStripe);
}

} // namespace pos

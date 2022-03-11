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
#include "src/array/service/array_service_layer.h"
#include "src/bio/ubio.h"
#include "src/logger/logger.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/event_scheduler/io_completer.h"
#include "src/io_scheduler/io_dispatcher.h"
#include "src/io/backend_io/rebuild_io/rebuild_read.h"
#include "src/resource_manager/buffer_pool.h"

namespace pos
{
StripeBasedRaceRebuild::StripeBasedRaceRebuild(unique_ptr<RebuildContext> c)
: RebuildBehavior(move(c))
{
    POS_TRACE_DEBUG(POS_EVENT_ID::REBUILD_DEBUG_MSG, "StripeBasedRaceRebuild");
    locker = IOLockerSingleton::Instance();
    assert(locker != nullptr);

    bool ret = _InitBuffers();
    assert(ret);
}

StripeBasedRaceRebuild::~StripeBasedRaceRebuild(void)
{
    POS_TRACE_DEBUG(POS_EVENT_ID::REBUILD_DEBUG_MSG, "~StripeBasedRaceRebuild");
}

string
StripeBasedRaceRebuild::_GetClassName(void)
{
    return typeid(this).name();
}

int
StripeBasedRaceRebuild::_GetTotalReadChunksForRecovery(void)
{
    return 1; // Mirror
}

bool
StripeBasedRaceRebuild::Read(void)
{
    POS_TRACE_DEBUG(POS_EVENT_ID::REBUILD_DEBUG_MSG, "StripeBasedRaceRebuild::Rebuild");
    uint32_t strPerSeg = ctx->size->stripesPerSegment;
    uint32_t blkCnt = ctx->size->blksPerChunk;
    uint32_t maxStripeId = ctx->size->totalSegments * strPerSeg - 1;

    UpdateProgress(baseStripe);

    if (baseStripe >= maxStripeId ||
        ctx->GetResult() >= RebuildState::CANCELLED)
    {
        if (locker->ResetBusyLock(ctx->faultDev) == false)
        {
            POS_TRACE_DEBUG(EID(REBUILD_DEBUG_MSG),
                "Partition {} rebuild done, but waiting lock release",
                ctx->part);
            return false;
        }
        if (ctx->GetResult() == RebuildState::CANCELLED)
        {
            POS_TRACE_WARN((int)POS_EVENT_ID::REBUILD_STOPPED,
                "Partition {} (RAID1) rebuilding stopped",
                ctx->part);
        }
        else if (ctx->GetResult() == RebuildState::FAIL)
        {
            POS_TRACE_WARN((int)POS_EVENT_ID::REBUILD_FAILED,
                "Partition {} (RAID1) rebuilding failed",
                ctx->part);
        }
        else
        {
            POS_TRACE_DEBUG(EID(REBUILD_DEBUG_MSG),
                "Partition {} (RAID1) rebuilding done",
                ctx->part);
            ctx->SetResult(RebuildState::PASS);
            UpdateProgress(ctx->stripeCnt);
        }

        EventSmartPtr complete(new RebuildCompleted(this));
        complete->SetEventType(BackendEvent_MetadataRebuild);
        EventSchedulerSingleton::Instance()->EnqueueEvent(complete);
        baseStripe = 0;
        return true;
    }

    uint32_t from = baseStripe;
    uint32_t to = baseStripe + strPerSeg - 1;
    if (to > maxStripeId)
    {
        to = maxStripeId;
    }

    uint32_t currWorkload = to - from + 1;

    if (locker->TryBusyLock(ctx->faultDev, from, to) == false)
    {
        return false;
    }

    ctx->taskCnt = currWorkload;
    POS_TRACE_DEBUG(EID(REBUILD_DEBUG_MSG),
        "StripeBasedRaceRebuild - from:{}, to:{}", from, to);
    for (uint32_t offset = 0; offset < currWorkload; offset++)
    {
        uint32_t stripeId = baseStripe + offset;
        void* buffer = recoverBuffers->TryGetBuffer();
        assert(buffer != nullptr);


        UbioSmartPtr ubio(new Ubio(buffer, blkCnt * Ubio::UNITS_PER_BLOCK, ctx->arrayIndex));
        ubio->dir = UbioDir::Write;
        FtBlkAddr fta = {.stripeId = stripeId,
            .offset = ctx->faultIdx * blkCnt};
        PhysicalBlkAddr addr = ctx->translate(fta);
        ubio->SetPba(addr);
        CallbackSmartPtr callback(new UpdateDataHandler(stripeId, ubio, this));
        callback->SetEventType(BackendEvent_MetadataRebuild);
        ubio->SetEventType(BackendEvent_MetadataRebuild);
        ubio->SetCallback(callback);
        RebuildRead rebuildRead;
        int res = rebuildRead.Recover(ubio, rebuildReadBuffers);
        if (res != 0)
        {
            POS_TRACE_ERROR((int)POS_EVENT_ID::REBUILD_FAILED,
                "Failed to recover stripe {} in Partition {} (RAID1), maxStripes:{}",
                stripeId, ctx->part, maxStripeId);
            ctx->SetResult(RebuildState::FAIL);
        }
    }

    baseStripe += currWorkload;
    return true;
}

bool StripeBasedRaceRebuild::Write(uint32_t targetId, UbioSmartPtr ubio)
{
    POS_TRACE_DEBUG(2831, "StripeBasedRaceRebuild::Write, target stripe:{}", targetId);
    CallbackSmartPtr event(
        new UpdateDataCompleteHandler(targetId, ubio, this));
    event->SetEventType(BackendEvent_MetadataRebuild);
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

    ubio = nullptr;
    return true;
}

bool StripeBasedRaceRebuild::Complete(uint32_t targetId, UbioSmartPtr ubio)
{
    locker->Unlock(ctx->faultDev, targetId);

    uint32_t currentTaskCnt = ctx->taskCnt -= 1;

    if (currentTaskCnt == 0)
    {
        EventSmartPtr nextEvent(new Rebuilder(this));
        nextEvent->SetEventType(BackendEvent_MetadataRebuild);
        EventSchedulerSingleton::Instance()->EnqueueEvent(nextEvent);
    }
    recoverBuffers->ReturnBuffer(ubio->GetBuffer());
    ubio = nullptr;

    return true;
}

void StripeBasedRaceRebuild::UpdateProgress(uint32_t val)
{
    ctx->prog->Update(ctx->part, val, ctx->stripeCnt);
}

} // namespace pos

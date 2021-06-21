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
#include "raid1_rebuild.h"

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
Raid1Rebuild::Raid1Rebuild(unique_ptr<RebuildContext> c)
: RebuildBehavior(move(c))
{
    POS_TRACE_DEBUG(POS_EVENT_ID::REBUILD_DEBUG_MSG, "Raid1Rebuild");
    locker = ArrayService::Instance()->Getter()->GetLocker();
    assert(locker != nullptr);

    bool ret = _InitBuffers();
    assert(ret);
}

Raid1Rebuild::~Raid1Rebuild(void)
{
    POS_TRACE_DEBUG(POS_EVENT_ID::REBUILD_DEBUG_MSG, "~Raid1Rebuild");
}

string
Raid1Rebuild::_GetClassName(void)
{
    return typeid(this).name();
}

int
Raid1Rebuild::_GetTotalReadChunksForRecovery(void)
{
    return 1; // Mirror
}


bool
Raid1Rebuild::Read(void)
{
    POS_TRACE_DEBUG(POS_EVENT_ID::REBUILD_DEBUG_MSG, "Raid1Rebuild::Rebuild");
    uint32_t strPerSeg = ctx->size->stripesPerSegment;
    uint32_t blkCnt = ctx->size->blksPerChunk;
    uint32_t maxStripeId = ctx->size->totalSegments * strPerSeg - 1;

    UpdateProgress(baseStripe);

    if (baseStripe >= maxStripeId ||
        ctx->result >= RebuildState::CANCELLED)
    {
        bool ret = locker->TryChange(
            ctx->array, LockerMode::NORMAL);
        if (ret == false)
        {
            POS_TRACE_DEBUG((int)POS_EVENT_ID::REBUILD_DEBUG_MSG,
                "Partition {} rebuild done, but waiting lock release",
                ctx->part);
            return false;
        }
        if (ctx->result == RebuildState::CANCELLED)
        {
            POS_TRACE_WARN((int)POS_EVENT_ID::REBUILD_STOPPED,
                "Partition {} (RAID1) rebuilding stopped",
                ctx->part);
        }
        else if (ctx->result == RebuildState::FAIL)
        {
            POS_TRACE_WARN((int)POS_EVENT_ID::REBUILD_FAILED,
                "Partition {} (RAID1) rebuilding failed",
                ctx->part);
        }
        else
        {
            POS_TRACE_DEBUG((int)POS_EVENT_ID::REBUILD_DEBUG_MSG,
                "Partition {} (RAID1) rebuilding done",
                ctx->part);
            ctx->result = RebuildState::PASS;
        }

        EventSmartPtr complete(new RebuildCompleted(this));
        complete->SetEventType(BackendEvent_MetadataRebuild);
        EventSchedulerSingleton::Instance()->EnqueueEvent(complete);
        baseStripe = 0;
        return true;
    }

    if (baseStripe == 0)
    {
        bool ret =  locker->TryChange(
            ctx->array, LockerMode::BUSY);
        if (ret == false)
        {
            return false;
        }
    }

    uint32_t taskCnt = 0;
    uint32_t currWorkload = strPerSeg;
    if (baseStripe + currWorkload > maxStripeId)
    {
        currWorkload = maxStripeId - baseStripe;
    }

    for (uint32_t offset = 0; offset < currWorkload; offset++)
    {
        uint32_t stripeId = baseStripe + offset;
        if (locker->TryLock(ctx->array, stripeId) == false)
        {
            break;
        }
        taskCnt++;
    }

    if (taskCnt == 0)
    {
        return false;
    }

    ctx->taskCnt = taskCnt;
    POS_TRACE_DEBUG((int)POS_EVENT_ID::REBUILD_DEBUG_MSG,
        "Raid1Rebuild - from:{}, cnt:{}", baseStripe, taskCnt);
    for (uint32_t offset = 0; offset < taskCnt; offset++)
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
            ctx->result = RebuildState::FAIL;
        }
    }

    baseStripe += taskCnt;
    return true;
}

bool Raid1Rebuild::Write(uint32_t targetId, UbioSmartPtr ubio)
{
    POS_TRACE_DEBUG(2831, "Raid1Rebuild::Write, target stripe:{}", targetId);
    CallbackSmartPtr event(
        new UpdateDataCompleteHandler(targetId, ubio, this));
    event->SetEventType(BackendEvent_MetadataRebuild);
    IODispatcher* ioDisp = IODispatcherSingleton::Instance();

    ubio->ClearCallback();
    ubio->SetCallback(event);

    if (likely(ctx->result != RebuildState::FAIL))
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

bool Raid1Rebuild::Complete(uint32_t targetId, UbioSmartPtr ubio)
{
    locker->Unlock(ctx->array, targetId);

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

} // namespace pos

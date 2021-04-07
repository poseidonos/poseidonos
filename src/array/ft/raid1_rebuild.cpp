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

#include "src/array/config/array_config.h"
#include "src/array/ft/stripe_locker.h"
#include "src/array/partition/partition_size_info.h"
#include "src/include/ibof_event_id.h"
#include "src/io/general_io/ubio.h"
#include "src/logger/logger.h"
#include "src/scheduler/event_argument.h"
namespace ibofos
{
static StripeId rebuild_stripe = 0;

Raid1Rebuild::Raid1Rebuild(unique_ptr<RebuildContext> ctx)
: RebuildBehavior(move(ctx))
{
    IBOF_TRACE_DEBUG(IBOF_EVENT_ID::REBUILD_DEBUG_MSG, "Raid1Rebuild");
}

Raid1Rebuild::~Raid1Rebuild()
{
    IBOF_TRACE_DEBUG(IBOF_EVENT_ID::REBUILD_DEBUG_MSG, "~Raid1Rebuild");
};

bool
Raid1Rebuild::Rebuild()
{
    uint32_t strCnt = ctx->size->stripesPerSegment;
    uint32_t blkCnt = ctx->size->blksPerChunk;
    uint32_t totalStrCnt = ctx->size->totalSegments * strCnt;

    StripeId baseStripe = rebuild_stripe;
    UpdateProgress(baseStripe);

    if (baseStripe >= totalStrCnt ||
        ctx->result >= RebuildState::CANCELLED)
    {
        bool ret = ctx->locker->TryModeChanging(StripeLockerMode::NORMAL);
        if (ret == false)
        {
            IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::REBUILD_DEBUG_MSG,
                "Partition {} rebuild done, but waiting lock release",
                ctx->id);
            return false;
        }

        if (ctx->result == RebuildState::CANCELLED)
        {
            IBOF_TRACE_WARN((int)IBOF_EVENT_ID::REBUILD_STOPPED,
                "Partition {} (RAID1) rebuilding stopped",
                ctx->id);
        }
        else if (ctx->result == RebuildState::FAIL)
        {
            IBOF_TRACE_WARN((int)IBOF_EVENT_ID::REBUILD_FAILED,
                "Partition {} (RAID1) rebuilding failed",
                ctx->id);
        }
        else
        {
            IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::REBUILD_DEBUG_MSG,
                "Partition {} (RAID1) rebuilding done",
                ctx->id);
            ctx->result = RebuildState::PASS;
        }
        rebuild_stripe = 0;
        EventSmartPtr complete(new RebuildComplete(this));
#if defined QOS_ENABLED_BE
        complete->SetEventType(BackendEvent_MetadataRebuild);
#endif
        EventArgument::GetEventScheduler()->EnqueueEvent(complete);
        return true;
    }

    if (baseStripe == 0)
    {
        bool ret = ctx->locker->TryModeChanging(StripeLockerMode::BUSY);
        if (ret == false)
        {
            return false;
        }
    }

    uint32_t taskCnt = 0;

    for (uint32_t offset = 0; offset < strCnt; offset++)
    {
        uint32_t stripeId = baseStripe + offset;
        if (ctx->locker->TryLock(stripeId) == false)
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
    IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::REBUILD_DEBUG_MSG,
        "Raid1Rebuild - from:{}, cnt:{}", baseStripe, taskCnt);
    for (uint32_t offset = 0; offset < taskCnt; offset++)
    {
        uint32_t stripeId = baseStripe + offset;

        void* buffer = Memory<ArrayConfig::BLOCK_SIZE_BYTE>::Alloc(blkCnt);
        UbioSmartPtr ubio(new Ubio(buffer, blkCnt * Ubio::UNITS_PER_BLOCK));
        ubio->dir = UbioDir::Write;
        FtBlkAddr fta = {.stripeId = stripeId,
            .offset = ctx->faultIdx * blkCnt};
        PhysicalBlkAddr addr = ctx->translate(fta);
        ubio->SetPba(addr);
        CallbackSmartPtr callback(new UpdateDataHandler(stripeId, ubio, this));
#if defined QOS_ENABLED_BE
        callback->SetEventType(BackendEvent_MetadataRebuild);
        ubio->SetEventType(BackendEvent_MetadataRebuild);
#endif
        ubio->SetCallback(callback);
        int res = ctx->restore(ubio);
        if (res != 0)
        {
            IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::REBUILD_FAILED,
                "Failed to recover stripe {} in Partition {} (RAID1)",
                stripeId, ctx->id);
            ctx->result = RebuildState::FAIL;
        }
    }

    rebuild_stripe += taskCnt;
    return true;
}

} // namespace ibofos

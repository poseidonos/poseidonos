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

#include "raid5_rebuild.h"

#include "src/allocator/allocator.h"
#include "src/array/config/array_config.h"
#include "src/array/partition/partition_size_info.h"
#include "src/include/ibof_event_id.h"
#include "src/io/general_io/ubio.h"
#include "src/logger/logger.h"
#include "src/scheduler/event_argument.h"

namespace ibofos
{
Raid5Rebuild::Raid5Rebuild(unique_ptr<RebuildContext> ctx)
: RebuildBehavior(move(ctx))
{
    IBOF_TRACE_DEBUG(IBOF_EVENT_ID::REBUILD_DEBUG_MSG, "Raid5Rebuild");
}

Raid5Rebuild::~Raid5Rebuild()
{
    IBOF_TRACE_DEBUG(IBOF_EVENT_ID::REBUILD_DEBUG_MSG, "~Raid5Rebuild");
};

SegmentId
Raid5Rebuild::_NextSegment()
{
    SegmentId segId = AllocatorSingleton::Instance()->GetRebuildTargetSegment();

    if (segId == UINT32_MAX)
    {
        return ctx->size->totalSegments;
    }
    return segId;
}

bool
Raid5Rebuild::Rebuild()
{
    uint32_t strCnt = ctx->size->stripesPerSegment;
    uint32_t blkCnt = ctx->size->blksPerChunk;

    SegmentId segId = _NextSegment();
    UpdateProgress(segId * strCnt);

    if (segId == ctx->size->totalSegments ||
        ctx->result >= RebuildState::CANCELLED)
    {
        if (ctx->result == RebuildState::CANCELLED)
        {
            IBOF_TRACE_WARN((int)IBOF_EVENT_ID::REBUILD_STOPPED,
                "Partition {} (RAID5) rebuilding stopped",
                ctx->id);
        }
        else if (ctx->result == RebuildState::FAIL)
        {
            IBOF_TRACE_WARN((int)IBOF_EVENT_ID::REBUILD_FAILED,
                "Partition {} (RAID5) rebuilding failed",
                ctx->id);
        }
        else
        {
            IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::REBUILD_DEBUG_MSG,
                "Partition {} (RAID5) rebuilding done",
                ctx->id);
            ctx->result = RebuildState::PASS;
        }

        EventSmartPtr complete(new RebuildComplete(this));
#if defined QOS_ENABLED_BE
        complete->SetEventType(BackendEvent_UserdataRebuild);
#endif
        EventArgument::GetEventScheduler()->EnqueueEvent(complete);

        return true;
    }

    ctx->taskCnt = strCnt;
    StripeId baseStripe = segId * strCnt;
    IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::REBUILD_DEBUG_MSG,
        "Raid5Rebuild - from:{}, cnt:{}", baseStripe, strCnt);
    for (uint32_t offset = 0; offset < strCnt; offset++)
    {
        StripeId stripeId = baseStripe + offset;
        void* buffer = Memory<ArrayConfig::BLOCK_SIZE_BYTE>::Alloc(blkCnt);
        UbioSmartPtr ubio(new Ubio(buffer, blkCnt * Ubio::UNITS_PER_BLOCK));
        ubio->dir = UbioDir::Write;
        FtBlkAddr fta = {.stripeId = stripeId,
            .offset = ctx->faultIdx * blkCnt};
        PhysicalBlkAddr addr = ctx->translate(fta);
        ubio->SetPba(addr);
        CallbackSmartPtr callback(new UpdateDataHandler(segId, ubio, this));
#if defined QOS_ENABLED_BE
        callback->SetEventType(BackendEvent_UserdataRebuild);
        ubio->SetEventType(BackendEvent_UserdataRebuild);
#endif
        ubio->SetCallback(callback);
        int res = ctx->restore(ubio);
        if (res != 0)
        {
            IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::REBUILD_FAILED,
                "Failed to recover stripe {} in Partition {} (RAID5)",
                stripeId, ctx->id);
            ctx->result = RebuildState::FAIL;
        }
    }

    return true;
}

} // namespace ibofos

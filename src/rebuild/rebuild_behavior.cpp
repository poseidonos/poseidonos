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

#include "rebuild_behavior.h"

#include "src/array_models/dto/partition_physical_size.h"
#include "src/include/array_config.h"
#include "src/resource_manager/buffer_pool.h"
#include "src/resource_manager/memory_manager.h"
#include "src/logger/logger.h"

using namespace pos;

RebuildBehavior::RebuildBehavior(unique_ptr<RebuildContext> ctx,
    MemoryManager* mm)
: ctx(move(ctx)),
  mm(mm)
{
}
// LCOV_EXCL_START
RebuildBehavior::~RebuildBehavior(void)
{
    if (recoverBuffers != nullptr)
    {
        if (recoverBuffers->IsFull() == false)
        {
            POS_TRACE_ERROR(EID(REBUILD_DEBUG_MSG),
                "Some buffers in recoverBuffers were not returned but deleted.");
        }
        mm->DeleteBufferPool(recoverBuffers);
        recoverBuffers = nullptr;
    }
    if (rebuildReadBuffers != nullptr)
    {
        if (rebuildReadBuffers->IsFull() == false)
        {
            POS_TRACE_ERROR(EID(REBUILD_DEBUG_MSG),
                "Some buffers in rebuildReadBuffers were not returned but deleted.");
        }
        mm->DeleteBufferPool(rebuildReadBuffers);
        rebuildReadBuffers = nullptr;
    }
}
// LCOV_EXCL_STOP
void
RebuildBehavior::StopRebuilding(void)
{
    ctx->SetResult(RebuildState::CANCELLED);
}

RebuildContext*
RebuildBehavior::GetContext(void)
{
    return ctx.get();
}

bool
RebuildBehavior::_InitRecoverBuffers(string owner)
{
    BufferInfo info = {
        .owner = owner,
        .size = ctx->size->blksPerChunk * ArrayConfig::BLOCK_SIZE_BYTE,
        .count = ctx->size->stripesPerSegment};
    recoverBuffers = mm->CreateBufferPool(info);
    if (recoverBuffers == nullptr)
    {
        return false;
    }
    return true;
}

bool
RebuildBehavior::_InitRebuildReadBuffers(string owner, int totalChunksToRead)
{
    BufferInfo info = {
        .owner = owner,
        .size = ctx->size->blksPerChunk * ArrayConfig::BLOCK_SIZE_BYTE * totalChunksToRead,
        .count = ctx->size->stripesPerSegment};
    rebuildReadBuffers = mm->CreateBufferPool(info);
    if (rebuildReadBuffers == nullptr)
    {
        return false;
    }
    return true;
}

bool
RebuildBehavior::_InitBuffers(void)
{
    string owner = _GetClassName() + "_" + ctx->array;
    bool ret = _InitRecoverBuffers(owner);
    if (ret == false)
    {
        POS_TRACE_WARN(EID(REBUILD_DEBUG_MSG), "Failed to alloc rebuild(recover) BufferPool, owner:{}, array:{}, part:{}",
            owner, ctx->array, PARTITION_TYPE_STR[ctx->part]);
        return ret;
    }
    else
    {
        POS_TRACE_DEBUG(EID(REBUILD_DEBUG_MSG), "BufferPool for rebuild(recover) allocation has been successful, owner:{}, array:{}, part:{}",
            owner, ctx->array, PARTITION_TYPE_STR[ctx->part]);
    }

    int totalChunks = _GetTotalReadChunksForRecovery();
    ret = _InitRebuildReadBuffers(owner, totalChunks);
    if (ret == false)
    {
        POS_TRACE_WARN(EID(REBUILD_DEBUG_MSG), "Failed to alloc rebuild(read) BufferPool, owner:{}, array:{}, part:{}",
            owner, ctx->array, PARTITION_TYPE_STR[ctx->part]);
        mm->DeleteBufferPool(recoverBuffers);
        recoverBuffers = nullptr;
    }
    else
    {
        POS_TRACE_DEBUG(EID(REBUILD_DEBUG_MSG), "BufferPool for rebuild(read) allocation has been successful, owner:{}, array:{}, part:{}",
            owner, ctx->array, PARTITION_TYPE_STR[ctx->part]);
    }

    return true;
}

int
RebuildBehavior::_GetTotalReadChunksForRecovery(void)
{
    if (ctx->raidType == RaidTypeEnum::RAID10)
    {
        return 1;
    }
    return ctx->size->chunksPerStripe - 1; // for RAID5
}

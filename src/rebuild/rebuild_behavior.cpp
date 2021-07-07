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

#include "rebuild_behavior.h"

#include "src/array_models/dto/partition_physical_size.h"
#include "src/include/array_config.h"
#include "src/resource_manager/buffer_pool.h"
#include "src/resource_manager/memory_manager.h"

using namespace pos;

RebuildBehavior::RebuildBehavior(unique_ptr<RebuildContext> ctx,
    MemoryManager* mm)
: ctx(move(ctx)),
  mm(mm)
{
}

RebuildBehavior::~RebuildBehavior(void)
{
    delete recoverBuffers;
    delete rebuildReadBuffers;
}

void
RebuildBehavior::StopRebuilding(void)
{
    ctx->result = RebuildState::CANCELLED;
}

void
RebuildBehavior::UpdateProgress(uint32_t val)
{
    ctx->prog->Update(ctx->part, val);
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
    string owner = _GetClassName();
    bool ret = _InitRecoverBuffers(owner);
    if (ret == false)
    {
        return ret;
    }

    int totalChunks = _GetTotalReadChunksForRecovery();
    ret = _InitRebuildReadBuffers(owner, totalChunks);
    return ret;
}

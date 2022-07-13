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
#include "src/rebuild/recovery_methods/quick_recovery.h"
#include "src/rebuild/recovery_methods/rebuild_recovery.h"
#include "src/array_models/dto/partition_physical_size.h"
#include "src/include/array_config.h"
#include "src/resource_manager/buffer_pool.h"
#include "src/resource_manager/memory_manager.h"
#include "src/logger/logger.h"

using namespace pos;

RebuildBehavior::RebuildBehavior(unique_ptr<RebuildContext> c,
    MemoryManager* mm)
: ctx(move(c)),
  memMgr(mm)
{
    if (ctx == nullptr)
    {
        POS_TRACE_WARN(EID(REBUILD_DEBUG_MSG), "Failed to initialize rebuild, ctx is null");
        return;
    }

    uint32_t bufCnt = ctx->size->stripesPerSegment;
    uint64_t chunkSize = ctx->size->blksPerChunk * ArrayConfig::BLOCK_SIZE_BYTE;
    uint64_t destCnt = 1;
    uint64_t srcCnt = destCnt;
    if (ctx->rebuildType == RebuildTypeEnum::QUICK)
    {
        uint64_t srcSize = chunkSize * srcCnt;
        uint64_t destSize = chunkSize * destCnt;
        recovery = new QuickRecovery(srcSize, destSize, bufCnt);
    }
    else
    {
        if (ctx->raidType != RaidTypeEnum::RAID10)
        {
            srcCnt = ctx->size->chunksPerStripe - destCnt;
        }
        uint64_t srcSize = chunkSize * srcCnt;
        uint64_t destSize = chunkSize * destCnt;
        recovery = new RebuildRecovery(srcSize, destSize, bufCnt);
    }
}
// LCOV_EXCL_START
RebuildBehavior::~RebuildBehavior(void)
{
    delete recovery;
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
RebuildBehavior::_InitBuffers(void)
{
    string owner = _GetClassName() + "_" + ctx->array;
    bool ret = recovery->Init(memMgr, owner);
    if (ret == false)
    {
        int logInterval = INIT_REBUILD_MAX_RETRY / 10;
        if (initRebuildRetryCnt % logInterval == 0)
        {
            POS_TRACE_WARN(EID(REBUILD_DEBUG_MSG), "Failed to initialize buffers for rebuild, owner:{}, array:{}, part:{}, retried:{}",
                owner, ctx->array, PARTITION_TYPE_STR[ctx->part], initRebuildRetryCnt);
        }
        initRebuildRetryCnt++;
        return ret;
    }

    POS_TRACE_DEBUG(EID(REBUILD_DEBUG_MSG), "Buffers for rebuild are initialized successfully, owner:{}, array:{}, part:{}",
        owner, ctx->array, PARTITION_TYPE_STR[ctx->part]);
    initRebuildRetryCnt = 0;
    return true;
}
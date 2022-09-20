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

#include "rebuild_behavior_factory.h"
#include "src/logger/logger.h"
#include "src/include/pos_event_id.h"
#include "src/include/partition_type.h"
#include "src/rebuild/rebuild_methods/n_to_m_rebuild.h"

namespace pos
{

RebuildBehaviorFactory::RebuildBehaviorFactory(IContextManager* allocator)
: allocatorSvc(allocator)
{
}

RebuildBehavior*
RebuildBehaviorFactory::CreateRebuildBehavior(unique_ptr<RebuildContext> ctx)
{
    vector<NToMRebuild*> rm;
    for (auto rp : ctx->rp)
    {
        rm.emplace_back(new NToMRebuild(rp->srcs, rp->dsts, rp->recovery));
    }

    RebuildPairs backupRp;
    ctx->GetSecondaryRebuildPairs(backupRp);
    if (backupRp.size() > 0)
    {
        POS_TRACE_INFO(EID(REBUILD_BEHAVIOR_CREATE), "Backup rebuild method found, count:{}, part:{}",
            backupRp.size(), PARTITION_TYPE_STR[ctx->part]);
        assert(backupRp.size() == rm.size());
        uint32_t index = 0;
        for (auto rp : backupRp)
        {
            NToMRebuild* backupRm = new NToMRebuild(rp->srcs, rp->dsts, rp->recovery);
            rm.at(index)->SetBackupMethod(backupRm);
            index++;
        }
    }
    else
    {
        POS_TRACE_INFO(EID(REBUILD_BEHAVIOR_CREATE), "No backup rebuild method, part:{}",
            PARTITION_TYPE_STR[ctx->part]);
    }
    for (auto item : rm)
    {
        ctx->rm.push_back(item);
    }

    if (ctx->part == PartitionType::JOURNAL_SSD)
    {
        POS_TRACE_INFO(EID(REBUILD_BEHAVIOR_CREATE), "StripeBasedRaceRebuild for {} partition is created",
            PARTITION_TYPE_STR[ctx->part]);
        return new StripeBasedRaceRebuild(move(ctx));
    }
    else if (ctx->part == PartitionType::META_SSD)
    {
        POS_TRACE_INFO(EID(REBUILD_BEHAVIOR_CREATE), "StripeBasedRaceRebuild for {} partition is created",
            PARTITION_TYPE_STR[ctx->part]);
        return new StripeBasedRaceRebuild(move(ctx));
    }
    else if (ctx->part == PartitionType::USER_DATA)
    {
        POS_TRACE_INFO(EID(REBUILD_BEHAVIOR_CREATE), "SegmentBasedRebuild for {} partition is created",
            PARTITION_TYPE_STR[ctx->part]);
        return new SegmentBasedRebuild(move(ctx), allocatorSvc);
    }

    POS_TRACE_ERROR(EID(REBUILD_BEHAVIOR_CREATE), "{} partition does not support rebuild", PARTITION_TYPE_STR[ctx->part]);
    return nullptr;
}
} // namespace pos

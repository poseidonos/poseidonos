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

#include "partition_rebuild.h"
#include "rebuilder.h"
#include "raid1_rebuild.h"
#include "raid5_rebuild.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
PartitionRebuild::PartitionRebuild(RebuildTarget* t, ArrayDevice* d,
    RebuildProgress* p, RebuildLogger* l)
: targetPart(t), targetDev(d)
{
    unique_ptr<RebuildContext> ctx = targetPart->GetRebuildCtx(targetDev);
    if (ctx)
    {
        bhvr = _GetRebuildBehavior(move(ctx));
        if (bhvr != nullptr)
        {
            bhvr->GetContext()->prog = p;
            bhvr->GetContext()->logger = l;
        }
    }
}

PartitionRebuild::~PartitionRebuild()
{
    delete bhvr;
}

void PartitionRebuild::Start(RebuildComplete cb)
{
    POS_TRACE_INFO((int)POS_EVENT_ID::REBUILD_DEBUG_MSG,
        "PartitionRebuild::Start");
    completeCb = cb;
    if (bhvr != nullptr)
    {
        bhvr->GetContext()->logger->SetPartitionRebuildStart(bhvr->GetContext()->part);
        bhvr->GetContext()->rebuildComplete =
            bind(&PartitionRebuild::_Complete, this, placeholders::_1);
        EventSmartPtr rebuilder(new Rebuilder(bhvr));
        EventSchedulerSingleton::Instance()->EnqueueEvent(rebuilder);
    }
    else
    {
        POS_TRACE_INFO((int)POS_EVENT_ID::REBUILD_DEBUG_MSG,
            "not a rebuild target partition");
        RebuildResult res;
        res.result = RebuildState::PASS;
        _Complete(res);
    }
}

void PartitionRebuild::Stop()
{
    if (_GetResult() <= RebuildState::REBUILDING && bhvr != nullptr)
    {
        bhvr->StopRebuilding();
    }
}

uint64_t PartitionRebuild::TotalStripes()
{
    if (bhvr != nullptr)
    {
        return bhvr->GetContext()->stripeCnt;
    }
    return 0;
}

void PartitionRebuild::_Complete(RebuildResult res)
{
    POS_TRACE_INFO((int)POS_EVENT_ID::REBUILD_DEBUG_MSG,
        "PartitionRebuild::_Complete, res {}", res.result);

    res.target = targetDev;
    if (bhvr != nullptr)
    {
        bhvr->UpdateProgress(bhvr->GetContext()->stripeCnt);
    }
    completeCb(res);
}

RebuildBehavior* PartitionRebuild::_GetRebuildBehavior(unique_ptr<RebuildContext> ctx)
{
    switch (ctx->raidType)
    {
    case RaidTypeEnum::RAID5 :
        return new Raid5Rebuild(move(ctx));

    case RaidTypeEnum::RAID1 :
        return new Raid1Rebuild(move(ctx));

    default:
        return nullptr;
    }
}

RebuildState PartitionRebuild::_GetResult()
{
    if (bhvr == nullptr)
    {
        return RebuildState::PASS;
    }
    return bhvr->GetContext()->result;
}


} // namespace pos

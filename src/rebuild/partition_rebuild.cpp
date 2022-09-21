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

#include "partition_rebuild.h"

#include "rebuilder.h"
#include "src/allocator_service/allocator_service.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
PartitionRebuild::PartitionRebuild(RebuildBehavior* b, EventScheduler* eventSchedulerArg)
: bhvr(b),
  eventScheduler(eventSchedulerArg)
{
    if (nullptr == eventScheduler)
    {
        eventScheduler = EventSchedulerSingleton::Instance();
    }
}

PartitionRebuild::~PartitionRebuild(void)
{
    delete bhvr;
}

void PartitionRebuild::Start(RebuildComplete cb)
{
    completeCb = cb;
    if (bhvr != nullptr)
    {
        string partName = PARTITION_TYPE_STR[bhvr->GetContext()->part];
        POS_TRACE_INFO(EID(PARTITION_REBUILD_START),
            "part_name:{}", partName);
        bhvr->GetContext()->SetResult(RebuildState::REBUILDING);
        bhvr->GetContext()->logger->SetPartitionRebuildStart(partName);
        bhvr->GetContext()->rebuildComplete =
            bind(&PartitionRebuild::_Complete, this, placeholders::_1);
        EventSmartPtr rebuilder(new Rebuilder(bhvr));
        eventScheduler->EnqueueEvent(rebuilder);
    }
    else
    {
        POS_TRACE_INFO(EID(PARTITION_REBUILD_SKIP),
            "not a target partition");
        RebuildResult res;
        res.result = RebuildState::PASS;
        _Complete(res);
    }
}

void PartitionRebuild::Stop(void)
{
    if (GetResult() <= RebuildState::REBUILDING && bhvr != nullptr)
    {
        string partName = PARTITION_TYPE_STR[bhvr->GetContext()->part];
        POS_TRACE_INFO(EID(PARTITION_REBUILD_STOP),
            "part_name:{}", partName);
        bhvr->StopRebuilding();
    }
}

uint64_t PartitionRebuild::TotalStripes(void)
{
    if (bhvr != nullptr)
    {
        return bhvr->GetContext()->stripeCnt;
    }
    return 0;
}

RebuildState PartitionRebuild::GetResult(void)
{
    if (bhvr == nullptr)
    {
        return RebuildState::PASS;
    }
    return bhvr->GetContext()->GetResult();
}

void PartitionRebuild::_Complete(RebuildResult res)
{
    string partition = "";
    if (bhvr != nullptr)
    {
        partition = PARTITION_TYPE_STR[bhvr->GetContext()->part];
    }
    POS_TRACE_INFO(EID(PARTITION_REBUILD_END),
        "array:{}, partition:{}, result:{}", res.array, partition, REBUILD_STATE_STR[(int)res.result]);
    if (completeCb != nullptr)
    {
        completeCb(res);
    }
    else
    {
        POS_TRACE_ERROR(EID(PARTITION_REBUILD_END), "No proper callback");
    }
}

} // namespace pos

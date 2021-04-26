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

#include "replay_logs.h"

#include <iomanip>
#include <iostream>
#include <algorithm>

#include "src/journal_manager/log/log_handler.h"
#include "src/journal_manager/replay/log_delete_checker.h"
#include "src/journal_manager/replay/pending_stripe.h"
#include "src/journal_manager/replay/gc_replay_stripe.h"
#include "src/journal_manager/replay/user_replay_stripe.h"

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
ReplayLogs::ReplayLogs(ReplayLogList& logList, IVSAMap* vsaMap, IStripeMap* stripeMap,
    IBlockAllocator* blockAllocator, IWBStripeAllocator* wbStripeAllocator,
    IWBStripeCtx* wbStripeCtx, ISegmentCtx* segmentCtx, IArrayInfo* arrayInfo,
    ReplayProgressReporter* reporter, PendingStripeList& pendingWbStripes)
: ReplayTask(reporter),
  logList(logList),
  vsaMap(vsaMap),
  stripeMap(stripeMap),
  blockAllocator(blockAllocator),
  wbStripeAllocator(wbStripeAllocator),
  wbStripeCtx(wbStripeCtx),
  segmentCtx(segmentCtx),
  arrayInfo(arrayInfo)
{
    logDeleteChecker = new LogDeleteChecker();

    wbStripeReplayer = new ActiveWBStripeReplayer(wbStripeCtx,
        wbStripeAllocator, pendingWbStripes);
    userStripeReplayer = new ActiveUserStripeReplayer(segmentCtx, arrayInfo);
}

ReplayLogs::~ReplayLogs(void)
{
    for (auto stripe : replayingStripeList)
    {
        delete stripe;
    }
    replayingStripeList.clear();

    for (auto stripe : replayedStripeList)
    {
        delete stripe;
    }
    replayedStripeList.clear();

    delete logDeleteChecker;

    delete wbStripeReplayer;
    delete userStripeReplayer;
}

int
ReplayLogs::GetNumSubTasks(void)
{
    return 4;
}

int
ReplayLogs::Start(void)
{
    int result = 0;

    POS_TRACE_DEBUG(POS_EVENT_ID::JOURNAL_REPLAY_STATUS,
        "[ReplayTask] Log replay started");

    logDeleteChecker->Update(logList.GetDeletingLogs());

    result = _ReplayFinishedStripes(logList.GetReplayLogs());
    if (result != 0)
    {
        return result;
    }
    reporter->SubTaskCompleted(GetId(), 1);

    result = _ReplayUnfinishedStripes();
    if (result != 0)
    {
        return result;
    }
    reporter->SubTaskCompleted(GetId(), 1);

    result = wbStripeReplayer->Replay();
    if (result != 0)
    {
        return result;
    }
    reporter->SubTaskCompleted(GetId(), 1);

    result = userStripeReplayer->Replay();
    if (result != 0)
    {
        return result;
    }
    reporter->SubTaskCompleted(GetId(), 1);

    std::ostringstream os;
    os << "[Replay] " << replayedStripeList.size() << " stripes are replayed";

    POS_TRACE_DEBUG(POS_EVENT_ID::JOURNAL_REPLAY_STATUS, os.str());
    POS_TRACE_DEBUG_IN_MEMORY(ModuleInDebugLogDump::JOURNAL,
        POS_EVENT_ID::JOURNAL_REPLAY_STATUS, os.str());

    return result;
}

int
ReplayLogs::_ReplayFinishedStripes(std::vector<ReplayLog>& replayLogs)
{
    for (auto replayLog : replayLogs)
    {
        LogHandlerInterface* log = replayLog.log;

        if (log->GetType() == LogType::BLOCK_WRITE_DONE)
        {
            ReplayStripe* stripe = _FindUserStripe(log->GetVsid());
            stripe->AddLog(log);
        }
        else if (log->GetType() == LogType::STRIPE_MAP_UPDATED)
        {
            ReplayStripe* stripe = _FindUserStripe(log->GetVsid());
            stripe->AddLog(log);

            int result = _ReplayStripe(stripe);
            if (result != 0)
            {
                return result;
            }

            _MoveToReplayedStripe(stripe);
        }
        else if (log->GetType() == LogType::GC_STRIPE_FLUSHED)
        {
            ReplayStripe* stripe = new GcReplayStripe(log->GetVsid(), vsaMap, stripeMap,
                wbStripeCtx, segmentCtx, blockAllocator, arrayInfo,
                wbStripeReplayer, userStripeReplayer);
            stripe->AddLog(log);

            int result = _ReplayStripe(stripe);
            if (result != 0)
            {
                return result;
            }

            replayedStripeList.push_back(stripe);
        }
        else
        {
            POS_TRACE_DEBUG(POS_EVENT_ID::JOURNAL_REPLAY_STATUS,
                "Unknwon log type {} found", log->GetType());
        }

        logDeleteChecker->ReplayedUntil(replayLog.time);
    }
    return 0;
}

int
ReplayLogs::_ReplayStripe(ReplayStripe* stripe)
{
    if (logDeleteChecker->IsDeleted(stripe->GetVolumeId()))
    {
        stripe->DeleteBlockMapReplayEvents();
    }

    return stripe->Replay();
}

void
ReplayLogs::_MoveToReplayedStripe(ReplayStripe* stripe)
{
    replayedStripeList.push_back(stripe);

    replayingStripeList.erase(std::remove(replayingStripeList.begin(),
        replayingStripeList.end(), stripe), replayingStripeList.end());
}

ReplayStripe*
ReplayLogs::_FindUserStripe(StripeId vsid)
{
    for (auto stripe : replayingStripeList)
    {
        if (stripe->IsFlushed() == false && stripe->GetVsid() == vsid)
        {
            return stripe;
        }
    }

    ReplayStripe* stripe = new UserReplayStripe(vsid, vsaMap, stripeMap,
        wbStripeCtx, segmentCtx, blockAllocator, arrayInfo,
        wbStripeReplayer, userStripeReplayer);

    replayingStripeList.push_back(stripe);

    return stripe;
}

int
ReplayLogs::_ReplayUnfinishedStripes(void)
{
    POS_TRACE_DEBUG(POS_EVENT_ID::JOURNAL_REPLAY_STATUS,
        "{} stripes are not flushed", replayingStripeList.size());

    while (replayingStripeList.size() != 0)
    {
        ReplayStripe* stripe = replayingStripeList.front();

        int result = stripe->Replay();
        if (result != 0)
        {
            return result;
        }

        _MoveToReplayedStripe(stripe);
    }
    return 0;
}

ReplayTaskId
ReplayLogs::GetId(void)
{
    return ReplayTaskId::REPLAY_LOGS;
}

int
ReplayLogs::GetWeight(void)
{
    return 30;
}

} // namespace pos

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
ReplayLogs::ReplayLogs(ReplayLogList& logList, LogDeleteChecker* deleteChecker,
    IVSAMap* vsaMap, IStripeMap* stripeMap,
    ISegmentCtx* segmentCtx, IWBStripeAllocator* wbStripeAllocator,
    IContextReplayer* ctxReplayer, IArrayInfo* arrayInfo,
    ReplayProgressReporter* reporter, PendingStripeList& pendingWbStripes)
: ReplayTask(reporter),
  logList(logList),
  logDeleteChecker(deleteChecker),
  vsaMap(vsaMap),
  stripeMap(stripeMap),
  segmentCtx(segmentCtx),
  wbStripeAllocator(wbStripeAllocator),
  contextReplayer(ctxReplayer),
  arrayInfo(arrayInfo)
{
    wbStripeReplayer = new ActiveWBStripeReplayer(contextReplayer,
        wbStripeAllocator, stripeMap, pendingWbStripes, arrayInfo);
    userStripeReplayer = new ActiveUserStripeReplayer(contextReplayer, arrayInfo);
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

    for (auto replayLog : replayLogs)
    {
        delete replayLog.log;
    }
    replayLogs.clear();

    delete wbStripeReplayer;
    delete userStripeReplayer;
}

int
ReplayLogs::GetNumSubTasks(void)
{
    return 5;
}

int
ReplayLogs::Start(void)
{
    int result = 0;

    POS_TRACE_INFO(EID(JOURNAL_REPLAY_STATUS),
        "[ReplayTask] Log replay started");

    logDeleteChecker->Update(logList.GetDeletingLogs());

    result = _ReplayFinishedStripes();
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

    contextReplayer->ResetSegmentsStates();
    reporter->SubTaskCompleted(GetId(), 1);

    std::ostringstream os;
    os << "numStripesReplayed:" << replayedStripeList.size();

    POS_TRACE_DEBUG(EID(JOURNAL_REPLAY_STATUS), os.str());
    POS_TRACE_DEBUG_IN_MEMORY(ModuleInDebugLogDump::JOURNAL,
        EID(JOURNAL_REPLAY_STATUS), os.str());

    return result;
}

int
ReplayLogs::_ReplayFinishedStripes(void)
{
    replayLogs.clear();
    replayLogs = logList.PopReplayLogGroup();

    POS_TRACE_TRACE(EID(JOURNAL_REPLAY_STATUS),
        "Start replaying logs, numLogs:{}", replayLogs.size());

    int result = 0;

    int numReplayedUserStripes = 0;
    int numReplayedGcStripes = 0;

    for (auto replayLog : replayLogs)
    {
        LogHandlerInterface* log = replayLog.log;

        if (log->GetType() == LogType::BLOCK_WRITE_DONE)
        {
            ReplayStripe* stripe = _FindUserStripe(log->GetVsid());
            stripe->AddLog(replayLog);

            int volumeId = reinterpret_cast<BlockWriteDoneLog*>(log->GetData())->volId;
            logDeleteChecker->ReplayedUntil(replayLog.time, volumeId);
        }
        else if (log->GetType() == LogType::STRIPE_MAP_UPDATED)
        {
            ReplayStripe* stripe = _FindUserStripe(log->GetVsid());
            stripe->AddLog(replayLog);

            result = _ReplayStripe(stripe);
            numReplayedUserStripes++;
            if (result != 0)
            {
                break;
            }

            _MoveToReplayedStripe(stripe);
        }
        else if (log->GetType() == LogType::GC_BLOCK_WRITE_DONE)
        {
            ReplayStripe* stripe = _FindGcStripe(log->GetVsid());
            stripe->AddLog(replayLog);

            int volumeId = reinterpret_cast<GcBlockWriteDoneLog*>(log->GetData())->volId;
            logDeleteChecker->ReplayedUntil(replayLog.time, volumeId);
        }
        else if (log->GetType() == LogType::GC_STRIPE_FLUSHED)
        {
            ReplayStripe* stripe = _FindGcStripe(log->GetVsid());
            stripe->AddLog(replayLog);

            result = _ReplayStripe(stripe);
            numReplayedGcStripes++;
            if (result != 0)
            {
                break;
            }

            _MoveToReplayedStripe(stripe);
        }
        else
        {
            POS_TRACE_WARN(EID(JOURNAL_REPLAY_STATUS),
                "Unknwon log type {} found", log->GetType());
        }
    }

    POS_TRACE_TRACE(EID(JOURNAL_REPLAY_STATUS), "Replayed finished stripes, numReplayedUserStripes:{}, numReplayedGcStripes:{}",
        numReplayedUserStripes, numReplayedGcStripes);

    return result;
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
        contextReplayer, segmentCtx, arrayInfo,
        wbStripeReplayer, userStripeReplayer);

    replayingStripeList.push_back(stripe);

    return stripe;
}

ReplayStripe*
ReplayLogs::_FindGcStripe(StripeId vsid)
{
    for (auto stripe : replayingStripeList)
    {
        if (stripe->IsFlushed() == false && stripe->GetVsid() == vsid)
        {
            return stripe;
        }
    }

    ReplayStripe* stripe = new GcReplayStripe(vsid, vsaMap, stripeMap,
        contextReplayer, segmentCtx, arrayInfo,
        wbStripeReplayer, userStripeReplayer);

    replayingStripeList.push_back(stripe);

    return stripe;
}

int
ReplayLogs::_ReplayUnfinishedStripes(void)
{
    POS_TRACE_TRACE(EID(JOURNAL_REPLAY_STATUS),
        "Start replaying unflushed stripes, numUnflushedStripes:{}",
        replayingStripeList.size());

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

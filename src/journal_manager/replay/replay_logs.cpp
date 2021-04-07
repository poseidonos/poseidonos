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

#include "pending_stripe.h"
#include "src/include/ibof_event_id.h"
#include "src/logger/logger.h"

namespace ibofos
{
ReplayLogs::ReplayLogs(LogList& logList, Mapper* mapper, Allocator* allocator,
    Array* array, ReplayProgressReporter* reporter, PendingStripeList& pendingWbStripes)
: ReplayTask(reporter),
  logList(logList),
  mapper(mapper),
  allocator(allocator),
  array(array)
{
    wbStripeReplayer = new ActiveWBStripeReplayer(allocator, pendingWbStripes);
    userStripeReplayer = new ActiveUserStripeReplayer(allocator, array);
}

ReplayLogs::~ReplayLogs(void)
{
    for (auto stripe : replayStripeList)
    {
        delete stripe;
    }
    replayStripeList.clear();

    delete wbStripeReplayer;
    delete userStripeReplayer;
}

int
ReplayLogs::GetNumSubTasks(void)
{
    return 2;
}

int
ReplayLogs::Start(void)
{
    _CreateReplayStripe();
    reporter->SubTaskCompleted(GetId(), 1);

    int result = _Replay();
    reporter->SubTaskCompleted(GetId(), 1);

    return result;
}

void
ReplayLogs::_CreateReplayStripe(void)
{
    for (auto log : logList)
    {
        if (log->GetType() == LogType::VOLUME_DELETED)
        {
            _DeleteVolumeLogs(log);
        }
        else
        {
            ReplayStripe* stripe = _FindStripe(log->GetVsid());
            if (stripe == nullptr)
            {
                // TODO(huijeong.kim) There can be two stripes with same vsid
                // if there was a GC in-between
                stripe = new ReplayStripe(log->GetVsid(), mapper, allocator, array,
                    wbStripeReplayer, userStripeReplayer);
                replayStripeList.push_back(stripe);

                IBOF_TRACE_INFO(EID(JOURNAL_REPLAY_STRIPE),
                    "[Replay] new stripe (vsid {}) is found for replay", log->GetVsid());
            }
            stripe->AddLog(log);
        }
        delete log;
    }
    logList.clear();
}

void
ReplayLogs::_DeleteVolumeLogs(LogHandlerInterface* log)
{
    VolumeDeletedLog* logData = reinterpret_cast<VolumeDeletedLog*>(log->GetData());

    IBOF_TRACE_INFO(EID(JOURNAL_REPLAY_VOLUME_EVENT),
        "[Replay] Deleted volume {} is found", logData->volId);

    for (auto stripe : replayStripeList)
    {
        if (stripe->GetVolumeId() == logData->volId)
        {
            stripe->DeleteBlockMapReplayEvents();
        }
    }
}

int
ReplayLogs::_Replay(void)
{
    int result = 0;

    for (auto stripe : replayStripeList)
    {
        result = stripe->Replay();
        if (result != 0)
        {
            IBOF_TRACE_ERROR(EID(JOURNAL_REPLAY_FAILED),
                "[Replay] Replay Failed, Stop Replaying");
            return result;
        }
    }

    wbStripeReplayer->Replay();
    userStripeReplayer->Replay();

    return result;
}

ReplayStripe*
ReplayLogs::_FindStripe(StripeId vsid)
{
    ReplayStripe* stripeFound = nullptr;
    for (auto stripe : replayStripeList)
    {
        if (stripe->GetVsid() == vsid)
        {
            stripeFound = stripe;
            break;
        }
    }
    return stripeFound;
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

} // namespace ibofos

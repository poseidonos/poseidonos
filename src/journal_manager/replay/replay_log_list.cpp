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

#include "src/journal_manager/replay/replay_log_list.h"

#include "src/journal_manager/log/log_handler.h"
#include "src/journal_manager/log/volume_deleted_log_handler.h"
#include "src/logger/logger.h"

namespace pos
{
ReplayLogList::ReplayLogList(void)
: time(0)
{
}

ReplayLogList::~ReplayLogList(void)
{
    for (auto logGroup : logGroups)
    {
        logGroup.second.logs.clear();
    }
    logGroups.clear();

    for (auto replayLog : deletingLogs)
    {
        delete replayLog.log;
    }
    deletingLogs.clear();
}

void
ReplayLogList::AddLog(LogHandlerInterface* log)
{
    ReplayLog replayLog = {
        .time = _GetTime(),
        .log = log};

    if (log->GetType() == LogType::VOLUME_DELETED)
    {
        deletingLogs.push_back(replayLog);
    }
    else
    {
        logGroups[log->GetSeqNum()].logs.push_back(replayLog);
        logGroups[log->GetSeqNum()].seqNum = log->GetSeqNum();
        logGroups[log->GetSeqNum()].isFooterValid = false;
    }
}

uint64_t
ReplayLogList::_GetTime(void)
{
    return time++;
}

bool
ReplayLogList::IsEmpty(void)
{
    return (logGroups.size() == 0);
}

void
ReplayLogList::SetLogGroupFooter(uint32_t seqNum, LogGroupFooter footer)
{
    logGroups[seqNum].footer = footer;
    logGroups[seqNum].isFooterValid = true;
}

void
ReplayLogList::EraseReplayLogGroup(uint32_t seqNum)
{
    ReplayLogGroup replayLogGroup = logGroups[seqNum];
    int event = static_cast<int>(EID(JOURNAL_INVALID_LOG_FOUND));
    POS_TRACE_INFO(event, "Erasing Replay Log Group for SeqNum {} with {} entries", seqNum, replayLogGroup.logs.size());
    for (ReplayLog replayLog : replayLogGroup.logs)
    {
        delete replayLog.log;
    }
    logGroups.erase(seqNum);
}

ReplayLogGroup
ReplayLogList::PopReplayLogGroup(void)
{
    ReplayLogGroup logGroup = logGroups.begin()->second;
    logGroups.erase(logGroups.begin());
    return logGroup;
}

std::vector<ReplayLog>&
ReplayLogList::GetDeletingLogs(void)
{
    return deletingLogs;
}

} // namespace pos

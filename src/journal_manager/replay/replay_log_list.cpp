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

#include "src/journal_manager/log/log_event.h"
#include "src/journal_manager/log/log_handler.h"
#include "src/journal_manager/log/volume_deleted_log_handler.h"
#include "src/logger/logger.h"

#include "src/journal_manager/log/block_write_done_log_handler.h"

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
        for (auto seqNumGroup : logGroup)
        {
            seqNumGroup.second.logs.clear();
        }
        logGroup.clear();
    }
    logGroups.clear();

    for (auto replayLog : deletingLogs)
    {
        delete replayLog.log;
    }
    deletingLogs.clear();
}

void
ReplayLogList::Init(int numLogGroups)
{
    logGroups.resize(numLogGroups);

    // Invalid log group footer will be used, when log group footer was never written
    LogGroupFooter invalidFooter;
    invalidFooter.isReseted = false;
    invalidFooter.lastCheckpointedSeginfoVersion = UINT32_MAX;
    invalidFooter.resetedSequenceNumber = UINT32_MAX;
    footers.resize(numLogGroups, invalidFooter);
}

void
ReplayLogList::AddLog(int logGroupId, LogHandlerInterface* log)
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
        uint32_t seqNum = log->GetSeqNum();

        if (logGroups[logGroupId].find(seqNum) == logGroups[logGroupId].end())
        {
            ReplayLogGroup logGroup(seqNum);
            logGroups[logGroupId].emplace(seqNum, logGroup);
        }

        logGroups[logGroupId][seqNum].logs.push_back(replayLog);
        logGroups[logGroupId][seqNum].logsFoundPerType[(int)(log->GetType())]++;
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
    for (auto logGroup: logGroups)
    {
        if (logGroup.size() != 0)
        {
            return false;
        }
    }
    return true;
}

void
ReplayLogList::SetLogGroupFooter(int logGroupId, LogGroupFooter footer)
{
    footers[logGroupId] = footer;

    POS_TRACE_INFO(EID(JOURNAL_REPLAY_STATUS),
        "Footer found, logGroupId:{}, lastCheckpointedSeginfoVersion:{}, isReseted:{}, resetedSequenceNumber:{}",
        logGroupId, footer.lastCheckpointedSeginfoVersion, footer.isReseted ? "true" : "false", footer.resetedSequenceNumber);
}

LogGroupFooter
ReplayLogList::GetLogGroupFooter(int logGroupId)
{
    return footers[logGroupId];
}

void
ReplayLogList::PrintLogStatistics(void)
{
    for (int id = 0; id < (int)logGroups.size(); id++)
    {
        for (auto seqNumGroup : logGroups[id])
        {
            uint32_t sequenceNumber = seqNumGroup.first;
            auto logsFoundPerType = seqNumGroup.second.logsFoundPerType;

            int numBlockMapUpdatedLogs = logsFoundPerType[(int)LogType::BLOCK_WRITE_DONE];
            int numStripeMapUpdatedLogs = logsFoundPerType[(int)LogType::STRIPE_MAP_UPDATED];
            int numGcStripeFlushedLogs = logsFoundPerType[(int)LogType::GC_STRIPE_FLUSHED];
            int numVolumeDeletedLogs = logsFoundPerType[(int)LogType::VOLUME_DELETED];
            POS_TRACE_INFO(EID(JOURNAL_REPLAY_STATUS),
                "Logs found: logGroupId:{}, SeqNum: {}, total: {}, block_map: {}, stripe_map: {}, gc_stripes: {}, volumes_deleted: {}",
                id, sequenceNumber, seqNumGroup.second.logs.size(), numBlockMapUpdatedLogs, numStripeMapUpdatedLogs, numGcStripeFlushedLogs, numVolumeDeletedLogs);
        }
    }
}

int
ReplayLogList::EraseReplayLogGroup(int logGroupId, uint32_t seqNum)
{
    POS_TRACE_INFO(EID(JOURNAL_LOG_GROUP_FOOTER_FOUND),
        "Log Group Reset by footer found. Logs with SeqNumber ({}) or less will be removed",
        seqNum);

    for (auto it = logGroups[logGroupId].cbegin(); it != logGroups[logGroupId].cend();)
    {
        // TODO (cheolho.kang): It should be refactored later to erase replay logs with invalid sequence numbers using LogBufferParser.logs 
        if (it->first <= seqNum || it->first == LOG_VALID_MARK)
        {
            int event = static_cast<int>(EID(JOURNAL_REPLAY_STATUS));
            POS_TRACE_INFO(event, "Erasing logs, logGroupId:{}, seqNum:{}, numLogsErased:{}", logGroupId, it->first, it->second.logs.size());
            for (ReplayLog replayLog : it->second.logs)
            {
                delete replayLog.log;
            }
            logGroups[logGroupId].erase(it++);
        }
        else
        {
            it++;
        }
    }

    if (logGroups[logGroupId].size() > 1)
    {
        std::string seqNumList = "";
        for (auto it = logGroups[logGroupId].cbegin(); it != logGroups[logGroupId].cend(); it++)
        {
            seqNumList += (std::to_string(it->first) + ", ");
        }
        POS_TRACE_ERROR(EID(JOURNAL_INVALID_LOG_FOUND),
            "Several sequence numbers are found in single log group, logGroupId:{}, resetedSequenceNumber:{}, seqNumsSeen:{}",
            logGroupId, footers[logGroupId].resetedSequenceNumber, seqNumList);
        return ERRID(JOURNAL_INVALID_LOG_FOUND);
    }

    return EID(SUCCESS);
}

void
ReplayLogList::SetSegInfoFlushed(int logGroupId)
{
    assert(logGroups[logGroupId].size() <= 1);

    if (logGroups[logGroupId].size() == 1)
    {
        auto replayLogGroup = logGroups[logGroupId].begin()->second;
        POS_TRACE_INFO(EID(JOURNAL_REPLAY_STATUS), "SetSegInfoFlushed, id:{}, numLogs:{}", logGroupId, replayLogGroup.logs.size());

        for (auto it = replayLogGroup.logs.begin(); it != replayLogGroup.logs.end(); it++)
        {
            it->segInfoFlushed = true;
        }
    }
}

std::vector<ReplayLog>
ReplayLogList::PopReplayLogGroup(void)
{
    SeqNumGroup logGroupsBySeqNum;
    for (int id = 0; id < (int)logGroups.size(); id++)
    {
        assert(logGroups[id].size() <= 1);

        if (logGroups[id].size() == 1)
        {
            auto replayLogGroup = logGroups[id].begin()->second;
            logGroupsBySeqNum.emplace(replayLogGroup.seqNum, replayLogGroup);
        }
    }

    std::vector<ReplayLog> returnLogs;
    for (auto logGroup : logGroupsBySeqNum)
    {
        auto seqNum = logGroup.first;
        auto replayLogGroup = logGroup.second;

        POS_TRACE_INFO(EID(JOURNAL_REPLAY_STATUS),
            "Adding logs to replay, seqNum:{}, numLogs:{}", seqNum, replayLogGroup.logs.size());

        returnLogs.insert(returnLogs.end(), replayLogGroup.logs.begin(), replayLogGroup.logs.end());
    }

    return returnLogs;
}

std::vector<ReplayLog>&
ReplayLogList::GetDeletingLogs(void)
{
    return deletingLogs;
}

} // namespace pos

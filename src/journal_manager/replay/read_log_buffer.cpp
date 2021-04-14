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

#include "read_log_buffer.h"

#include <queue>
#include <vector>
#include <iostream>
#include <iomanip>

#include "../config/journal_configuration.h"
#include "../log/log_buffer_parser.h"
#include "../log_buffer/journal_log_buffer.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
namespace pos
{

ReadLogBuffer::ReadLogBuffer(JournalConfiguration* journalConfig,
    JournalLogBuffer* logBuffer, LogList& logList, ReplayProgressReporter* reporter)
: ReplayTask(reporter),
  config(journalConfig),
  logBuffer(logBuffer),
  logList(logList)
{
}

ReadLogBuffer::~ReadLogBuffer(void)
{
}

int
ReadLogBuffer::GetNumSubTasks(void)
{
    return 2 * config->GetNumLogGroups();
}

int
ReadLogBuffer::Start(void)
{
    int eventId = static_cast<int>(POS_EVENT_ID::JOURNAL_REPLAY_STATUS);
    POS_TRACE_DEBUG(eventId, "[ReplayTask] Read log buffer started");

    int result = 0;

    auto comp = [](LogList a, LogList b) { return (a.back()->GetSeqNum() > b.back()->GetSeqNum());};
    std::priority_queue<LogList, std::vector<LogList>, decltype(comp)> logs(comp);
    LogBufferParser parser;

    int numLogGroups = config->GetNumLogGroups();
    uint64_t groupSize = config->GetLogGroupSize();
    void* logGroupBuffer = malloc(groupSize);

    for (int groupId = 0; groupId < numLogGroups; groupId++)
    {
        result = logBuffer->ReadLogBuffer(groupId, logGroupBuffer);
        if (result != 0)
        {
            break;
        }

        LogList groupLogs;
        result = parser.GetLogs(logGroupBuffer, groupSize, groupLogs);
        if (result != 0)
        {
            break;
        }

        if (groupLogs.size() != 0)
        {
            logs.push(groupLogs);
            POS_TRACE_DEBUG(eventId, "{} logs are found from log group {}",
                groupLogs.size(), groupId);
        }
        reporter->SubTaskCompleted(GetId(), 1);
    }

    if (result == 0)
    {
        while (logs.size() != 0)
        {
            LogList groupLogs = logs.top();
            logList.insert(logList.end(), groupLogs.begin(), groupLogs.end());
            logs.pop();

            reporter->SubTaskCompleted(GetId(), 1);
        }

        if (logList.size() == 0)
        {
            int eventId = static_cast<int>(POS_EVENT_ID::JOURNAL_REPLAY_STOPPED);
            std::ostringstream os;
            os << "No logs to replay. Stop replaying";

            POS_TRACE_DEBUG(eventId, os.str());
            POS_TRACE_DEBUG_IN_MEMORY(ModuleInDebugLogDump::JOURNAL, eventId, os.str());
            result = eventId;
        }
    }

    free(logGroupBuffer);
    return result;
}

ReplayTaskId
ReadLogBuffer::GetId(void)
{
    return ReplayTaskId::READ_LOG_BUFFER;
}

int
ReadLogBuffer::GetWeight(void)
{
    return 50;
}

} // namespace pos

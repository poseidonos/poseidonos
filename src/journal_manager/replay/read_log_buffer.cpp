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

#include "read_log_buffer.h"

#include <queue>
#include <vector>
#include <iostream>
#include <iomanip>

#include "src/journal_manager/config/journal_configuration.h"
#include "src/journal_manager/replay/replay_log_list.h"
#include "src/journal_manager/log/log_buffer_parser.h"
#include "src/journal_manager/log_buffer/journal_log_buffer.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
namespace pos
{
ReadLogBuffer::ReadLogBuffer(JournalConfiguration* journalConfig,
    IJournalLogBuffer* logBuffer, ReplayLogList& logList, ReplayProgressReporter* reporter)
: ReadLogBuffer(journalConfig, logBuffer, logList, reporter, new LogBufferParser())
{
}

ReadLogBuffer::ReadLogBuffer(JournalConfiguration* journalConfig,
    IJournalLogBuffer* logBuffer, ReplayLogList& logList,
    ReplayProgressReporter* reporter, LogBufferParser* logBufferParser)
: ReplayTask(reporter),
  config(journalConfig),
  logBuffer(logBuffer),
  logList(logList),
  parser(logBufferParser)
{
}

ReadLogBuffer::~ReadLogBuffer(void)
{
    for (auto buffer : readLogBuffer)
    {
        free(buffer);
    }
    readLogBuffer.clear();
    if (parser != nullptr)
    {
        delete parser;
    }
}

int
ReadLogBuffer::GetNumSubTasks(void)
{
    return config->GetNumLogGroups();
}

int
ReadLogBuffer::Start(void)
{
    int eventId = static_cast<int>(EID(JOURNAL_REPLAY_STATUS));
    POS_TRACE_DEBUG(eventId, "[ReplayTask] Read log buffer started");

    int result = 0;
    int numLogGroups = config->GetNumLogGroups();
    uint64_t groupSize = config->GetLogGroupSize() * 2;

    for (int groupId = 0; groupId < numLogGroups; groupId++)
    {
        void* logGroupBuffer = calloc(groupSize, sizeof(char));
        readLogBuffer.push_back(logGroupBuffer);

        result = logBuffer->ReadLogBuffer(groupId, logGroupBuffer);
        if (result != 0)
        {
            break;
        }

        result = parser->GetLogs(logGroupBuffer, groupSize, logList);
        if (result != 0)
        {
            break;
        }
        reporter->SubTaskCompleted(GetId(), 1);
    }

    if (result == 0)
    {
        if (logList.IsEmpty() == true)
        {
            int eventId = static_cast<int>(EID(JOURNAL_REPLAY_STOPPED));
            std::ostringstream os;
            os << "No logs to replay. Stop replaying";

            POS_TRACE_INFO(eventId, os.str());
            POS_TRACE_DEBUG_IN_MEMORY(ModuleInDebugLogDump::JOURNAL, eventId, os.str());
            result = eventId;
        }
    }

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

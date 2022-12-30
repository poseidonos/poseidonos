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

#include "src/journal_manager/replay/filter_logs.h"

#include "src/allocator/i_context_manager.h"
#include "src/include/pos_event_id.h"
#include "src/journal_manager/config/journal_configuration.h"
#include "src/journal_manager/replay/replay_log_list.h"
#include "src/journal_manager/replay/replay_progress_reporter.h"
#include "src/logger/logger.h"

namespace pos
{
FilterLogs::FilterLogs(JournalConfiguration* config, ReplayLogList& logList,
    IContextManager* contextManager,
    ReplayProgressReporter* reporter)
: ReplayTask(reporter),
  config(config),
  logList(logList),
  contextManager(contextManager)
{
}

int
FilterLogs::Start(void)
{
    POS_TRACE_INFO(EID(JOURNAL_REPLAY_STATUS), "[ReplayTask] Filter logs started");

    int ret = _EraseInvalidSequenceNumbers();
    if (ret != EID(SUCCESS))
    {
        return ret;
    }
    reporter->SubTaskCompleted(GetId(), 1);

    ret = _UpdateSegInfoFlushed();
    if (ret != EID(SUCCESS))
    {
        return ret;
    }
    reporter->SubTaskCompleted(GetId(), 1);

    if (logList.IsEmpty() == true)
    {
        int eventId = static_cast<int>(EID(JOURNAL_REPLAY_STOPPED));
        std::ostringstream os;
        os << "No logs to replay. Stop replaying";

        POS_TRACE_INFO(eventId, os.str());
        POS_TRACE_DEBUG_IN_MEMORY(ModuleInDebugLogDump::JOURNAL, eventId, os.str());
        ret = eventId;
    }
    else
    {
        logList.PrintLogStatistics();
    }

    return ret;
}

int
FilterLogs::_EraseInvalidSequenceNumbers(void)
{
    // TODO (cheolho.kang): To handle the case where the valid mark is read incorrectly due to offset align.
    // Need to add a LogGroup Header, record the sequence number to be used in the future
    // and have to fix it to erase invalid logs based on this.

    int ret = EID(SUCCESS);
    for (int id = 0; id < config->GetNumLogGroups(); id++)
    {
        LogGroupFooter footer = logList.GetLogGroupFooter(id);

        // Case 1. Footer was not written, then isReseted = false, resetedSequenceNumber = UINT32_MAX
        //    => DO NOT ERASE ANY LOG
        // Case 2. Footer written once on first-used group, then isReseted = false, resetedSequenceNumber = UINT32_MAX
        //    => DO NOT ERASE ANY LOG
        // Case 3. Footer written once on re-used group, then isReseted = false, resetedSequenceNumber = valid number
        //    => ERASE LOGS with the sequence number or less
        // Case 4. Footer written twice, then isReseted = true, resetedSequenceNumber = valid number
        //    => ERASE LOGS with the sequence number or less

        if (footer.isReseted == true)
        {
            ret = logList.EraseReplayLogGroup(id, footer.resetedSequenceNumber);
            if (ret != EID(SUCCESS))
            {
                return ret;
            }
        }
        else if (footer.resetedSequenceNumber != UINT32_MAX)
        {
            ret = logList.EraseReplayLogGroup(id, footer.resetedSequenceNumber);
            if (ret != EID(SUCCESS))
            {
                return ret;
            }
        }
    }

    return ret;
}

int
FilterLogs::_UpdateSegInfoFlushed(void)
{
    // All invalid logs are removed before reaching here
    // Walk through log groups, and find the log group which is not reseted,
    // and have new version of seg info than current version
    uint64_t currentSegInfoVersion = contextManager->GetStoredContextVersion(SEGMENT_CTX);
    for (int id = 0; id < config->GetNumLogGroups(); id++)
    {
        LogGroupFooter footer = logList.GetLogGroupFooter(id);

        if (footer.isReseted == false)
        {
            // Checkpoint started, allocator context is stored, but checkpoint is not completed
            if (footer.lastCheckpointedSeginfoVersion < currentSegInfoVersion)
            {
                POS_TRACE_INFO(EID(JOURNAL_REPLAY_STATUS),
                    "logGroupId:{}, lastCheckpointedSeginfoVersion:{}, currentSegInfoVersion:{}",
                    id, footer.lastCheckpointedSeginfoVersion, currentSegInfoVersion);

                logList.SetSegInfoFlushed(id);
            }
        }
    }
    return EID(SUCCESS);
}

ReplayTaskId
FilterLogs::GetId(void)
{
    return ReplayTaskId::FILTER_LOGS;
}

int
FilterLogs::GetWeight(void)
{
    return 10;
}

int
FilterLogs::GetNumSubTasks(void)
{
    return 2;
}

} // namespace pos

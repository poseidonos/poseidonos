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

#include "replay_handler.h"

#include <functional>
#include <string>

#include "../config/journal_configuration.h"
#include "../log_buffer/journal_log_buffer.h"
#include "flush_metadata.h"
#include "flush_pending_stripes.h"
#include "read_log_buffer.h"
#include "replay_logs.h"
#include "reset_log_buffer.h"

namespace pos
{
ReplayHandler::ReplayHandler(IStateControl* iState)
: replayState(iState),
  config(nullptr),
  logBuffer(nullptr)
{
    reporter = new ReplayProgressReporter();
}

void
ReplayHandler::Init(JournalConfiguration* journalConfiguration,
    JournalLogBuffer* journalLogBuffer, IVSAMap* vsaMap, IStripeMap* stripeMap,
    IMapFlush* mapFlush, IBlockAllocator* blockAllocator,
    IWBStripeAllocator* wbStripeAllocator, IContextManager* contextManager, IContextReplayer* contextReplayer, IArrayInfo* arrayInfo)
{
    config = journalConfiguration;
    logBuffer = journalLogBuffer;

    _InitializeTaskList(vsaMap, stripeMap, mapFlush, blockAllocator,
        wbStripeAllocator, contextManager, contextReplayer, arrayInfo);
}

void
ReplayHandler::_InitializeTaskList(IVSAMap* vsaMap, IStripeMap* stripeMap,
    IMapFlush* mapFlush, IBlockAllocator* blockAllocator,
    IWBStripeAllocator* wbStripeAllocator, IContextManager* contextManager, IContextReplayer* contextReplayer, IArrayInfo* arrayInfo)
{
    _AddTask(new ReadLogBuffer(config, logBuffer, logList, reporter));
    _AddTask(new ReplayLogs(logList, vsaMap, stripeMap, blockAllocator,
        wbStripeAllocator, contextReplayer, arrayInfo, reporter, pendingWbStripes));
    _AddTask(new FlushMetadata(mapFlush, contextManager, reporter));
    _AddTask(new ResetLogBuffer(logBuffer, reporter));
    _AddTask(new FlushPendingStripes(pendingWbStripes, wbStripeAllocator, reporter));
}

void
ReplayHandler::_AddTask(ReplayTask* task)
{
    taskList.push_back(task);
    reporter->RegisterTask(task->GetId(), task->GetWeight());
}

ReplayHandler::~ReplayHandler(void)
{
    delete reporter;

    for (auto task : taskList)
    {
        delete task;
    }
    taskList.clear();
}

int
ReplayHandler::Start(void)
{
    replayState.GetRecoverState();

    int eventId = static_cast<int>(POS_EVENT_ID::JOURNAL_REPLAY_STARTED);
    POS_TRACE_INFO(eventId, "Journal replay started");

    int result = _ExecuteReplayTasks();
    if (result < 0)
    {
        POS_TRACE_CRITICAL((int)POS_EVENT_ID::JOURNAL_REPLAY_FAILED,
            "Journal replay failed");
    }
    replayState.RemoveRecoverState();

    eventId = static_cast<int>(POS_EVENT_ID::JOURNAL_REPLAY_COMPLETED);
    POS_TRACE_INFO(eventId, "Journal replay completed");

    return result;
}

int
ReplayHandler::_ExecuteReplayTasks(void)
{
    int result = 0;
    for (auto task : taskList)
    {
        reporter->TaskStarted(task->GetId(), task->GetNumSubTasks());
        result = task->Start();
        reporter->TaskCompleted(task->GetId());

        if (result != 0)
        {
            if (result > 0)
            {
                reporter->CompleteAll();
            }
            return result;
        }
    }
    return result;
}

} // namespace pos

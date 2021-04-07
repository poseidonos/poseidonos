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

#include "../log_buffer/journal_log_buffer.h"
#include "flush_metadata.h"
#include "flush_pending_stripes.h"
#include "read_log_buffer.h"
#include "replay_logs.h"
#include "reset_log_buffer.h"
#include "src/allocator/allocator.h"
#include "src/array/array.h"
#include "src/mapper/mapper.h"

namespace ibofos
{
ReplayHandler::ReplayHandler(void)
: logBuffer(nullptr)
{
    reporter = new ReplayProgressReporter();

    mapper = MapperSingleton::Instance();
    allocator = AllocatorSingleton::Instance();
    array = ArraySingleton::Instance();
}

void
ReplayHandler::Init(JournalLogBuffer* journalLogBuffer)
{
    logBuffer = journalLogBuffer;
    _InitializeTaskList();
}

void
ReplayHandler::_InitializeTaskList(void)
{
    _AddTask(new ReadLogBuffer(logBuffer, logList, reporter));
    _AddTask(new ReplayLogs(logList, mapper, allocator, array, reporter, pendingWbStripes));
    _AddTask(new FlushMetadata(mapper, allocator, reporter));
    _AddTask(new ResetLogBuffer(logBuffer, reporter));
    _AddTask(new FlushPendingStripes(pendingWbStripes, allocator, reporter));
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

void
ReplayHandler::SetMapperToUse(Mapper* mapperToUse)
{
    mapper = mapperToUse;
}

void
ReplayHandler::SetAllocatorToUse(Allocator* allocatorToUse)
{
    allocator = allocatorToUse;
}

void
ReplayHandler::SetArrayToUse(Array* arrayToUse)
{
    array = arrayToUse;
}

int
ReplayHandler::Start(void)
{
    replayState.GetRecoverState();

    int result = _ExecuteReplayTasks();
    if (result < 0)
    {
        IBOF_TRACE_CRITICAL((int)IBOF_EVENT_ID::JOURNAL_REPLAY_FAILED,
            "Journal replay failed");
    }
    replayState.RemoveRecoverState();

    return result;
}

int
ReplayHandler::_ExecuteReplayTasks(void)
{
    int result = 0;
    for (auto task : taskList)
    {
        int eventId = (int)IBOF_EVENT_ID::JOURNAL_REPLAY_STATUS;
        IBOF_TRACE_INFO(eventId, "Start replay task {}", task->GetId());

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

} // namespace ibofos

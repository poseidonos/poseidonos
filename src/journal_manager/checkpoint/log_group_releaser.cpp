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

#include "log_group_releaser.h"

#include "../log_buffer/journal_log_buffer.h"
#include "../log_write/buffer_write_done_notifier.h"
#include "../log_write/log_write_handler.h"
#include "checkpoint_handler.h"
#include "dirty_map_manager.h"
#include "src/include/ibof_event_id.h"
#include "src/logger/logger.h"
#include "src/mapper/mapper.h"
#include "src/scheduler/event_argument.h"

namespace ibofos
{
LogGroupReleaser::LogGroupReleaser(void)
: releaseNotifier(nullptr),
  logBuffer(nullptr),
  logWriteHandler(nullptr),
  dirtyPageManager(nullptr),
  flushingLogGroupId(-1)
{
    checkpointHandler = new CheckpointHandler(this);
}

LogGroupReleaser::~LogGroupReleaser(void)
{
    delete checkpointHandler;
}

void
LogGroupReleaser::Init(LogBufferWriteDoneNotifier* released, JournalLogBuffer* buffer,
    LogWriteHandler* writeHandler, DirtyMapManager* dirtyPage)
{
    releaseNotifier = released;
    logBuffer = buffer;
    logWriteHandler = writeHandler;
    dirtyPageManager = dirtyPage;
}

void
LogGroupReleaser::Reset(void)
{
    flushingLogGroupId = -1;
    fullLogGroup.clear();
}

void
LogGroupReleaser::AddToFullLogGroup(int gorupId)
{
    _AddToFullLogGroupList(gorupId);
    _FlushNextLogGroup();
}

void
LogGroupReleaser::_AddToFullLogGroupList(int groupId)
{
    std::unique_lock<std::mutex> lock(fullLogGroupLock);
    fullLogGroup.push_back(groupId);
}

void
LogGroupReleaser::_FlushNextLogGroup(void)
{
    if ((flushingLogGroupId == -1) && (_HasFullLogGroup()))
    {
        IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::JOURNAL_FLUSH_LOG_GROUP,
            "Flush next log group");

        StartCheckpoint();
    }
}

bool
LogGroupReleaser::_HasFullLogGroup(void)
{
    std::unique_lock<std::mutex> lock(flushLock);
    return (fullLogGroup.size() != 0);
}

int
LogGroupReleaser::StartCheckpoint(void)
{
    _UpdateFlushingLogGroup();
    assert(flushingLogGroupId != -1);

    MapPageList dirtyPages = dirtyPageManager->GetDirtyList(flushingLogGroupId);
    IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::JOURNAL_CHECKPOINT_STARTED,
        "Checkpoint started for log group {}", flushingLogGroupId);
    int ret = checkpointHandler->Start(dirtyPages);
    if (ret != 0)
    {
        // TODO(huijeong.kim): Go to the fail mode - not to journal any more
    }
    return 0;
}

void
LogGroupReleaser::_UpdateFlushingLogGroup(void)
{
    std::unique_lock<std::mutex> lock(flushLock);
    flushingLogGroupId = _PopFullLogGroup();
}

int
LogGroupReleaser::_PopFullLogGroup(void)
{
    std::unique_lock<std::mutex> lock(fullLogGroupLock);

    assert(fullLogGroup.size() != 0);
    int retLogGroup = fullLogGroup.front();
    fullLogGroup.pop_front();

    return retLogGroup;
}

void
LogGroupReleaser::CheckpointCompleted(void)
{
    assert(flushingLogGroupId != -1);
    int ret = logBuffer->AsyncReset(flushingLogGroupId,
        std::bind(&LogGroupReleaser::_LogGroupResetCompleted,
            this, std::placeholders::_1));

    if (ret != 0)
    {
        // TODO(huijeong.kim) add log
    }
}

int
LogGroupReleaser::GetNumFullLogGroups(void)
{
    if (flushingLogGroupId != -1)
    {
        return fullLogGroup.size() + 1;
    }
    else
    {
        return fullLogGroup.size();
    }
}

int
LogGroupReleaser::GetFlushingLogGroupId(void)
{
    std::unique_lock<std::mutex> lock(flushLock);
    return flushingLogGroupId;
}

void
LogGroupReleaser::_LogGroupResetCompleted(int logGroupId)
{
    releaseNotifier->NotifyLogBufferReseted(logGroupId);

    _ResetFlushingLogGroup();
    _FlushNextLogGroup();

    logWriteHandler->StartWaitingIos();
}

void
LogGroupReleaser::_ResetFlushingLogGroup(void)
{
    std::unique_lock<std::mutex> lock(flushLock);
    flushingLogGroupId = -1;
}

} // namespace ibofos

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

#include "log_group_releaser.h"

#include "src/event_scheduler/event_scheduler.h"
#include "src/include/pos_event_id.h"
#include "src/journal_manager/checkpoint/checkpoint_manager.h"
#include "src/journal_manager/checkpoint/checkpoint_submission.h"
#include "src/journal_manager/log_buffer/buffer_write_done_notifier.h"
#include "src/journal_manager/log_buffer/journal_log_buffer.h"
#include "src/journal_manager/log_buffer/log_group_footer_write_context.h"
#include "src/journal_manager/log_buffer/log_group_footer_write_event.h"
#include "src/journal_manager/log_buffer/log_group_reset_completed_event.h"
#include "src/journal_manager/log_buffer/reset_log_group.h"
#include "src/journal_manager/log_write/log_write_handler.h"
#include "src/logger/logger.h"

namespace pos
{
LogGroupReleaser::LogGroupReleaser(void)
: config(nullptr),
  releaseNotifier(nullptr),
  logBuffer(nullptr),
  flushingLogGroupId(-1),
  flushingSequenceNumber(0),
  checkpointTriggerInProgress(false),
  checkpointManager(nullptr),
  contextManager(nullptr),
  eventScheduler(nullptr)
{
}

LogGroupReleaser::~LogGroupReleaser(void)
{
}

void
LogGroupReleaser::Init(JournalConfiguration* journalConfiguration,
    LogBufferWriteDoneNotifier* released, IJournalLogBuffer* buffer, CheckpointManager* cpManager,
    IMapFlush* mapFlush, IContextManager* ctxManager, EventScheduler* scheduler)
{
    config = journalConfiguration;
    releaseNotifier = released;
    logBuffer = buffer;
    checkpointManager = cpManager;
    contextManager = ctxManager;
    eventScheduler = scheduler;
}

void
LogGroupReleaser::Reset(void)
{
    flushingLogGroupId = -1;
    fullLogGroup.clear();
}

void
LogGroupReleaser::AddToFullLogGroup(struct LogGroupInfo logGroupInfo)
{
    _AddToFullLogGroupList(logGroupInfo);
    _FlushNextLogGroup();
}

void
LogGroupReleaser::_AddToFullLogGroupList(struct LogGroupInfo logGroupInfo)
{
    std::unique_lock<std::mutex> lock(fullLogGroupLock);
    fullLogGroup.push_back(logGroupInfo);
}

void
LogGroupReleaser::_FlushNextLogGroup(void)
{
    if ((flushingLogGroupId == -1) && (_HasFullLogGroup()))
    {
        if (checkpointTriggerInProgress.exchange(true) == false)
        {
            _UpdateFlushingLogGroup();
            assert(flushingLogGroupId != -1);
            checkpointTriggerInProgress = false;

            _TriggerCheckpoint();
        }
    }
}

void
LogGroupReleaser::_TriggerCheckpoint(void)
{
    LogGroupFooter footer;
    uint64_t footerOffset;

    POS_TRACE_DEBUG(EID(JOURNAL_CHECKPOINT_STARTED),
        "Submit checkpoint start for log group {}", flushingLogGroupId);

    _CreateFlushingLogGroupFooter(footer, footerOffset);

    EventSmartPtr checkpointSubmission = _CreateCheckpointSubmissionEvent();

    EventSmartPtr event(new LogGroupFooterWriteEvent(logBuffer, footer, footerOffset, flushingLogGroupId, checkpointSubmission));
    eventScheduler->EnqueueEvent(event);
}

EventSmartPtr
LogGroupReleaser::_CreateCheckpointSubmissionEvent(void)
{
    // Checkpoint will be in this sequence:
    // LogGroupFooterWriteEvent -> CheckpointSubmission -> ResetLogGroup -> LogGroupResetCompletion
    // TODO (huijeong.kim) to use Callback class instead of Event
    LogGroupFooter footer;
    uint64_t footerOffset;
    _CreateFlushingLogGroupFooter(footer, footerOffset);
    footer.isReseted = true;
    footer.resetedSequenceNumber = flushingSequenceNumber;

    EventSmartPtr resetLogGroupCompletion(new LogGroupResetCompletedEvent(this, flushingLogGroupId));
    EventSmartPtr resetLogGroup(new ResetLogGroup(logBuffer, flushingLogGroupId, footer, footerOffset, resetLogGroupCompletion));
    EventSmartPtr checkpointSubmission(new CheckpointSubmission(checkpointManager, resetLogGroup, flushingLogGroupId));

    return checkpointSubmission;
}

void
LogGroupReleaser::_CreateFlushingLogGroupFooter(LogGroupFooter& footer, uint64_t& footerOffset)
{
    LogGroupLayout layout = config->GetLogBufferLayout(flushingLogGroupId);
    uint64_t version = contextManager->GetStoredContextVersion(SEGMENT_CTX);

    footer.lastCheckpointedSeginfoVersion = version;
    footer.isReseted = false;
    footer.resetedSequenceNumber = UINT32_MAX;
    footerOffset = layout.footerStartOffset;
}

bool
LogGroupReleaser::_HasFullLogGroup(void)
{
    std::unique_lock<std::mutex> lock(fullLogGroupLock);
    return (fullLogGroup.size() != 0);
}

void
LogGroupReleaser::_UpdateFlushingLogGroup(void)
{
    struct LogGroupInfo result = _PopFullLogGroup();
    flushingLogGroupId = result.logGroupId;
    flushingSequenceNumber = result.sequenceNumber;
    POS_TRACE_DEBUG(EID(JOURNAL_FLUSH_LOG_GROUP),
        "Flush next log group {}, seq number {}", flushingLogGroupId, flushingSequenceNumber);
}

struct LogGroupInfo
LogGroupReleaser::_PopFullLogGroup(void)
{
    std::unique_lock<std::mutex> lock(fullLogGroupLock);

    assert(fullLogGroup.size() != 0);
    int retLogGroup = fullLogGroup.front().logGroupId;
    uint32_t retSequenceNumber = fullLogGroup.front().sequenceNumber;
    fullLogGroup.pop_front();

    return {retLogGroup, retSequenceNumber};
}

void
LogGroupReleaser::LogGroupResetCompleted(int logGroupId)
{
    releaseNotifier->NotifyLogBufferReseted(logGroupId);

    _ResetFlushingLogGroup();
    _FlushNextLogGroup();
}

void
LogGroupReleaser::_ResetFlushingLogGroup(void)
{
    flushingLogGroupId = -1;
}

int
LogGroupReleaser::GetFlushingLogGroupId(void)
{
    return flushingLogGroupId;
}

std::list<int>
LogGroupReleaser::GetFullLogGroups(void)
{
    std::list<int> result;
    for (auto it: fullLogGroup)
    {
        result.push_back(it.logGroupId);
    }
    return result;
}

CheckpointStatus
LogGroupReleaser::GetStatus(void)
{
    return checkpointManager->GetStatus();
}

} // namespace pos

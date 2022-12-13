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
  nextLogGroupId(0),
  checkpointManager(nullptr),
  contextManager(nullptr),
  eventScheduler(nullptr)
{
}

LogGroupReleaser::~LogGroupReleaser(void)
{
    logGroups.clear();
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

    for (int groupId = 0; groupId < config->GetNumLogGroups(); groupId++)
    {
        LogGroupReleaseStatus info(groupId);
        logGroups.push_back(info);
    }
}

void
LogGroupReleaser::Reset(void)
{
    nextLogGroupId = 0;
    for (auto group : logGroups)
    {
        group.Reset();
    }
}

void
LogGroupReleaser::MarkLogGroupFull(int logGroupId, uint32_t sequenceNumber)
{
    assert((uint32_t)logGroupId < logGroups.size());
    logGroups[logGroupId].SetWaiting(sequenceNumber);

    _FlushNextLogGroup();
}

void
LogGroupReleaser::_FlushNextLogGroup(void)
{
    std::unique_lock<std::mutex> lock(flushTriggerLock);

    if (_IsFlushInProgress() == false)
    {
        if (logGroups[nextLogGroupId].IsFull() == true)
        {
            logGroups[nextLogGroupId].SetReleasing();

            POS_TRACE_INFO(EID(JOURNAL_RELEASE_LOG_GROUP_STARTED),
                "logGroupId:{}, sequenceNumber:{}", nextLogGroupId, logGroups[nextLogGroupId].GetSeqNum());

            _TriggerCheckpoint();
        }
    }
}
bool
LogGroupReleaser::_IsFlushInProgress(void)
{
    return (logGroups[nextLogGroupId].IsReleasing());
}

void
LogGroupReleaser::_TriggerCheckpoint(void)
{
    POS_TRACE_INFO(EID(JOURNAL_CHECKPOINT_STARTED),
        "logGroupId:{}", nextLogGroupId);

    EventSmartPtr event = _CreateCheckpointSubmissionEvent();
    eventScheduler->EnqueueEvent(event);
}

EventSmartPtr
LogGroupReleaser::_CreateCheckpointSubmissionEvent(void)
{
    // Checkpoint will be in this sequence:
    // LogGroupFooterWriteEvent -> CheckpointSubmission -> ResetLogGroup -> LogGroupResetCompletion
    // TODO (huijeong.kim) to use Callback class instead of Event

    LogGroupLayout layout = config->GetLogBufferLayout(nextLogGroupId);
    uint64_t footerOffset = layout.footerStartOffset;

    LogGroupFooter footerBeforeMapFlush = _CreateLogGroupFooter(nextLogGroupId);
    LogGroupFooter footerAfterMapFlush = _CreateLogGroupFooterForReset(nextLogGroupId, footerBeforeMapFlush);

    EventSmartPtr resetLogGroupCompletion(new LogGroupResetCompletedEvent(this, nextLogGroupId));
    EventSmartPtr resetLogGroup(new ResetLogGroup(logBuffer, nextLogGroupId, footerAfterMapFlush, footerOffset, resetLogGroupCompletion));
    EventSmartPtr checkpointSubmission(new CheckpointSubmission(checkpointManager, resetLogGroup, nextLogGroupId));
    EventSmartPtr event(new LogGroupFooterWriteEvent(logBuffer, footerBeforeMapFlush, footerOffset, nextLogGroupId, checkpointSubmission));

    return event;
}

LogGroupFooter
LogGroupReleaser::_CreateLogGroupFooter(int logGroupId)
{
    LogGroupFooter footer;

    footer.lastCheckpointedSeginfoVersion = contextManager->GetStoredContextVersion(SEGMENT_CTX);
    footer.isReseted = false;
    footer.resetedSequenceNumber = logGroups[logGroupId].GetPrevSeqNum();

    return footer;
}

LogGroupFooter
LogGroupReleaser::_CreateLogGroupFooterForReset(int logGroupId, LogGroupFooter& prevFooter)
{
    LogGroupFooter footer = prevFooter;
    footer.isReseted = true;
    footer.resetedSequenceNumber = logGroups[logGroupId].GetSeqNum();

    return footer;
}

void
LogGroupReleaser::LogGroupResetCompleted(int logGroupId)
{
    POS_TRACE_INFO(EID(JOURNAL_RELEASE_LOG_GROUP_COMPLETED),
        "logGroupId:{}", logGroupId);

    releaseNotifier->NotifyLogBufferReseted(logGroupId);

    logGroups[nextLogGroupId].Reset();
    nextLogGroupId = (nextLogGroupId + 1) % config->GetNumLogGroups();

    _FlushNextLogGroup();
}

int
LogGroupReleaser::GetFlushingLogGroupId(void)
{
    return nextLogGroupId;
}

std::list<int>
LogGroupReleaser::GetFullLogGroups(void)
{
    std::list<int> result;
    for (auto group : logGroups)
    {
        if (group.IsFull())
        {
            result.push_back(group.GetId());
        }
    }
    return result;
}

CheckpointStatus
LogGroupReleaser::GetStatus(void)
{
    return checkpointManager->GetStatus();
}

} // namespace pos

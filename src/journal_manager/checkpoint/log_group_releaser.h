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

#pragma once

#include <atomic>
#include <list>
#include <mutex>

#include "src/include/smart_ptr_type.h"
#include "src/journal_manager/log/log_group_footer.h"
#include "src/journal_manager/log_buffer/i_log_group_reset_completed.h"
#include "src/journal_manager/status/i_checkpoint_status.h"

namespace pos
{
class JournalConfiguration;
class IJournalLogBuffer;
class CheckpointManager;
class LogBufferWriteDoneNotifier;
class EventScheduler;

class IMapFlush;
class IContextManager;

struct LogGroupInfo
{
    int logGroupId;
    uint32_t sequenceNumber;

    inline bool
    operator==(LogGroupInfo input) const
    {
        return (input.logGroupId == logGroupId && input.sequenceNumber == sequenceNumber);
    }
};

class LogGroupReleaser : public ICheckpointStatus, public ILogGroupResetCompleted
{
public:
    LogGroupReleaser(void);
    virtual ~LogGroupReleaser(void);

    virtual void Init(JournalConfiguration* config,
        LogBufferWriteDoneNotifier* notified, IJournalLogBuffer* logBuffer, CheckpointManager* cpManager,
        IMapFlush* mapFlush, IContextManager* contextManager, EventScheduler* scheduler);
    void Reset(void);

    virtual void AddToFullLogGroup(struct LogGroupInfo logGroupInfo);

    virtual int GetFlushingLogGroupId(void) override;
    virtual std::list<int> GetFullLogGroups(void) override;
    virtual CheckpointStatus GetStatus(void) override;

    virtual void LogGroupResetCompleted(int logGroupId) override;

protected:
    void _AddToFullLogGroupList(struct LogGroupInfo logGroupInfo);
    bool _HasFullLogGroup(void);

    virtual void _FlushNextLogGroup(void);
    void _UpdateFlushingLogGroup(void);
    LogGroupInfo _PopFullLogGroup(void);
    virtual void _TriggerCheckpoint(void);

    void _ResetFlushingLogGroup(void);

    void _CreateFlushingLogGroupFooter(LogGroupFooter& footer, uint64_t& footerOffset);

    EventSmartPtr _CreateCheckpointSubmissionEvent(void);

    JournalConfiguration* config;
    LogBufferWriteDoneNotifier* releaseNotifier;
    IJournalLogBuffer* logBuffer;

    std::mutex fullLogGroupLock;
    std::list<LogGroupInfo> fullLogGroup;

    std::atomic<int> flushingLogGroupId;
    std::atomic<uint32_t> flushingSequenceNumber;

    std::atomic<bool> checkpointTriggerInProgress;
    CheckpointManager* checkpointManager;

    IContextManager* contextManager;
    EventScheduler* eventScheduler;
};

} // namespace pos

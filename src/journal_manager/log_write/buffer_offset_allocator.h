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

#include <mutex>
#include <vector>

#include "log_group_buffer_status.h"
#include "src/journal_manager/log_buffer/buffer_write_done_notifier.h"
#include "src/journal_manager/status/i_log_buffer_status.h"

namespace pos
{
class JournalConfiguration;
class LogGroupReleaser;

class BufferOffsetAllocator : public LogBufferWriteDoneEvent, public ILogBufferStatus
{
public:
    BufferOffsetAllocator(void);
    virtual ~BufferOffsetAllocator(void);

    virtual void Init(LogGroupReleaser* releaser, JournalConfiguration* journalConfiguration);
    // For UT
    void Init(LogGroupReleaser* logGroupReleaser,
        JournalConfiguration* journalConfiguration, std::vector<LogGroupBufferStatus*> LogBufferstatusList);

    virtual void Dispose(void);

    void Reset(void);

    virtual int AllocateBuffer(uint32_t logSize, uint64_t& allocatedOffset);
    virtual void LogWriteCanceled(int logGroupId);

    virtual void LogFilled(int logGroupId, MapList& dirty) override;
    virtual void LogBufferReseted(int logGroupId) override;

    uint64_t GetNumLogsAdded(void);
    uint64_t GetNextOffset(void);

    virtual LogGroupStatus GetBufferStatus(int logGroupId) override;
    virtual uint32_t GetSequenceNumber(int logGroupId) override;

    virtual int GetLogGroupId(uint64_t fileOffset);

private:
    int _GetNewActiveGroup(void);
    uint32_t _GetNextSeqNum(void);
    void _TryToSetFull(int logGroupId);

    JournalConfiguration* config;
    LogGroupReleaser* releaser;

    std::mutex allocateLock;
    std::vector<LogGroupBufferStatus*> statusList;

    uint32_t nextSeqNumber;
    int currentLogGroupId;
};
} // namespace pos

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

#include "../log/waiting_log_list.h"
#include "../log_buffer/buffer_write_done_notifier.h"
#include "src/meta_file_intf/async_context.h"
#include "src/journal_manager/log_buffer/i_journal_log_buffer.h"
#include "src/metafs/lib/concurrent_metafs_time_interval.h"

namespace pos
{
class BufferOffsetAllocator;
class LogWriteContext;
class IJournalLogBuffer;
class JournalConfiguration;
class LogWriteStatistics;
class TelemetryPublisher;

class LogWriteHandler : public LogBufferWriteDoneEvent
{
public:
    LogWriteHandler(void);
    LogWriteHandler(LogWriteStatistics* statistics, WaitingLogList* waitingList);
    virtual ~LogWriteHandler(void);

    virtual void Init(BufferOffsetAllocator* allocator, IJournalLogBuffer* buffer,
        JournalConfiguration* config, TelemetryPublisher* telemetryPublisher,
        ConcurrentMetaFsTimeInterval* timeInterval = nullptr);
    virtual void Dispose(void);

    virtual int AddLog(LogWriteContext* context);
    virtual void AddLogToWaitingList(LogWriteContext* context);
    void LogWriteDone(AsyncMetaFileIoCtx* ctx);

    virtual void LogFilled(int logGroupId, MapList& dirty) override;
    virtual void LogBufferReseted(int logGroupId) override;

private:
    void _StartWaitingIos(void);
    void _PublishPeriodicMetrics(LogWriteContext* context);

    IJournalLogBuffer* logBuffer;
    BufferOffsetAllocator* bufferAllocator;

    LogWriteStatistics* logWriteStats;
    WaitingLogList* waitingList;

    std::atomic<uint64_t> numIosRequested;
    std::atomic<uint64_t> numIosCompleted;

    TelemetryPublisher* telemetryPublisher;
    ConcurrentMetaFsTimeInterval* interval;
    std::atomic<uint64_t> sumOfTimeSpentPerInterval;
    std::atomic<uint64_t> doneCountPerInterval;
};

} // namespace pos

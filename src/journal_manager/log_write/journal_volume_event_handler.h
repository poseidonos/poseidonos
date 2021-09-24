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

#pragma once

#include <condition_variable>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "src/allocator/i_context_manager.h"
#include "src/journal_manager/checkpoint/meta_flush_completed.h"
#include "src/journal_manager/log_write/i_journal_volume_event_handler.h"

namespace pos
{
class EventScheduler;
class LogWriteContextFactory;
class CheckpointManager;
class LogWriteHandler;
class JournalConfiguration;

class JournalVolumeEventHandler : public IJournalVolumeEventHandler, public IMetaFlushCompleted
{
public:
    JournalVolumeEventHandler(void);
    virtual ~JournalVolumeEventHandler(void);

    virtual void Init(LogWriteContextFactory* logFactory,
        CheckpointManager* cpManager,
        LogWriteHandler* logWritter, JournalConfiguration* journalConfiguration,
        IContextManager* contextManager, EventScheduler* scheduler);

    virtual int WriteVolumeDeletedLog(int volId) override;
    virtual int TriggerMetadataFlush(void) override;

    virtual void MetaFlushed(void) override;

    virtual void VolumeDeletedLogWriteDone(int volumeId);

private:
    int _WriteVolumeDeletedLog(int volumeId, uint64_t segCtxVersion);
    void _WaitForLogWriteDone(int volumeId);

    int _FlushAllocatorContext(void);
    void _WaitForAllocatorContextFlushCompleted(void);

    bool isInitialized;

    IContextManager* contextManager;
    EventScheduler* eventScheduler;

    JournalConfiguration* config;
    LogWriteContextFactory* logFactory;
    CheckpointManager* checkpointManager;
    LogWriteHandler* logWriteHandler;

    std::mutex logWriteMutex;
    std::condition_variable logWriteCondVar;
    bool logWriteInProgress;

    std::mutex flushMutex;
    std::condition_variable flushCondVar;
    bool flushInProgress;
};

} // namespace pos

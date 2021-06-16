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

#include "journal_volume_event_handler.h"

#include <functional>

#include "src/event_scheduler/event_scheduler.h"
#include "src/include/pos_event_id.h"
#include "src/journal_manager/checkpoint/checkpoint_manager.h"
#include "src/journal_manager/checkpoint/checkpoint_submission.h"
#include "src/journal_manager/checkpoint/meta_flush_completed.h"
#include "src/journal_manager/config/journal_configuration.h"
#include "src/journal_manager/log_buffer/journal_log_buffer.h"
#include "src/journal_manager/log_buffer/log_write_context_factory.h"
#include "src/journal_manager/log_write/log_write_handler.h"
#include "src/journal_manager/log_write/volume_deleted_log_write_callback.h"
#include "src/logger/logger.h"

namespace pos
{
JournalVolumeEventHandler::JournalVolumeEventHandler(void)
: isInitialized(false),
  contextManager(nullptr),
  eventScheduler(nullptr),
  config(nullptr),
  logFactory(nullptr),
  checkpointManager(nullptr),
  logWriteHandler(nullptr),
  logWriteInProgress(false),
  flushInProgress(false)
{
}

JournalVolumeEventHandler::~JournalVolumeEventHandler(void)
{
}

void
JournalVolumeEventHandler::Init(LogWriteContextFactory* factory,
    CheckpointManager* cpManager, LogWriteHandler* writter,
    JournalConfiguration* journalConfiguration,
    IContextManager* contextManagerToUse, EventScheduler* scheduler)
{
    config = journalConfiguration;
    logFactory = factory;
    checkpointManager = cpManager;
    logWriteHandler = writter;

    contextManager = contextManagerToUse;
    eventScheduler = scheduler;

    isInitialized = true;
}

int
JournalVolumeEventHandler::VolumeDeleted(int volumeId)
{
    if (isInitialized == false || config->IsEnabled() == false)
    {
        return 0;
    }
    else
    {
        POS_TRACE_DEBUG(POS_EVENT_ID::JOURNAL_HANDLE_VOLUME_DELETION,
            "Start volume delete event handler (volume id {})", volumeId);

        int ret = 0;
        ret = _WriteVolumeDeletedLog(volumeId, contextManager->GetStoredContextVersion(SEGMENT_CTX));
        if (ret != 0)
        {
            POS_TRACE_DEBUG(POS_EVENT_ID::JOURNAL_HANDLE_VOLUME_DELETION,
                "Writing volume deleted log failed (volume id {})", volumeId);
            return ret;
        }
        _WaitForLogWriteDone(volumeId);

        EventSmartPtr callback(new MetaFlushCompleted(this));
        EventSmartPtr cpSubmission(new CheckpointSubmission(checkpointManager, callback));

        flushInProgress = true;
        eventScheduler->EnqueueEvent(cpSubmission);

        _WaitForAllocatorContextFlushCompleted();

        return 0;
    }
}

int
JournalVolumeEventHandler::_WriteVolumeDeletedLog(int volumeId, uint64_t segCtxVersion)
{
    POS_TRACE_DEBUG(POS_EVENT_ID::JOURNAL_HANDLE_VOLUME_DELETION,
        "Write volume deleted log, segInfo version is {}", segCtxVersion);
    EventSmartPtr callback(new VolumeDeletedLogWriteCallback(this, volumeId));

    LogWriteContext* logWriteContext =
        logFactory->CreateVolumeDeletedLogWriteContext(volumeId, segCtxVersion, callback);

    logWriteInProgress = true;
    return logWriteHandler->AddLog(logWriteContext);
}

void
JournalVolumeEventHandler::_WaitForLogWriteDone(int volumeId)
{
    std::unique_lock<std::mutex> lock(logWriteMutex);
    logWriteCondVar.wait(lock, [&] {
        return (logWriteInProgress == false);
    });
}

void
JournalVolumeEventHandler::_WaitForAllocatorContextFlushCompleted(void)
{
    std::unique_lock<std::mutex> lock(flushMutex);
    flushCondVar.wait(lock, [&] {
        return (flushInProgress == false);
    });
}

void
JournalVolumeEventHandler::VolumeDeletedLogWriteDone(int volumeId)
{
    std::unique_lock<std::mutex> lock(logWriteMutex);
    logWriteInProgress = false;
    logWriteCondVar.notify_one();
}

void
JournalVolumeEventHandler::MetaFlushed(void)
{
    std::unique_lock<std::mutex> lock(flushMutex);
    flushInProgress = false;
    flushCondVar.notify_one();
}
} // namespace pos

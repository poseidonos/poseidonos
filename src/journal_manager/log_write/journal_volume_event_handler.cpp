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

#include "src/include/pos_event_id.h"
#include "src/journal_manager/checkpoint/dirty_map_manager.h"
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
  config(nullptr),
  logFactory(nullptr),
  dirtyPageManager(nullptr),
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
    DirtyMapManager* dirtyPages, LogWriteHandler* writter,
    JournalConfiguration* journalConfiguration, IContextManager* contextManagerToUse)
{
    config = journalConfiguration;
    logFactory = factory;
    dirtyPageManager = dirtyPages;
    logWriteHandler = writter;

    contextManager = contextManagerToUse;

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

        // TODO (huijeong.kim) need to flush map also
        ret = _FlushAllocatorContext();
        if (ret != 0)
        {
            POS_TRACE_DEBUG(POS_EVENT_ID::JOURNAL_HANDLE_VOLUME_DELETION,
                "Failed to flush allocator context");
            return ret;
        }
        _WaitForAllocatorContextFlushCompleted();

        dirtyPageManager->DeleteDirtyList(volumeId);

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

int
JournalVolumeEventHandler::_FlushAllocatorContext(void)
{
    EventSmartPtr callback(new AllocatorContextFlushCompleted(this));

    flushInProgress = true;
    return contextManager->FlushContextsAsync(callback);
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
JournalVolumeEventHandler::AllocatorContextFlushed(void)
{
    std::unique_lock<std::mutex> lock(flushMutex);
    flushInProgress = false;
    flushCondVar.notify_one();
}
} // namespace pos

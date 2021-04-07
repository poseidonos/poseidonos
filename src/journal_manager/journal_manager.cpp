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

#include "journal_manager.h"

#include <iostream>

#include "checkpoint/dirty_map_manager.h"
#include "checkpoint/log_group_releaser.h"
#include "journal_configuration.h"
#include "log_buffer/journal_log_buffer.h"
#include "log_buffer/log_write_context_factory.h"
#include "log_write/buffer_offset_allocator.h"
#include "log_write/buffer_write_done_notifier.h"
#include "log_write/journal_volume_event_handler.h"
#include "log_write/log_write_handler.h"
#include "replay/replay_handler.h"
#include "src/include/ibof_event_id.h"
#include "src/logger/logger.h"

namespace ibofos
{
JournalManager::JournalManager(void)
: journalStatus(JOURNAL_INVALID)
{
    config = new JournalConfiguration();

    logFactory = new LogWriteContextFactory();
    logWriteHandler = new LogWriteHandler();
    volumeEventHandler = new JournalVolumeEventHandler();

    logBuffer = new JournalLogBuffer();
    bufferAllocator = new BufferOffsetAllocator();
    logGroupReleaser = new LogGroupReleaser();

    dirtyMapManager = new DirtyMapManager();
    logFilledNotifier = new LogBufferWriteDoneNotifier();

    replayHandler = new ReplayHandler();
}

JournalManager::~JournalManager(void)
{
    delete replayHandler;

    delete logFilledNotifier;
    delete dirtyMapManager;

    delete logGroupReleaser;
    delete bufferAllocator;

    delete logWriteHandler;
    delete volumeEventHandler;

    delete logBuffer;

    delete logFactory;

    delete config;
}

int
JournalManager::Init(void)
{
    if (config->IsEnabled() == false)
    {
        IBOF_TRACE_INFO(EID(JOURNAL_DISABLED), "Journal is disabled");
        return 0;
    }
    else
    {
        IBOF_TRACE_INFO(EID(JOURNAL_ENABLED), "Journal is enabled");
    }

    int ret = logBuffer->Setup();
    if (ret < 0)
    {
        return ret;
    }

    _InitModules();

    journalStatus = JOURNAL_INIT;

    if (logBuffer->IsLoaded() == false)
    {
        int ret = Reset();
        if (ret < 0)
        {
            return ret;
        }
        journalStatus = JOURNALING;
    }
    else
    {
        journalStatus = WAITING_TO_BE_REPLAYED;
        IBOF_TRACE_INFO(EID(JOURNAL_LOG_BUFFER_LOADED), "Journal log buffer is loaded");
    }

    return 0;
}

int
JournalManager::DoRecovery(void)
{
    if (config->IsEnabled() == false || journalStatus == JOURNALING)
    {
        return 0;
    }

    if (journalStatus == JOURNAL_INVALID)
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::JOURNAL_MANAGER_NOT_INITIALIZED,
            "Journal manager accessed without initialization");
        return -EID(JOURNAL_REPLAY_FAILED);
    }

    if (logBuffer->IsLoaded() == true)
    {
        journalStatus = REPLAYING_JOURNAL;

        IBOF_TRACE_INFO(EID(JOURNAL_REPLAY_STARTED), "Journal replay started");

        int result = replayHandler->Start();
        if (result < 0)
        {
            journalStatus = JOURNAL_BROKEN;
            return -EID(JOURNAL_REPLAY_FAILED);
        }

        _ResetModules();
        journalStatus = JOURNALING;
    }

    return 0;
}

bool
JournalManager::AddBlockMapUpdatedLog(VolumeIoSmartPtr volumeIo,
    MpageList dirty, EventSmartPtr callbackEvent)
{
    bool executionSuccessful = true;

    if (config->IsEnabled() == true)
    {
        if (journalStatus != JOURNALING)
        {
            return false;
        }

        LogWriteContext* logWriteContext =
            logFactory->CreateBlockMapLogWriteContext(volumeIo, dirty, callbackEvent);
        executionSuccessful = (logWriteHandler->AddLog(logWriteContext) == 0);
    }
    else
    {
        executionSuccessful = callbackEvent->Execute();
    }

    return executionSuccessful;
}

bool
JournalManager::AddStripeMapUpdatedLog(Stripe* stripe, StripeAddr oldAddr,
    MpageList dirty, EventSmartPtr callbackEvent)
{
    bool executionSuccessful = true;

    if (config->IsEnabled() == true)
    {
        if (journalStatus != JOURNALING)
        {
            return false;
        }

        LogWriteContext* logWriteContext =
            logFactory->CreateStripeMapLogWriteContext(stripe, oldAddr, dirty, callbackEvent);
        executionSuccessful = (logWriteHandler->AddLog(logWriteContext) == 0);
    }
    else
    {
        executionSuccessful = callbackEvent->Execute();
    }

    return executionSuccessful;
}

int
JournalManager::Reset(void)
{
    if (journalStatus != JOURNAL_INVALID)
    {
        _ResetModules();

        int ret = logBuffer->SyncResetAll();
        return ret;
    }

    return 0;
}

void
JournalManager::_InitModules(void)
{
    bufferAllocator->Init(logBuffer->GetNumLogGroups(),
        logBuffer->GetLogGroupSize(), logGroupReleaser);
    dirtyMapManager->Init(logBuffer->GetNumLogGroups());

    logFactory->Init(logFilledNotifier);

    logFilledNotifier->Register(bufferAllocator);
    logFilledNotifier->Register(dirtyMapManager);

    logGroupReleaser->Init(logFilledNotifier, logBuffer, logWriteHandler, dirtyMapManager);

    logWriteHandler->Init(bufferAllocator, logBuffer);
    volumeEventHandler->Init(logFactory, dirtyMapManager, logWriteHandler, config);

    replayHandler->Init(logBuffer);
}

void
JournalManager::_ResetModules(void)
{
    bufferAllocator->Reset();
    logGroupReleaser->Reset();
}

} // namespace ibofos

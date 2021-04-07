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

#include "journal_write_context.h"

#include "../log_write/buffer_write_done_notifier.h"
#include "src/include/ibof_event_id.h"
#include "src/logger/logger.h"
#include "src/scheduler/event_argument.h"

namespace ibofos
{
LogWriteContext::LogWriteContext(LogHandlerInterface* log)
: log(log),
  logGroupId(INVALID_LOG_INDEX)
{
}

LogWriteContext::~LogWriteContext(void)
{
    delete log;
}

LogHandlerInterface*
LogWriteContext::GetLog(void)
{
    return log;
}

void
LogWriteContext::SetLogGroupId(int id)
{
    logGroupId = id;
}

int
LogWriteContext::GetLogGroupId(void)
{
    return logGroupId;
}

MapUpdateLogWriteContext::MapUpdateLogWriteContext(LogHandlerInterface* log,
    MapPageList dirtyList, EventSmartPtr callback, LogBufferWriteDoneNotifier* target)
: LogWriteContext(log),
  logFilledNotifier(target),
  dirty(dirtyList),
  callbackEvent(callback)
{
}

MapPageList&
MapUpdateLogWriteContext::GetDirtyList(void)
{
    return dirty;
}

void
MapUpdateLogWriteContext::LogWriteDone(void)
{
    logFilledNotifier->NotifyLogFilled(GetLogGroupId(), dirty);

    bool executionSuccessful = callbackEvent->Execute();
    if (executionSuccessful == false)
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::JOURNAL_CALLBACK_FAILED,
            "Failed to execute journal write callback {}", typeid(callbackEvent).name());
        EventArgument::GetEventScheduler()->EnqueueEvent(callbackEvent);
    }
    callbackEvent = nullptr;
}

BlockMapUpdatedLogWriteContext::BlockMapUpdatedLogWriteContext(VolumeIoSmartPtr inputVolumeIo,
    LogHandlerInterface* log, MapPageList dirty, EventSmartPtr callback, LogBufferWriteDoneNotifier* target)
: MapUpdateLogWriteContext(log, dirty, callback, target)
{
    this->volumeIo = inputVolumeIo;
}

StripeMapUpdatedLogWriteContext::StripeMapUpdatedLogWriteContext(LogHandlerInterface* log,
    MapPageList dirty, EventSmartPtr callback, LogBufferWriteDoneNotifier* target)
: MapUpdateLogWriteContext(log, dirty, callback, target)
{
}

VolumeDeletedLogWriteContext::VolumeDeletedLogWriteContext(int inputVolumeId,
    LogHandlerInterface* log, JournalInternalEventCallback callback)
: LogWriteContext(log),
  volumeId(inputVolumeId),
  callerCallback(callback)
{
}

void
VolumeDeletedLogWriteContext::LogWriteDone(void)
{
    callerCallback(volumeId);
}

JournalResetContext::JournalResetContext(int id, JournalInternalEventCallback callback)
: logGroupId(id),
  resetCallback(callback)
{
}

void
JournalResetContext::ResetDone(void)
{
    resetCallback(logGroupId);
}

} // namespace ibofos

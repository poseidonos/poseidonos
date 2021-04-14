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

#include "../log_buffer/buffer_write_done_notifier.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/event_scheduler/event_scheduler.h"

namespace pos
{
LogWriteContext::LogWriteContext(void)
: log(nullptr),
  logGroupId(INVALID_LOG_INDEX)
{
}

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

uint32_t
LogWriteContext::GetLogSize(void)
{
    return log->GetSize();
}

int
LogWriteContext::GetLogGroupId(void)
{
    return logGroupId;
}

void
LogWriteContext::SetAllocated(int groupId, uint32_t seqNum, MetaIoCbPtr cb)
{
    this->logGroupId = groupId;
    this->callback = cb;
    log->SetSeqNum(seqNum);
}

void
LogWriteContext::SetIoRequest(MetaFsIoOpcode op, int fileDescriptor, uint64_t offset)
{
    this->opcode = op;
    this->fd = fileDescriptor;
    this->fileOffset = offset;
    this->length = log->GetSize();
    this->buffer = log->GetData();
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
    bool executionSuccessful = callbackEvent->Execute();
    // BlockMapUpdateCompletion never fails for now
    // If it can fail, need to set callback to notify log filled to this event
    assert(executionSuccessful == true);

    callbackEvent = nullptr;

    // Log filled notify should be after the callback function completed
    logFilledNotifier->NotifyLogFilled(GetLogGroupId(), dirty);
}

BlockMapUpdatedLogWriteContext::BlockMapUpdatedLogWriteContext(LogHandlerInterface* log,
    MapPageList dirty, EventSmartPtr callback, LogBufferWriteDoneNotifier* target)
: MapUpdateLogWriteContext(log, dirty, callback, target)
{
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

void
JournalResetContext::SetIoRequest(MetaFsIoOpcode op, int fileDescriptor,
    uint64_t offset, uint64_t len, char* buf, MetaIoCbPtr cb)
{
    this->opcode = op;
    this->fd = fileDescriptor;
    this->fileOffset = offset;
    this->length = len;
    this->buffer = buf;
    this->callback = cb;
}

} // namespace pos

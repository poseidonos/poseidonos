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

#include "src/journal_manager/log_buffer/log_write_context.h"

#include "src/journal_manager/log_buffer/buffer_write_done_notifier.h"

namespace pos
{
LogWriteContext::LogWriteContext(void)
: LogBufferIoContext(INVALID_GROUP_ID, nullptr),
  logFilledNotifier(nullptr),
  log(nullptr)
{
}

LogWriteContext::LogWriteContext(EventSmartPtr callback, LogBufferWriteDoneNotifier* notifie)
: LogBufferIoContext(INVALID_GROUP_ID, callback),
  logFilledNotifier(notifie),
  log(nullptr)
{
}

LogWriteContext::LogWriteContext(LogHandlerInterface* log, EventSmartPtr callback,
    LogBufferWriteDoneNotifier* notifier)
: LogBufferIoContext(INVALID_GROUP_ID, callback),
  logFilledNotifier(notifier),
  log(log)
{
    this->opcode = MetaFsIoOpcode::Write;
    this->length = log->GetSize();
    this->buffer = log->GetData();
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

int
LogWriteContext::GetLogGroupId(void)
{
    return logGroupId;
}

void
LogWriteContext::SetBufferAllocated(uint64_t offset, int groupId, uint32_t seqNum)
{
    this->fileOffset = offset;
    this->logGroupId = groupId;

    log->SetSeqNum(seqNum);
}

void
LogWriteContext::IoDone(void)
{
    MapList emptyDirtyList;
    logFilledNotifier->NotifyLogFilled(GetLogGroupId(), emptyDirtyList);

    LogBufferIoContext::IoDone();
}

} // namespace pos

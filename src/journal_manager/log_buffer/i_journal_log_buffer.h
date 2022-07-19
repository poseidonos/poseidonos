/*
 *   BSD LICENSE
 *   Copyright (c) 2022 Samsung Electronics Corporation
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

#include <string>

#include "src/include/smart_ptr_type.h"
#include "src/journal_manager/config/journal_configuration.h"
#include "src/journal_manager/log_buffer/i_log_group_reset_completed.h"
#include "src/meta_file_intf/meta_file_intf.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"

namespace pos
{
class LogWriteContext;
class LogBufferIoContext;
class LogGroupResetContext;
class LogWriteContextFactory;

class IJournalLogBuffer : public ILogGroupResetCompleted
{
public:
    virtual ~IJournalLogBuffer(void)
    {
    }
    virtual int Init(JournalConfiguration* journalConfiguration, LogWriteContextFactory* logWriteContextFactory,
        int arrayId, TelemetryPublisher* tp) = 0;
    virtual void InitDataBuffer(void) = 0;
    virtual void Dispose(void) = 0;

    virtual int Create(uint64_t logBufferSize) = 0;
    virtual int Open(uint64_t& logBufferSize) = 0;

    virtual int ReadLogBuffer(int groupId, void* buffer) = 0;
    virtual int WriteLog(LogWriteContext* context) = 0;

    virtual int SyncResetAll(void) = 0;
    virtual int AsyncReset(int id, EventSmartPtr callbackEvent) = 0;

    virtual int InternalIo(LogBufferIoContext* context) = 0;
    virtual void InternalIoDone(AsyncMetaFileIoCtx* ctx) = 0;

    virtual int Delete(void) = 0;

    virtual void LogGroupResetCompleted(int logGroupId) = 0;

    virtual bool DoesLogFileExist(void) = 0;
};
} // namespace pos

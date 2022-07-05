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

#include "src/include/smart_ptr_type.h"
#include "src/journal_manager/log/log_handler.h"
#include "src/journal_manager/log_buffer/log_buffer_io_context.h"
#include "src/metafs/common/metafs_stopwatch.h"

namespace pos
{
class LogBufferWriteDoneNotifier;

enum class LogStage
{
    Issue,
    Complete,
    Count
};

class LogWriteContext : public LogBufferIoContext
{
public:
    LogWriteContext(void);
    // For UT
    LogWriteContext(EventSmartPtr callback, LogBufferWriteDoneNotifier* notifie);

    LogWriteContext(LogHandlerInterface* log, EventSmartPtr callbackEvent, LogBufferWriteDoneNotifier* notifier);
    virtual ~LogWriteContext(void);

    // This two methods are for log write statistics
    virtual LogHandlerInterface* GetLog(void);
    virtual int GetLogGroupId(void);

    virtual void SetBufferAllocated(uint64_t offset, int groupId, uint32_t seqNum);

    virtual void IoDone(void) override;

    // For UT
    inline LogBufferWriteDoneNotifier*
    GetLogBufferWriteDoneNotifier(void)
    {
        return logFilledNotifier;
    }
    MetaFsStopwatch<LogStage> stopwatch;

protected:
    LogBufferWriteDoneNotifier* logFilledNotifier;

private:
    LogHandlerInterface* log;
};

} // namespace pos

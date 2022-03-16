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
#include <string>

#include "src/include/smart_ptr_type.h"
#include "src/journal_manager/config/journal_configuration.h"
#include "src/journal_manager/log_buffer/i_log_group_reset_completed.h"
#include "src/meta_file_intf/meta_file_intf.h"

namespace pos
{
class LogWriteContext;
class LogBufferIoContext;
class LogGroupResetContext;
class LogWriteContextFactory;

class JournalLogBuffer : public ILogGroupResetCompleted
{
public:
    JournalLogBuffer(void);
    explicit JournalLogBuffer(MetaFileIntf* metaFile);
    virtual ~JournalLogBuffer(void);

    virtual int Init(JournalConfiguration* journalConfiguration, LogWriteContextFactory* logWriteContextFactory, int arrayId);
    virtual void InitDataBuffer(void);
    virtual void Dispose(void);

    virtual int Create(uint64_t logBufferSize);
    virtual int Open(uint64_t& logBufferSize);

    virtual int ReadLogBuffer(int groupId, void* buffer);
    virtual int WriteLog(LogWriteContext* context);

    virtual int SyncResetAll(void);
    virtual int AsyncReset(int id, EventSmartPtr callbackEvent);

    virtual int InternalIo(LogBufferIoContext* context);
    virtual void InternalIoDone(AsyncMetaFileIoCtx* ctx);

    int Delete(void); // TODO(huijeong.kim): move to tester code

    virtual void LogGroupResetCompleted(int logGroupId) override;

    virtual bool DoesLogFileExist(void);
    inline bool
    IsInitialized(void)
    {
        return numInitializedLogGroup == config->GetNumLogGroups();
    }

    // For UT
    char* GetInitializedDataBuffer(void)
    {
        return initializedDataBuffer;
    }
    void SetLogBufferReadDone(bool val)
    {
        logBufferReadDone = val;
    }

private:
    void _LoadBufferSize(void);
    void _LogBufferReadDone(AsyncMetaFileIoCtx* ctx);

    inline uint64_t
    _GetFileOffset(int groupId, uint64_t offset)
    {
        uint64_t groupSize = config->GetLogGroupSize();
        return (groupId * groupSize + offset);
    }

    JournalConfiguration* config;
    LogWriteContextFactory* logFactory;
    std::atomic<int> numInitializedLogGroup;
    std::atomic<bool> logBufferReadDone;
    MetaFileIntf* logFile;

    char* initializedDataBuffer;
};
} // namespace pos

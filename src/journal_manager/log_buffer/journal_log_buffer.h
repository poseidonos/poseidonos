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
#include "src/meta_file_intf/meta_file_intf.h"

#include <string>
#include "../log_buffer/journal_write_context.h"
#include "../config/journal_configuration.h"
namespace pos
{

class JournalLogBuffer
{
public:
    JournalLogBuffer(void);
    explicit JournalLogBuffer(std::string arrayName);
    explicit JournalLogBuffer(MetaFileIntf* metaFile);
    virtual ~JournalLogBuffer(void);

    virtual int Create(uint64_t logBufferSize);
    virtual int Open(uint64_t& logBufferSize);

    virtual int Init(JournalConfiguration* journalConfiguration);
    virtual void Dispose(void);

    int ReadLogBuffer(int groupId, void* buffer);
    int AsyncIO(AsyncMetaFileIoCtx* ctx);

    inline bool
    IsInitialized(void)
    {
        return numInitializedLogGroup == config->GetNumLogGroups();
    }

    virtual int WriteLog(LogWriteContext* context, int logGroupID, uint64_t offset);

    virtual int SyncResetAll(void);
    int AsyncReset(int id, JournalInternalEventCallback callbackFunc);
    void AsyncResetDone(AsyncMetaFileIoCtx* ctx);

    int Delete(void); // TODO(huijeong.kim): move to tester code

    virtual bool DoesLogFileExist(void);

private:
    void _LoadBufferSize(void);
    inline uint64_t
    _GetFileOffset(int groupId, uint64_t offset)
    {
        uint64_t groupSize = config->GetLogGroupSize();
        return (groupId * groupSize + offset);
    }

    void _LogBufferResetCompleted(int logGroupId);

    JournalConfiguration* config;

    std::atomic<int> numInitializedLogGroup;
    MetaFileIntf* logFile;

    char* initializedDataBuffer;
};
} // namespace pos

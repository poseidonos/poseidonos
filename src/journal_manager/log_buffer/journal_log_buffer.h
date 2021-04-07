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
#ifdef IBOF_CONFIG_USE_MOCK_FS
#include "src/meta_file_intf/mock_file_intf.h"
#else
#include "src/metafs/mfs_file_intf.h"
#endif

#include "../log_buffer/journal_write_context.h"

namespace ibofos
{
class JournalLogBuffer
{
public:
    JournalLogBuffer(void);
    virtual ~JournalLogBuffer(void);

    int Setup(void);

    int ReadLogBuffer(int groupId, void* buffer);
    int AsyncIO(AsyncMetaFileIoCtx* ctx);

    inline bool
    IsInitialized(void)
    {
        return numInitializedLogGroup == numLogGroups;
    }
    inline bool
    IsLoaded(void)
    {
        return logBufferLoaded;
    }
    inline uint32_t
    GetLogGroupSize(void)
    {
        return groupSize;
    }
    inline uint32_t
    GetLogBufferSize(void)
    {
        return bufferSize;
    }
    inline int
    GetNumLogGroups(void)
    {
        return numLogGroups;
    }

    int WriteLog(LogWriteContext* context, int logGroupID, int offset);

    int SyncResetAll(void);
    int AsyncReset(int id, JournalInternalEventCallback callbackFunc);
    void AsyncResetDone(AsyncMetaFileIoCtx* ctx);

    int Delete(void); // TODO(huijeong.kim): move to tester code

    void SetSize(uint32_t size);

private:
    void _SetupBufferSize(void);
    void _LoadBufferSize(void);

    uint32_t _DetermineLogBufferSize(void);

    inline uint32_t
    _GetFileOffset(int groupId, uint32_t offset)
    {
        return groupId * groupSize + offset;
    }

    void _LogBufferResetCompleted(int logGroupId);

    const uint32_t INVALID_BUFFER_SIZE = UINT32_MAX;
    const uint32_t DEFAULT_BUFFER_SIZE = 16 * 1024 * 1024;

    uint32_t bufferSize;
    uint32_t groupSize;
    int numLogGroups;

    std::atomic<int> numInitializedLogGroup;
    bool logBufferLoaded = false;
    MetaFileIntf* logFile = nullptr;

    char* initializedDataBuffer;
};
} // namespace ibofos

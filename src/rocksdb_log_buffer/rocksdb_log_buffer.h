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

#include "rocksdb/db.h"
#include "src/journal_manager/log_buffer/i_journal_log_buffer.h"

namespace pos
{
class RocksDBLogBuffer : public IJournalLogBuffer
{
public:
    RocksDBLogBuffer(void);
    explicit RocksDBLogBuffer(const std::string arrayName);
    virtual ~RocksDBLogBuffer(void);

    virtual int Init(JournalConfiguration* journalConfiguration, LogWriteContextFactory* logWriteContextFactory,
        int arrayId, TelemetryPublisher* tp) override;
    virtual void InitDataBuffer(void) override;
    virtual void Dispose(void) override;

    virtual int Create(uint64_t logBufferSize) override;
    virtual int Open(uint64_t& logBufferSize) override;
    virtual int Close(void);

    virtual int ReadLogBuffer(int groupId, void* buffer) override;
    virtual int WriteLog(LogWriteContext* context) override;

    virtual int SyncResetAll(void) override;
    virtual int AsyncReset(int id, EventSmartPtr callbackEvent) override;

    virtual int InternalIo(LogBufferIoContext* context) override;
    virtual void InternalIoDone(AsyncMetaFileIoCtx* ctx) override;

    virtual int Delete(void) override;

    virtual void LogGroupResetCompleted(int logGroupId) override;

    virtual bool DoesLogFileExist(void) override;

    virtual bool IsOpened(void);

    virtual std::string GetPathName(void)
    {
        return pathName;
    }

protected:
    virtual int _CreateDirectory(void);
    virtual int _DeleteDirectory(void);

    std::string pathName;
    bool isOpened;

private:
    inline uint64_t
    _GetFileOffset(int groupId, uint64_t offset)
    {
        uint64_t groupSize = config->GetLogGroupSize();
        return (groupId * groupSize + offset);
    }

    inline std::string
    _MakeRocksDbKey(int groupId, uint64_t offset)
    {
        int numDigits = std::to_string(config->GetLogBufferSize()).size();
        offset = offset % config->GetLogGroupSize();
        int zeroSize = numDigits - std::to_string(offset).size();
        std::string appendZero(zeroSize, '0');
        std::string key = std::to_string(groupId) + appendZero + std::to_string(offset) + "Log";
        return key;
    }

    JournalConfiguration* config;
    LogWriteContextFactory* logFactory;
    rocksdb::DB* rocksJournal;
    uint64_t logBufferSize;
    std::string arrayName;

    TelemetryPublisher* telemetryPublisher;
    std::string basePathName;
};

} // namespace pos

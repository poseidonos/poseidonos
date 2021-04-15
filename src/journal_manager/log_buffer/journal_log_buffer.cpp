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

#include "journal_log_buffer.h"

#include <memory>
#include <string>

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

#ifdef IBOF_CONFIG_USE_MOCK_FS
#include "src/meta_file_intf/mock_file_intf.h"
#else
#include "src/metafs/metafs_file_intf.h"
#endif
// TODO (cheolho.kang) remove ifdef after moving tests

namespace pos
{
JournalLogBuffer::JournalLogBuffer(void)
: config(nullptr),
  numInitializedLogGroup(0),
  logBufferLoaded(false),
  logFile(nullptr),
  initializedDataBuffer(nullptr)
{
}

JournalLogBuffer::JournalLogBuffer(std::string arrayName)
: JournalLogBuffer()
{
    logFile = new FILESTORE("JournalLogBuffer", arrayName);
}

JournalLogBuffer::JournalLogBuffer(MetaFileIntf* metaFile)
: JournalLogBuffer()
{
    logFile = metaFile;
}

JournalLogBuffer::~JournalLogBuffer(void)
{
    if (initializedDataBuffer != nullptr)
    {
        delete [] initializedDataBuffer;
    }

    delete logFile;
}

int
JournalLogBuffer::Init(JournalConfiguration* journalConfiguration)
{
    int ret = 0;
    config = journalConfiguration;

    if (logFile->DoesFileExist() == false)
    {
        ret = logFile->Create(config->GetLogBufferSize(), StorageOpt::NVRAM);
        if (ret != 0)
        {
            POS_TRACE_ERROR((int)POS_EVENT_ID::JOURNAL_LOG_BUFFER_CREATE_FAILED,
                "Failed to create log buffer");
            return ret;
        }

        ret = logFile->Open();
        if (ret != 0)
        {
            POS_TRACE_ERROR((int)POS_EVENT_ID::JOURNAL_LOG_BUFFER_OPEN_FAILED,
                "Failed to open log buffer");
            return ret;
        }

        POS_TRACE_INFO(EID(JOURNAL_LOG_BUFFER_CREATED), "Log buffer is created");
    }
    else
    {
        int ret = logFile->Open();
        if (ret != 0)
        {
            POS_TRACE_ERROR((int)POS_EVENT_ID::JOURNAL_LOG_BUFFER_OPEN_FAILED,
                "Failed to open log buffer");
            return ret;
        }
        else
        {
            _LoadBufferSize();
            logBufferLoaded = true;
        }
        ret = (int)POS_EVENT_ID::JOURNAL_LOG_BUFFER_LOADED;
    }

    assert(initializedDataBuffer == nullptr);

    uint64_t groupSize = config->GetLogGroupSize();
    initializedDataBuffer = new char[groupSize];
    memset(initializedDataBuffer, 0xFF, groupSize);
    return ret;
}

void
JournalLogBuffer::Dispose(void)
{
    if (logFile->IsOpened() == true)
    {
        int ret = logFile->Close();
        if (ret != 0)
        {
            POS_TRACE_ERROR((int)POS_EVENT_ID::JOURNAL_LOG_BUFFER_CLOSE_FAILED,
                "Failed to close journal log buffer");
        }
    }

    if (initializedDataBuffer != nullptr)
    {
        delete [] initializedDataBuffer;
        initializedDataBuffer = nullptr;
    }
}

int
JournalLogBuffer::ReadLogBuffer(int groupId, void* buffer)
{
    uint64_t groupSize = config->GetLogGroupSize();

    // TODO(huijeong.kim) change meta_file_intf file offset to uint64_t
    int ret = logFile->IssueIO(MetaFsIoOpcode::Read, _GetFileOffset(groupId, 0),
        groupSize, (char*)buffer);

    if (ret != 0)
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::JOURNAL_LOG_BUFFER_READ_FAILED,
            "Failed to read log buffer");
        return -1 * ((int)POS_EVENT_ID::JOURNAL_LOG_BUFFER_READ_FAILED);
    }

    return ret;
}

void
JournalLogBuffer::_LoadBufferSize(void)
{
    uint64_t bufferSize = logFile->GetFileSize();
    assert(bufferSize != 0);

    config->UpdateLogBufferSize(bufferSize);
}

int
JournalLogBuffer::WriteLog(LogWriteContext* context, int logGroupID, uint64_t offset)
{
    uint64_t fileOffset = logGroupID * config->GetLogGroupSize() + offset;
    context->SetIoRequest(MetaFsIoOpcode::Write, logFile->GetFd(), fileOffset);

    int ret = logFile->AsyncIO(context);

    if (ret != 0)
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::JOURNAL_LOG_WRITE_FAILED,
            "Failed to write journal log");
        ret = -1 * (int)POS_EVENT_ID::JOURNAL_LOG_WRITE_FAILED;
    }

    return ret;
}

int
JournalLogBuffer::SyncResetAll(void)
{
    int ret = 0;
    numInitializedLogGroup = 0;
    int numLogGroups = config->GetNumLogGroups();

    for (int groupId = 0; groupId < numLogGroups; groupId++)
    {
        ret = AsyncReset(groupId, std::bind(&JournalLogBuffer::_LogBufferResetCompleted, this, std::placeholders::_1));
        if (ret != 0)
        {
            POS_TRACE_ERROR((int)POS_EVENT_ID::JOURNAL_LOG_BUFFER_RESET_FAILED,
                "Failed to reset journal log buffer");
            return ret;
        }
    }

    while (!(IsInitialized() == true))
    {
    }

    POS_TRACE_INFO((int)POS_EVENT_ID::JOURNAL_LOG_BUFFER_RESET,
        "Journal log buffer is reset");

    return ret;
}

void
JournalLogBuffer::_LogBufferResetCompleted(int logGroupId)
{
    numInitializedLogGroup++;
}

int
JournalLogBuffer::AsyncReset(int id, JournalInternalEventCallback callbackFunc)
{
    uint64_t groupSize = config->GetLogGroupSize();

    JournalResetContext* resetRequest = new JournalResetContext(id, callbackFunc);
    resetRequest->SetIoRequest(MetaFsIoOpcode::Write, logFile->GetFd(),
        _GetFileOffset(id, 0), groupSize, initializedDataBuffer,
        std::bind(&JournalLogBuffer::AsyncResetDone, this, std::placeholders::_1));

    int ret = logFile->AsyncIO(resetRequest);
    if (ret != 0)
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::JOURNAL_LOG_BUFFER_RESET_FAILED,
            "Failed to reset log buffer");
    }
    return ret;
}

void
JournalLogBuffer::AsyncResetDone(AsyncMetaFileIoCtx* ctx)
{
    JournalResetContext* context = reinterpret_cast<JournalResetContext*>(ctx);
    context->ResetDone();
    delete context;
}

int
JournalLogBuffer::Delete(void)
{
    int ret = 0;
    if (logFile->DoesFileExist() == true)
    {
        ret = logFile->Delete();

        POS_TRACE_DEBUG((int)POS_EVENT_ID::JOURNAL_DEBUG,
            "Journal log buffer is deleted");
    }
    return ret;
}

} // namespace pos

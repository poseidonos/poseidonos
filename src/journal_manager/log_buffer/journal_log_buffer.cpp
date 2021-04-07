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

#include "src/include/ibof_event_id.h"
#include "src/logger/logger.h"

namespace ibofos
{
JournalLogBuffer::JournalLogBuffer(void)
: bufferSize(INVALID_BUFFER_SIZE),
  groupSize(INVALID_BUFFER_SIZE),
  numLogGroups(2),
  numInitializedLogGroup(0),
  initializedDataBuffer(nullptr)
{
    logFile = new FILESTORE("JournalLogBuffer");
}

JournalLogBuffer::~JournalLogBuffer(void)
{
    if (initializedDataBuffer != nullptr)
    {
        delete[] initializedDataBuffer;
    }

    if (logFile->IsOpened() == true)
    {
        int ret = logFile->Close();
        if (ret != 0)
        {
            IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::JOURNAL_LOG_BUFFER_CLOSE_FAILED,
                "Failed to close journal log buffer");
        }
    }

    delete logFile;
}

void
JournalLogBuffer::SetSize(uint32_t size)
{
    bufferSize = size;
    groupSize = bufferSize / numLogGroups;
}

int
JournalLogBuffer::Setup(void)
{
    int ret = 0;

    if (logFile->DoesFileExist() == false)
    {
        _SetupBufferSize();
        ret = logFile->Create(bufferSize, StorageOpt::NVRAM);
        if (ret != 0)
        {
            IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::JOURNAL_LOG_BUFFER_CREATE_FAILED,
                "Failed to create log buffer");
            return ret;
        }

        ret = logFile->Open();
        if (ret != 0)
        {
            IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::JOURNAL_LOG_BUFFER_OPEN_FAILED,
                "Failed to open log buffer");
            return ret;
        }

        IBOF_TRACE_INFO(EID(JOURNAL_LOG_BUFFER_CREATED), "Log buffer is created");
    }
    else
    {
        int ret = logFile->Open();
        if (ret != 0)
        {
            IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::JOURNAL_LOG_BUFFER_OPEN_FAILED,
                "Failed to open log buffer");
            return ret;
        }
        else
        {
            _LoadBufferSize();
            logBufferLoaded = true;
        }
        ret = (int)IBOF_EVENT_ID::JOURNAL_LOG_BUFFER_LOADED;
    }

    assert(initializedDataBuffer == nullptr);

    initializedDataBuffer = new char[groupSize];
    memset(initializedDataBuffer, 0xFF, groupSize);
    return ret;
}

int
JournalLogBuffer::ReadLogBuffer(int groupId, void* buffer)
{
    int ret = logFile->IssueIO(MetaFsIoOpcode::Read, _GetFileOffset(groupId, 0),
        groupSize, (char*)buffer);

    if (ret != 0)
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::JOURNAL_LOG_BUFFER_READ_FAILED,
            "Failed to read log buffer");
        return -1 * ((int)IBOF_EVENT_ID::JOURNAL_LOG_BUFFER_READ_FAILED);
    }

    return ret;
}

void
JournalLogBuffer::_SetupBufferSize(void)
{
    if (bufferSize == INVALID_BUFFER_SIZE)
    {
        bufferSize = _DetermineLogBufferSize();
    }
    groupSize = bufferSize / numLogGroups;
    assert(bufferSize != 0);
}

void
JournalLogBuffer::_LoadBufferSize(void)
{
    bufferSize = logFile->GetFileSize();
    groupSize = bufferSize / numLogGroups;
    assert(bufferSize != 0);
}

uint32_t
JournalLogBuffer::_DetermineLogBufferSize(void)
{
    uint32_t pageSize = 0;
    uint32_t maxSize = 0;

#ifndef IBOF_CONFIG_USE_MOCK_FS
    MetaFilePropertySet prop;
    prop.ioAccPattern = MDFilePropIoAccessPattern::ByteIntensive;
    prop.ioOpType = MDFilePropIoOpType::WriteDominant;
    prop.integrity = MDFilePropIntegrity::Lvl0_Disable;

    pageSize = metaFsMgr.util.EstimateAlignedFileIOSize(prop);
    maxSize = metaFsMgr.util.GetTheBiggestExtentSize(prop);
#endif

    if (pageSize == 0 || maxSize == 0)
    {
        return DEFAULT_BUFFER_SIZE;
    }
    else
    {
        return (maxSize / pageSize - 1) * pageSize;
    }
}

int
JournalLogBuffer::WriteLog(LogWriteContext* context, int logGroupID, int offset)
{
    context->opcode = MetaFsIoOpcode::Write;
    context->fd = logFile->GetFd();
    context->fileOffset = logGroupID * GetLogGroupSize() + offset;
    context->length = context->GetLog()->GetSize();
    context->buffer = context->GetLog()->GetData();

    int ret = logFile->AsyncIO(context);

    if (ret != 0)
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::JOURNAL_LOG_WRITE_FAILED,
            "Failed to write journal log");
        ret = -1 * (int)IBOF_EVENT_ID::JOURNAL_LOG_WRITE_FAILED;
    }

    return ret;
}

int
JournalLogBuffer::SyncResetAll(void)
{
    int ret = 0;
    numInitializedLogGroup = 0;
    for (int groupId = 0; groupId < numLogGroups; groupId++)
    {
        ret = AsyncReset(groupId, std::bind(&JournalLogBuffer::_LogBufferResetCompleted, this, std::placeholders::_1));
        if (ret != 0)
        {
            IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::JOURNAL_LOG_BUFFER_RESET_FAILED,
                "Failed to reset journal log buffer");
            return ret;
        }
    }

    while (!(IsInitialized() == true))
    {
    }

    IBOF_TRACE_INFO((int)IBOF_EVENT_ID::JOURNAL_LOG_BUFFER_RESET,
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
    JournalResetContext* resetRequest = new JournalResetContext(id, callbackFunc);
    resetRequest->opcode = MetaFsIoOpcode::Write;
    resetRequest->fd = logFile->GetFd();
    resetRequest->fileOffset = _GetFileOffset(id, 0);
    resetRequest->length = groupSize;
    resetRequest->buffer = initializedDataBuffer;
    resetRequest->callback = std::bind(&JournalLogBuffer::AsyncResetDone, this, std::placeholders::_1);

    int ret = logFile->AsyncIO(resetRequest);
    if (ret != 0)
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::JOURNAL_LOG_BUFFER_RESET_FAILED,
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

        IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::JOURNAL_DEBUG,
            "Journal log buffer is deleted");
    }
    return ret;
}

} // namespace ibofos

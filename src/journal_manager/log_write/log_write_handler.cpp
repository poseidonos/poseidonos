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

#include "log_write_handler.h"

#include "src/journal_manager/log_buffer/journal_log_buffer.h"
#include "src/journal_manager/log_write/log_write_statistics.h"
#include "src/journal_manager/replay/replay_stripe.h"
#include "buffer_offset_allocator.h"

#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"

namespace pos
{
// Constructor for product code
LogWriteHandler::LogWriteHandler(void)
: logBuffer(nullptr),
  bufferAllocator(nullptr),
  logWriteStats(new LogWriteStatistics()),
  waitingList(new WaitingLogList())
{
}

// Constructor for injecting dependencies in unit tests
LogWriteHandler::LogWriteHandler(LogWriteStatistics* statistics, WaitingLogList* waitingList)
: logBuffer(nullptr),
  bufferAllocator(nullptr),
  logWriteStats(statistics),
  waitingList(waitingList)
{
}

LogWriteHandler::~LogWriteHandler(void)
{
    delete logWriteStats;
    delete waitingList;
}

void
LogWriteHandler::Init(BufferOffsetAllocator* allocator, JournalLogBuffer* buffer,
    JournalConfiguration* journalConfig)
{
    bufferAllocator = allocator;
    logBuffer = buffer;

    if (journalConfig->IsDebugEnabled() == true)
    {
        logWriteStats->Init(journalConfig->GetNumLogGroups());
    }
}

int
LogWriteHandler::AddLog(LogWriteContext* context)
{
    int ret = 0;
    if (waitingList->IsEmpty() == false)
    {
        waitingList->AddToList(context);
    }
    else
    {
        ret = _AddLogInternal(context);
    }
    return ret;
}

int
LogWriteHandler::_AddLogInternal(LogWriteContext* context)
{
    OffsetInFile logOffset;
    int allocationResult =
        bufferAllocator->AllocateBuffer(context->GetLogSize(), logOffset);

    if (allocationResult == 0)
    {
        context->SetAllocated(logOffset.id, logOffset.seqNum,
            std::bind(&LogWriteHandler::LogWriteDone, this, std::placeholders::_1));

        int result = logBuffer->WriteLog(context, logOffset.id, logOffset.offset);
        if (result != 0)
        {
            delete context;
        }
        // TODO(huijeong.kim) move to no-journal mode, if failed
        return result;
    }
    else if (allocationResult > 0)
    {
        waitingList->AddToList(context);
        return 0;
    }
    else
    {
        return allocationResult;
    }
}

void
LogWriteHandler::LogWriteDone(AsyncMetaFileIoCtx* ctx)
{
    LogWriteContext* context = reinterpret_cast<LogWriteContext*>(ctx);

    // Status update should be followed by LogWriteDone callback
    bool statusUpdated = logWriteStats->UpdateStatus(context);
    context->LogWriteDone();

    if (statusUpdated == true)
    {
        logWriteStats->AddToList(context);
    }
    else
    {
        delete context;
    }

    _StartWaitingIos();
}

void
LogWriteHandler::_StartWaitingIos(void)
{
    if (waitingList->IsEmpty() == false)
    {
        auto log = waitingList->GetWaitingIo();
        _AddLogInternal(log);
    }
}

void
LogWriteHandler::LogFilled(int logGroupId, MapPageList& dirty)
{
    // Nothing to do
}

void
LogWriteHandler::LogBufferReseted(int logGroupId)
{
    logWriteStats->PrintStats(logGroupId);

    _StartWaitingIos();
}

} // namespace pos

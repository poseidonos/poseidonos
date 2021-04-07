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

#include "../log_buffer/journal_log_buffer.h"
#include "buffer_offset_allocator.h"
#include "src/include/ibof_event_id.hpp"
#include "src/logger/logger.h"

namespace ibofos
{
LogWriteHandler::LogWriteHandler(void)
: logBuffer(nullptr),
  bufferAllocator(nullptr)
{
}

LogWriteHandler::~LogWriteHandler(void)
{
}

void
LogWriteHandler::Init(BufferOffsetAllocator* allocator, JournalLogBuffer* buffer)
{
    bufferAllocator = allocator;
    logBuffer = buffer;
}

int
LogWriteHandler::AddLog(LogWriteContext* context)
{
    int ret = 0;
    if (waitingList.IsEmpty() == false)
    {
        waitingList.AddToList(context);
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
    OffsetInFile logOffset
        = bufferAllocator->AllocateBuffer(context->GetLog()->GetSize());

    int ret = 0;

    if (logOffset.offset < 0)
    {
        waitingList.AddToList(context);
    }
    else
    {
        context->GetLog()->SetSeqNum(logOffset.seqNum);
        context->SetLogGroupId(logOffset.id);
        context->callback = std::bind(&LogWriteHandler::LogWriteDone, this,
            std::placeholders::_1);

        ret = logBuffer->WriteLog(context, logOffset.id, logOffset.offset);
        if (ret != 0)
        {
            // write log
            IBOF_TRACE_INFO(EID(ADD_TO_JOURNAL_WAITING_LIST),
                "Add journal write request to waiting list");
            // TODO(huijeong.kim) move to no-journal mode
        }
    }

    return ret;
}

void
LogWriteHandler::LogWriteDone(AsyncMetaFileIoCtx* ctx)
{
    LogWriteContext* context = reinterpret_cast<LogWriteContext*>(ctx);

    context->LogWriteDone();
    delete context;

    StartWaitingIos();
}

void
LogWriteHandler::StartWaitingIos(void)
{
    if (waitingList.IsEmpty() == false)
    {
        auto log = waitingList.GetWaitingIo();
        _AddLogInternal(log);
    }
}

} // namespace ibofos

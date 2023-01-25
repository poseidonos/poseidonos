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

#include "src/journal_manager/log_buffer/log_write_context.h"

#include "src/event_scheduler/meta_update_call_back.h"
#include "src/journal_manager/log/log_handler.h"

namespace pos
{
LogWriteContext::LogWriteContext(void)
: log(nullptr),
  logGroupId(INVALID_GROUP_ID),
  callback(nullptr)
{
}

LogWriteContext::LogWriteContext(LogHandlerInterface* inputLog, EventSmartPtr inputCallback)
: LogWriteContext()
{
    log = inputLog;
    callback = inputCallback;
}

LogWriteContext::LogWriteContext(LogHandlerInterface* inputLog, MapList inputMapList, EventSmartPtr inputCallback)
: LogWriteContext()
{
    log = inputLog;
    dirtyMap = inputMapList;
    callback = inputCallback;
}

LogWriteContext::~LogWriteContext(void)
{
    delete log;
}

void
LogWriteContext::SetLogAllocated(int id, uint64_t seqNum)
{
    this->logGroupId = id;
    this->log->SetSeqNum(seqNum);

    MetaUpdateCallback* metaUpdateCb = dynamic_cast<MetaUpdateCallback*>(callback.get());

    if (nullptr != metaUpdateCb)
    {
        metaUpdateCb->SetLogGroupId(logGroupId);
    }
    else
    {
        // log writes that not use MetaUpdateCallback will be skipped intentionally.
        // metaUpdateCb is nullable by design since certain callback (e.g. VolumeDeletedLogWriteCallback)
        // may not want to use VersionedSegmentContext feature (hence, not inheriting MetaUpdateCallback)
    }
}

const MapList&
LogWriteContext::GetDirtyMapList(void)
{
    return dirtyMap;
}

int
LogWriteContext::GetLogGroupId(void)
{
    return logGroupId;
}

uint64_t
LogWriteContext::GetLogSize(void)
{
    return log->GetSize();
}

char*
LogWriteContext::GetBuffer(void)
{
    return log->GetData();
}

EventSmartPtr
LogWriteContext::GetCallback(void)
{
    return callback;
}

LogHandlerInterface*
LogWriteContext::GetLog(void)
{
    return log;
}

} // namespace pos

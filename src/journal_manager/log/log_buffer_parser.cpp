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

#include "log_buffer_parser.h"

#include <unordered_set>

#include "src/include/pos_event_id.h"
#include "src/journal_manager/log/block_write_done_log_handler.h"
#include "src/journal_manager/log/gc_block_write_done_log_handler.h"
#include "src/journal_manager/log/gc_stripe_flushed_log_handler.h"
#include "src/journal_manager/log/stripe_map_updated_log_handler.h"
#include "src/journal_manager/log/volume_deleted_log_handler.h"
#include "src/logger/logger.h"

namespace pos
{
LogBufferParser::ValidMarkFinder::ValidMarkFinder(char* bufferPtr, uint64_t maxOffset)
: buffer(bufferPtr),
  maxOffset(maxOffset)
{
}

bool
LogBufferParser::ValidMarkFinder::GetNextValidMarkOffset(uint64_t startOffset, uint64_t& foundOffset)
{
    uint64_t searchOffset = startOffset;
    while (searchOffset < maxOffset)
    {
        char* ptr = (char*)(buffer + searchOffset);
        uint32_t mark = *(uint32_t*)ptr;
        if (mark == LOG_VALID_MARK || mark == LOG_GROUP_FOOTER_VALID_MARK)
        {
            foundOffset = searchOffset;
            return true;
        }
        searchOffset += sizeof(uint32_t);
    }
    return false;
}

LogBufferParser::LogBufferParser(void)
{
}

LogBufferParser::~LogBufferParser(void)
{
}

int
LogBufferParser::GetLogs(void* buffer, int logGroupId, uint64_t bufferSize, LogList& logs)
{
    ValidMarkFinder finder((char*)buffer, bufferSize);

    bool validMarkFound = false;
    uint64_t searchOffset = 0;
    uint64_t foundOffset = 0;

    while ((validMarkFound = finder.GetNextValidMarkOffset(searchOffset, foundOffset)) == true)
    {
        char* dataPtr = (char*)buffer + foundOffset;
        uint32_t validMark = *(uint32_t*)(dataPtr);

        if (validMark == LOG_VALID_MARK)
        {
            LogHandlerInterface* log = _GetLogHandler(dataPtr);
            if (log == nullptr)
            {
                int event = static_cast<int>(EID(JOURNAL_INVALID_LOG_FOUND));
                POS_TRACE_ERROR(event, "Unknown type of log is found");
                return event * -1;
            }

            logs.AddLog(logGroupId, log);
        }
        else if (validMark == LOG_GROUP_FOOTER_VALID_MARK)
        {
            LogGroupFooter footer = *(LogGroupFooter*)(dataPtr);
            logs.SetLogGroupFooter(logGroupId, footer);
        }
        _GetNextSearchOffset(searchOffset, foundOffset);
    }

    return 0;
}

LogHandlerInterface*
LogBufferParser::_GetLogHandler(char* ptr)
{
    Log* logPtr = reinterpret_cast<Log*>(ptr);
    LogHandlerInterface* foundLog = nullptr;

    if (logPtr->type == LogType::BLOCK_WRITE_DONE)
    {
        foundLog = new BlockWriteDoneLogHandler(*reinterpret_cast<BlockWriteDoneLog*>(ptr));
    }
    else if (logPtr->type == LogType::STRIPE_MAP_UPDATED)
    {
        foundLog = new StripeMapUpdatedLogHandler(*reinterpret_cast<StripeMapUpdatedLog*>(ptr));
    }
    else if (logPtr->type == LogType::GC_BLOCK_WRITE_DONE)
    {
        foundLog = new GcBlockWriteDoneLogHandler(ptr);
    }
    else if (logPtr->type == LogType::GC_STRIPE_FLUSHED)
    {
        foundLog = new GcStripeFlushedLogHandler(*reinterpret_cast<GcStripeFlushedLog*>(ptr));
    }
    else if (logPtr->type == LogType::VOLUME_DELETED)
    {
        foundLog = new VolumeDeletedLogEntry(*reinterpret_cast<VolumeDeletedLog*>(ptr));
    }

    return foundLog;
}

void
LogBufferParser::_GetNextSearchOffset(uint64_t& searchOffset, uint64_t foundOffset)
{
    uint32_t sizeofValidMark = sizeof(uint32_t);
    searchOffset = foundOffset + sizeofValidMark;
}
} // namespace pos

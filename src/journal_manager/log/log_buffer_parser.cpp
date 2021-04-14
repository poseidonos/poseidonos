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

#include "log_buffer_parser.h"

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
LogBufferParser::LogBufferParser(void)
{
}

LogBufferParser::~LogBufferParser(void)
{
}

int
LogBufferParser::GetLogs(void* buffer, uint64_t bufferSize, LogList& logs)
{
    char* logPtr;
    uint64_t currentOffset = 0;
    int currentLogType;
    int result = 0;

    uint64_t numBlockMapUpdatedLogs = 0;
    uint64_t numStripeMapUpdatedLogs = 0;
    uint64_t numVolumeDeletedLogs = 0;

    while ((logPtr = _GetNextValidLogEntry((char*)buffer, currentOffset,
                currentLogType, bufferSize)) != nullptr)
    {
        LogType currentLogTypeCast = static_cast<LogType>(currentLogType);
        if (currentLogTypeCast == LogType::BLOCK_WRITE_DONE)
        {
            BlockWriteDoneLogHandler* log = new BlockWriteDoneLogHandler(*reinterpret_cast<BlockWriteDoneLog*>(logPtr));

            currentOffset += log->GetSize();
            logs.push_back(log);

            numBlockMapUpdatedLogs++;
        }
        else if (currentLogTypeCast == LogType::STRIPE_MAP_UPDATED)
        {
            StripeMapUpdatedLogHandler* log = new StripeMapUpdatedLogHandler(*reinterpret_cast<StripeMapUpdatedLog*>(logPtr));

            currentOffset += log->GetSize();
            logs.push_back(log);

            numStripeMapUpdatedLogs++;
        }
        else if (currentLogTypeCast == LogType::VOLUME_DELETED)
        {
            VolumeDeletedLogEntry* log = new VolumeDeletedLogEntry(*reinterpret_cast<VolumeDeletedLog*>(logPtr));

            currentOffset += log->GetSize();
            logs.push_back(log);

            numVolumeDeletedLogs++;
        }
        else
        {
            int eventId = static_cast<int>(POS_EVENT_ID::JOURNAL_INVALID_LOG_FOUND);
            result = -1 * eventId;
            POS_TRACE_ERROR(eventId, "Unknown type of log is found");
            break;
        }
    }

    int eventId = static_cast<int>(POS_EVENT_ID::JOURNAL_DEBUG);
    POS_TRACE_DEBUG(eventId, "{} block map updated, {} stripe flushed, {} volumes deleted",
        numBlockMapUpdatedLogs, numStripeMapUpdatedLogs, numVolumeDeletedLogs);

    return result;
}

char*
LogBufferParser::_GetNextValidLogEntry(char* buffer, uint64_t& currentOffset,
    int& curLogType, uint64_t bufferSize)
{
    while (currentOffset < bufferSize)
    {
        int* data = (int*)(buffer + currentOffset);
        if (*(uint32_t*)data == VALID_MARK)
        {
            char* logPointer = (char*)(data);
            curLogType = *(int*)(data + 1);
            return logPointer;
        }
        currentOffset += sizeof(int);
    }

    return nullptr;
}

} // namespace pos

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
LogBufferParser::GetLogs(void* buffer, uint64_t bufferSize, LogList& logs)
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

            logs.AddLog(log);
            _LogFound(log);

            searchOffset = foundOffset + sizeof(uint32_t);
        }
        else if (validMark == LOG_GROUP_FOOTER_VALID_MARK)
        {
            LogGroupFooter footer = *(LogGroupFooter*)(dataPtr);
            if (footer.isReseted)
            {
                int event = static_cast<int>(EID(JOURNAL_REPLAY_LOG_PARSE));
                POS_TRACE_INFO(event, "Reseted footer is found. Found logs with SeqNumber ({}) or less will be reseted ", footer.resetedSequenceNumber);
                logs.EraseReplayLogGroup(footer.resetedSequenceNumber);
                _ResetedLogFound(footer.resetedSequenceNumber);
            }
            else
            {
                logs.SetLogGroupFooter(_GetLatestSequenceNumber(), footer);
            }

            if (logsFound.size() > 1)
            {
                int event = static_cast<int>(EID(JOURNAL_INVALID_LOG_FOUND));
                std::string seqNumList = "";
                for (auto it = logsFound.cbegin(); it != logsFound.cend(); it++)
                {
                    seqNumList += (std::to_string(it->first) + ", ");
                }
                POS_TRACE_ERROR(event, "Several sequence numbers are found in single log group, latest checkpointed log group: {}, seqNumSeen List: {}", footer.resetedSequenceNumber, seqNumList);
                return event * -1;
            }
            searchOffset = foundOffset + sizeof(LogGroupFooter);
        }
    }

    _PrintFoundLogTypes();

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
LogBufferParser::_LogFound(LogHandlerInterface* log)
{
    uint32_t seqNumber = log->GetSeqNum();
    LogType type = log->GetType();

    if (logsFound.find(seqNumber) == logsFound.end())
    {
        logsFound[seqNumber].resize((int)LogType::COUNT, 0);
    }
    else
    {
        logsFound[seqNumber][(int)type]++;
    }
}

void
LogBufferParser::_ResetedLogFound(uint32_t seqNumber)
{
    for (auto it = logsFound.cbegin(); it != logsFound.cend();)
    {
        if (it->first <= seqNumber || it->first == LOG_VALID_MARK)
        {
            logsFound.erase(it++);
        }
        else
        {
            it++;
        }
    }
}

uint32_t
LogBufferParser::_GetLatestSequenceNumber(void)
{
    if (logsFound.size() > 1)
    {
        int event = static_cast<int>(EID(JOURNAL_REPLAY_LOG_PARSE));
        POS_TRACE_ERROR(event, "Multiple sequence numbers are found without checkpoint.");
    }

    return logsFound.begin()->first;
}

void
LogBufferParser::_PrintFoundLogTypes(void)
{
    for (auto it = logsFound.cbegin(); it != logsFound.cend(); it++)
    {
        int numBlockMapUpdatedLogs = it->second[(int)LogType::BLOCK_WRITE_DONE];
        int numStripeMapUpdatedLogs = it->second[(int)LogType::STRIPE_MAP_UPDATED];
        int numGcStripeFlushedLogs = it->second[(int)LogType::GC_STRIPE_FLUSHED];
        int numVolumeDeletedLogs = it->second[(int)LogType::VOLUME_DELETED];
        POS_TRACE_DEBUG(EID(JOURNAL_DEBUG),
            "Logs for SeqNum {} found: {} block map, {} stripe map, {} gc stripes, {} volumes deleted",
            it->first, numBlockMapUpdatedLogs, numStripeMapUpdatedLogs, numGcStripeFlushedLogs, numVolumeDeletedLogs);
    }
}
} // namespace pos

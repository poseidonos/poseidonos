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

#include "log_write_handler.h"

#include <string>

#include "buffer_offset_allocator.h"
#include "src/include/pos_event_id.hpp"
#include "src/journal_manager/log_buffer/journal_log_buffer.h"
#include "src/journal_manager/log_buffer/log_write_context.h"
#include "src/journal_manager/log_buffer/log_write_io_context.h"
#include "src/journal_manager/log_write/log_write_statistics.h"
#include "src/journal_manager/replay/replay_stripe.h"
#include "src/logger/logger.h"
#include "src/metadata/block_map_update.h"
#include "src/telemetry/telemetry_client/easy_telemetry_publisher.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"

namespace pos
{
// Constructor for product code
LogWriteHandler::LogWriteHandler(void)
: LogWriteHandler(new LogWriteStatistics(), new WaitingLogList())
{
}

// Constructor for injecting dependencies in unit tests
LogWriteHandler::LogWriteHandler(LogWriteStatistics* statistics, WaitingLogList* waitingList)
: logBuffer(nullptr),
  bufferAllocator(nullptr),
  numLogGroups(0),
  logWriteStats(statistics),
  waitingList(waitingList),
  numIosRequested(nullptr),
  numIosCompleted(nullptr),
  easyTp(nullptr),
  interval(nullptr),
  sumOfTimeSpentPerInterval(0),
  doneCountPerInterval(0)
{
}

LogWriteHandler::~LogWriteHandler(void)
{
    delete logWriteStats;
    delete waitingList;

    if (nullptr != interval)
    {
        delete interval;
    }

    if (nullptr != numIosRequested)
    {
        delete numIosRequested;
    }

    if (nullptr != numIosCompleted)
    {
        delete numIosCompleted;
    }
}

void
LogWriteHandler::Init(BufferOffsetAllocator* allocator, IJournalLogBuffer* buffer,
    JournalConfiguration* journalConfig, EasyTelemetryPublisher* tp,
    ConcurrentMetaFsTimeInterval* timeInterval)
{
    bufferAllocator = allocator;
    logBuffer = buffer;
    easyTp = tp;
    interval = timeInterval;
    numLogGroups = journalConfig->GetNumLogGroups();

    if (journalConfig->IsDebugEnabled() == true)
    {
        logWriteStats->Init(numLogGroups);
    }

    numIosRequested = new std::vector<std::atomic<uint64_t>>(numLogGroups);
    numIosCompleted = new std::vector<std::atomic<uint64_t>>(numLogGroups);
}

void
LogWriteHandler::Dispose(void)
{
    logWriteStats->Dispose();
}

int
LogWriteHandler::AddLog(LogWriteContext* context)
{
    uint64_t allocatedOffset = 0;

    int result = bufferAllocator->AllocateBuffer(context->GetLogSize(), allocatedOffset);

    if (EID(SUCCESS) == result)
    {
        int groupId = bufferAllocator->GetLogGroupId(allocatedOffset);
        assert(groupId < numLogGroups);
        uint32_t seqNum = bufferAllocator->GetSequenceNumber(groupId);

        context->SetLogAllocated(groupId, seqNum);

        result = logBuffer->WriteLog(context, allocatedOffset,
            std::bind(&LogWriteHandler::LogWriteDone, this, std::placeholders::_1));
        if (EID(SUCCESS) == result)
        {
            (*numIosRequested)[groupId]++;
        }
        else
        {
            delete context;

            // This is to cancel the buffer allocation
            POS_TRACE_ERROR(result, "Log write failed due to io error and canceled buffer allocation");
            bufferAllocator->LogWriteCanceled(groupId);
        }
    }

    return result;
}

void
LogWriteHandler::AddLogToWaitingList(LogWriteContext* context)
{
    waitingList->AddToList(context);
}

void
LogWriteHandler::LogWriteDone(AsyncMetaFileIoCtx* ctx)
{
    LogWriteIoContext* ioContext = dynamic_cast<LogWriteIoContext*>(ctx);

    if (ioContext != nullptr)
    {
        LogWriteContext* logWriteContext = ioContext->GetLogWriteContext();

        (*numIosCompleted)[ioContext->GetLogGroupId()]++;

        ioContext->stopwatch.StoreTimestamp(LogStage::Complete);
        _PublishPeriodicMetrics(ioContext);

        bool statusUpdatedToStats = false;

        if (ioContext->GetError() != 0)
        {
            // When log write fails due to error, should log the error and complete write
            POS_TRACE_ERROR(EID(JOURNAL_LOG_WRITE_FAILED),
                "Log write failed due to io error");

            statusUpdatedToStats = false;
        }
        else
        {
            // Status update should be followed by LogWriteDone callback
            statusUpdatedToStats = logWriteStats->UpdateStatus(logWriteContext);
        }

        ioContext->IoDone();

        if (statusUpdatedToStats == true)
        {
            logWriteStats->AddToList(logWriteContext);
        }
        else
        {
            delete logWriteContext;
        }

        delete ioContext;
    }
    else
    {
        POS_TRACE_ERROR(EID(JOURNAL_CRITICAL_ERROR),
            "Cannot complete log write request, {}", ctx->ToString());
    }
    _StartWaitingIos();
}

void
LogWriteHandler::_PublishPeriodicMetrics(LogWriteIoContext* context)
{
    uint64_t elapsedTime = context->stopwatch.GetElapsedInMilli(LogStage::Issue, LogStage::Complete).count();
    uint64_t time = sumOfTimeSpentPerInterval.fetch_and(elapsedTime) + elapsedTime;
    uint64_t count = doneCountPerInterval.fetch_and(1) + 1;

    if (easyTp && interval && interval->CheckInterval())
    {
        sumOfTimeSpentPerInterval = 0;
        doneCountPerInterval = 0;

        for (size_t i = 0; i < numIosRequested->size(); ++i)
        {
            VectorLabels labels;
            labels.push_back({"group_id", std::to_string(i)});
            easyTp->UpdateGauge(TEL36005_JRN_LOG_COUNT, (*numIosRequested)[i], labels);
            easyTp->UpdateGauge(TEL36006_JRN_LOG_DONE_COUNT, (*numIosCompleted)[i], labels);
        }

        if (count)
        {
            easyTp->UpdateGauge(TEL36007_JRN_LOG_WRITE_TIME_AVERAGE, (time / count));
        }
    }
}

void
LogWriteHandler::_StartWaitingIos(void)
{
    auto log = waitingList->GetWaitingIo();
    if (log != nullptr)
    {
        AddLog(log);
    }
}

void
LogWriteHandler::LogFilled(int logGroupId, const MapList& dirty)
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

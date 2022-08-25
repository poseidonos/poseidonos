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
#include "src/journal_manager/log_write/log_write_statistics.h"
#include "src/journal_manager/replay/replay_stripe.h"
#include "src/logger/logger.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"
#include "src/metadata/block_map_update.h"

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
  logWriteStats(statistics),
  waitingList(waitingList),
  telemetryPublisher(nullptr),
  interval(nullptr)
{
    numIosRequested = 0;
    numIosCompleted = 0;
}

LogWriteHandler::~LogWriteHandler(void)
{
    delete logWriteStats;
    delete waitingList;

    if (nullptr != interval)
    {
        delete interval;
    }
}

void
LogWriteHandler::Init(BufferOffsetAllocator* allocator, IJournalLogBuffer* buffer,
    JournalConfiguration* journalConfig, TelemetryPublisher* tp, ConcurrentMetaFsTimeInterval* timeInterval)
{
    bufferAllocator = allocator;
    logBuffer = buffer;
    telemetryPublisher = tp;
    interval = timeInterval;

    if (journalConfig->IsDebugEnabled() == true)
    {
        logWriteStats->Init(journalConfig->GetNumLogGroups());
    }
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

    int result = bufferAllocator->AllocateBuffer(context->GetLength(), allocatedOffset);

    if (EID(SUCCESS) == result)
    {
        int groupId = bufferAllocator->GetLogGroupId(allocatedOffset);
        uint32_t seqNum = bufferAllocator->GetSequenceNumber(groupId);

        context->SetBufferAllocated(allocatedOffset, groupId, seqNum);
        context->SetInternalCallback(std::bind(&LogWriteHandler::LogWriteDone, this, std::placeholders::_1));

        EventSmartPtr metaUpdateEvent = context->GetClientCallback();
        MetaUpdateCallback* metaUpdateCb = dynamic_cast<MetaUpdateCallback*>(metaUpdateEvent.get());

        if (nullptr != metaUpdateCb)
        {
            metaUpdateCb->SetLogGroupId(groupId);
        }
        else
        {
            // log writes that not use MetaUpdateCallback will be skipped intentionally.
            // metaUpdateCb is nullable by design since certain callback (e.g. VolumeDeletedLogWriteCallback)
            // may not want to use VersionedSegmentContext feature (hence, not inheriting MetaUpdateCallback)
        }

        context->stopwatch.StoreTimestamp(LogStage::Issue);
        result = logBuffer->WriteLog(context);
        if (EID(SUCCESS) == result)
        {
            numIosRequested++;
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
    numIosCompleted++;

    LogWriteContext* context = dynamic_cast<LogWriteContext*>(ctx);

    if (context != nullptr)
    {
        context->stopwatch.StoreTimestamp(LogStage::Complete);
        _PublishPeriodicMetrics(context);

        bool statusUpdatedToStats = false;

        if (context->GetError() != 0)
        {
            // When log write fails due to error, should log the error and complete write
            POS_TRACE_ERROR(EID(JOURNAL_LOG_WRITE_FAILED),
                "Log write failed due to io error");

            statusUpdatedToStats = false;
        }
        else
        {
            // Status update should be followed by LogWriteDone callback
            statusUpdatedToStats = logWriteStats->UpdateStatus(context);
        }

        context->IoDone();

        if (statusUpdatedToStats == true)
        {
            logWriteStats->AddToList(context);
        }
        else
        {
            delete context;
        }
    }
    _StartWaitingIos();
}

void
LogWriteHandler::_PublishPeriodicMetrics(LogWriteContext* context)
{
    uint64_t elapsedTime = context->stopwatch.GetElapsedInMilli(LogStage::Issue, LogStage::Complete).count();
    uint64_t time = sumOfTimeSpentPerInterval.fetch_and(elapsedTime) + elapsedTime;
    uint64_t count = doneCountPerInterval.fetch_and(1) + 1;

    if (telemetryPublisher && interval && interval->CheckInterval())
    {
        sumOfTimeSpentPerInterval = 0;
        doneCountPerInterval = 0;

        std::string logGroupId = std::to_string(context->GetLogGroupId());
        POSMetricVector* v = new POSMetricVector();

        POSMetric metricIssueCnt(TEL36005_JRN_LOG_COUNT, POSMetricTypes::MT_GAUGE);
        metricIssueCnt.AddLabel("group_id", logGroupId);
        metricIssueCnt.SetGaugeValue(numIosRequested);
        v->emplace_back(metricIssueCnt);

        POSMetric metricDoneCnt(TEL36006_JRN_LOG_DONE_COUNT, POSMetricTypes::MT_GAUGE);
        metricDoneCnt.AddLabel("group_id", logGroupId);
        metricDoneCnt.SetGaugeValue(numIosCompleted);
        v->emplace_back(metricDoneCnt);

        if (count)
        {
            POSMetric m(TEL36007_JRN_LOG_WRITE_TIME_AVERAGE, POSMetricTypes::MT_GAUGE);
            m.SetGaugeValue(time / count);
            v->emplace_back(m);
        }

        telemetryPublisher->PublishMetricList(v);
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
LogWriteHandler::LogFilled(int logGroupId, MapList& dirty)
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

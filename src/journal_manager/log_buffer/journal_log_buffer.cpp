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
#include "src/journal_manager/log_buffer/journal_log_buffer.h"

#include <memory>
#include <string>

#include "src/include/pos_event_id.h"
#include "src/journal_manager/log_buffer/log_group_reset_completed_event.h"
#include "src/journal_manager/log_buffer/log_group_reset_context.h"
#include "src/journal_manager/log_buffer/log_write_context.h"
#include "src/journal_manager/log_buffer/log_write_context_factory.h"
#include "src/logger/logger.h"
#include "src/metafs/metafs_file_intf.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"
#include "src/metafs/config/metafs_config_manager.h"
#include "src/metafs/include/metafs_service.h"
#include "src/meta_file_intf/rocksdb_metafs_intf.h"
namespace pos
{
JournalLogBuffer::JournalLogBuffer(void)
: config(nullptr),
  logFactory(nullptr),
  numInitializedLogGroup(0),
  logBufferReadDone(0),
  logFile(nullptr),
  initializedDataBuffer(nullptr),
  telemetryPublisher(nullptr),
  rocksDbEnabled(MetaFsServiceSingleton::Instance()->GetConfigManager()->IsRocksdbEnabled())
{
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
        delete[] initializedDataBuffer;
        initializedDataBuffer = nullptr;
    }

    if (logFile != nullptr)
    {
        delete logFile;
        logFile = nullptr;
    }
}

int
JournalLogBuffer::Init(JournalConfiguration* journalConfiguration, LogWriteContextFactory* logWriteContextFactory,
    int arrayId, TelemetryPublisher* tp)
{
    config = journalConfiguration;
    logFactory = logWriteContextFactory;
    telemetryPublisher = tp;

    if (logFile == nullptr)
    {
        // rocksDbEnabled case works when journal configuration "use_rocksdb" is false and metafs configuration "use_rocksdb" is true. 
        // TODO(sang7.park) : but the performance of POS is too low with this case, so have to figure out it.
        if (rocksDbEnabled)
        {
            logFile = new RocksDBMetaFsIntf("JournalLogBuffer", arrayId, MetaFileType::Journal, config->GetMetaVolumeToUse());
            POS_TRACE_INFO(EID(JOURNAL_LOG_BUFFER_INITIATED), "RocksDBMetaFsIntf for JournalLogBuffer has been instantiated with MetaVolumeType {}, this option is not recommended because of low performance. ", config->GetMetaVolumeToUse());
        }
        else
        {
            logFile = new MetaFsFileIntf("JournalLogBuffer", arrayId, MetaFileType::Journal, config->GetMetaVolumeToUse());
            POS_TRACE_INFO(EID(JOURNAL_LOG_BUFFER_INITIATED), "MetaFsFileIntf for JournalLogBuffer has been instantiated with MetaVolumeType {}", config->GetMetaVolumeToUse());
        }
    }
    return 0;
}

void
JournalLogBuffer::InitDataBuffer(void)
{
    assert(initializedDataBuffer == nullptr);

    uint64_t groupSize = config->GetLogGroupSize();

    initializedDataBuffer = new char[groupSize];
    memset(initializedDataBuffer, 0xFF, groupSize);
}

void
JournalLogBuffer::Dispose(void)
{
    if (logFile->IsOpened() == true)
    {
        int ret = logFile->Close();
        if (ret != 0)
        {
            POS_TRACE_ERROR(EID(JOURNAL_LOG_BUFFER_CLOSE_FAILED),
                "Failed to close journal log buffer");
        }
    }

    if (initializedDataBuffer != nullptr)
    {
        delete[] initializedDataBuffer;
        initializedDataBuffer = nullptr;
    }

    if (logFile != nullptr)
    {
        delete logFile;
        logFile = nullptr;
    }
}

int
JournalLogBuffer::Create(uint64_t logBufferSize)
{
    if (logFile->DoesFileExist() == true)
    {
        POS_TRACE_ERROR(EID(JOURNAL_LOG_BUFFER_CREATE_FAILED),
            "Log buffer already exists");
        return -1 * EID(JOURNAL_LOG_BUFFER_CREATE_FAILED);
    }

    int ret = logFile->Create(logBufferSize);
    if (ret != 0)
    {
        POS_TRACE_ERROR(EID(JOURNAL_LOG_BUFFER_CREATE_FAILED),
            "Failed to create log buffer");
        return ret;
    }

    ret = logFile->Open();
    if (ret != 0)
    {
        POS_TRACE_ERROR(EID(JOURNAL_LOG_BUFFER_OPEN_FAILED),
            "Failed to open log buffer");
        return ret;
    }

    POS_TRACE_INFO(EID(JOURNAL_LOG_BUFFER_CREATED), "Log buffer is created");
    return ret;
}

int
JournalLogBuffer::Open(uint64_t& logBufferSize)
{
    if (logFile->DoesFileExist() == false)
    {
        POS_TRACE_ERROR(EID(JOURNAL_LOG_BUFFER_OPEN_FAILED),
            "Log buffer does not exist");
        return (-1 * EID(JOURNAL_LOG_BUFFER_OPEN_FAILED));
    }

    int ret = logFile->Open();
    if (ret != 0)
    {
        POS_TRACE_ERROR(EID(JOURNAL_LOG_BUFFER_OPEN_FAILED),
            "Failed to open log buffer");
        return ret;
    }

    logBufferSize = logFile->GetFileSize();

    POS_TRACE_INFO(EID(JOURNAL_LOG_BUFFER_LOADED),
        "Journal log buffer is loaded");
    return ret;
}

int
JournalLogBuffer::ReadLogBuffer(int groupId, void* buffer)
{
    uint64_t groupSize = config->GetLogGroupSize();

    if (telemetryPublisher)
    {
        POSMetric metric(TEL36004_JRN_LOAD_LOG_GROUP, POSMetricTypes::MT_GAUGE);
        metric.AddLabel("group_id", std::to_string(groupId));
        metric.SetGaugeValue(1);
        telemetryPublisher->PublishMetric(metric);
    }

    AsyncMetaFileIoCtx* logBufferReadReq = new AsyncMetaFileIoCtx();
    logBufferReadReq->opcode = MetaFsIoOpcode::Read;
    logBufferReadReq->fd = logFile->GetFd();
    logBufferReadReq->fileOffset = _GetFileOffset(groupId, 0);
    logBufferReadReq->length = groupSize;
    logBufferReadReq->buffer = (char*)buffer;
    logBufferReadReq->callback = std::bind(&JournalLogBuffer::_LogBufferReadDone,
        this, std::placeholders::_1);

    logBufferReadDone = false;
    int ret = logFile->AsyncIO(logBufferReadReq);
    if (ret != 0)
    {
        POS_TRACE_ERROR(EID(JOURNAL_LOG_BUFFER_READ_FAILED),
            "Failed to read log buffer");
        delete logBufferReadReq;
        return -1 * (EID(JOURNAL_LOG_BUFFER_READ_FAILED));
    }

    while (logBufferReadDone == false)
    {
        usleep(1);
    }

    if (telemetryPublisher)
    {
        POSMetric metric(TEL36004_JRN_LOAD_LOG_GROUP, POSMetricTypes::MT_GAUGE);
        metric.AddLabel("group_id", std::to_string(groupId));
        metric.SetGaugeValue(0);
        telemetryPublisher->PublishMetric(metric);
    }

    return ret;
}

int
JournalLogBuffer::WriteLog(LogWriteContext* context)
{
    context->SetFile(logFile->GetFd());
    int ret = logFile->AsyncIO(context);

    if (ret != 0)
    {
        POS_TRACE_ERROR(EID(JOURNAL_LOG_WRITE_FAILED),
            "Failed to write journal log");
        ret = -1 * EID(JOURNAL_LOG_WRITE_FAILED);
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
        EventSmartPtr callbackEvent(new LogGroupResetCompletedEvent(this, groupId));
        ret = AsyncReset(groupId, callbackEvent);
        if (ret != 0)
        {
            POS_TRACE_ERROR(EID(JOURNAL_LOG_BUFFER_RESET_FAILED),
                "Failed to reset journal log buffer");
            return ret;
        }
    }

    while (!(IsInitialized() == true))
    {
    }

    POS_TRACE_INFO(EID(JOURNAL_LOG_BUFFER_RESET),
        "Journal log buffer is reset");

    return ret;
}

int
JournalLogBuffer::AsyncReset(int id, EventSmartPtr callbackEvent)
{
    if (telemetryPublisher)
    {
        POSMetric metric(TEL36002_JRN_LOG_GROUP_RESET_CNT, POSMetricTypes::MT_COUNT);
        metric.AddLabel("group_id", std::to_string(id));
        metric.SetCountValue(1);
        telemetryPublisher->PublishMetric(metric);
    }

    uint64_t offset = _GetFileOffset(id, 0);
    uint64_t groupSize = config->GetLogGroupSize();
    LogGroupResetContext* resetRequest = logFactory->CreateLogGroupResetContext(offset, id, groupSize, callbackEvent, initializedDataBuffer);

    return InternalIo(resetRequest);
}

int
JournalLogBuffer::InternalIo(LogBufferIoContext* context)
{
    MetaIoCbPtr callbackFunc = std::bind(&JournalLogBuffer::InternalIoDone, this, std::placeholders::_1);
    context->SetInternalCallback(callbackFunc);
    context->SetFile(logFile->GetFd());

    int ret = logFile->AsyncIO(context);
    if (ret != 0)
    {
        POS_TRACE_ERROR(EID(JOURNAL_LOG_BUFFER_RESET_FAILED),
            "Failed to reset log buffer");
    }
    return ret;
}

void
JournalLogBuffer::InternalIoDone(AsyncMetaFileIoCtx* ctx)
{
    LogBufferIoContext* context = dynamic_cast<LogBufferIoContext*>(ctx);
    if (context != nullptr)
    {
        context->IoDone();
        delete context;
    }
}

int
JournalLogBuffer::Delete(void)
{
    int ret = 0;
    if (logFile->DoesFileExist() == true)
    {
        ret = logFile->Delete();

        POS_TRACE_DEBUG(EID(JOURNAL_DEBUG),
            "Journal log buffer is deleted");
    }
    return ret;
}

void
JournalLogBuffer::LogGroupResetCompleted(int logGroupId)
{
    numInitializedLogGroup++;
    if (telemetryPublisher)
    {
        POSMetric metric(TEL36003_JRN_LOG_GROUP_RESET_DONE_CNT, POSMetricTypes::MT_COUNT);
        metric.AddLabel("group_id", std::to_string(logGroupId));
        metric.SetCountValue(1);
        telemetryPublisher->PublishMetric(metric);
    }
}

bool
JournalLogBuffer::DoesLogFileExist(void)
{
    return logFile->DoesFileExist();
}

void
JournalLogBuffer::_LogBufferReadDone(AsyncMetaFileIoCtx* ctx)
{
    logBufferReadDone = true;
    delete ctx;
}

} // namespace pos

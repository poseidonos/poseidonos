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

#include "src/rocksdb_log_buffer/rocksdb_log_buffer.h"

#include <math.h>

#include <experimental/filesystem>
#include <string>

#include "src/event_scheduler/callback.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/journal_manager/log/log_event.h"
#include "src/journal_manager/log_buffer/log_group_reset_completed_event.h"
#include "src/journal_manager/log_buffer/log_group_reset_context.h"
#include "src/journal_manager/log_buffer/log_write_context.h"
#include "src/journal_manager/log_buffer/log_write_context_factory.h"
#include "src/logger/logger.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"

namespace pos
{
RocksDBLogBuffer::RocksDBLogBuffer(void)
: pathName(""),
  isOpened(false),
  config(nullptr),
  logFactory(nullptr),
  rocksJournal(nullptr),
  logBufferSize(0),
  telemetryPublisher(nullptr),
  basePathName("")
{
}

RocksDBLogBuffer::RocksDBLogBuffer(const std::string arrayName)
: RocksDBLogBuffer()
{
    this->arrayName = arrayName;
}

// LCOV_EXCL_START
RocksDBLogBuffer::~RocksDBLogBuffer(void)
{
    if (rocksJournal != nullptr)
    {
        delete rocksJournal;
        rocksJournal = nullptr;
    }
}
// LCOV_EXCL_STOP

int
RocksDBLogBuffer::Init(JournalConfiguration* journalConfiguration, LogWriteContextFactory* logWriteContextFactory,
    int arrayId, TelemetryPublisher* tp)
{
    config = journalConfiguration;
    logFactory = logWriteContextFactory;
    telemetryPublisher = tp;
    basePathName = config->GetRocksdbPath();
    pathName = basePathName + "/" + this->arrayName + "_RocksJournal";
    POS_TRACE_INFO(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_INITIALIZED)), "RocksDB initialized(path : {})", pathName);
    return 0;
}

void
RocksDBLogBuffer::InitDataBuffer(void)
{
    // nothing to do
}

void
RocksDBLogBuffer::Dispose(void)
{
    if (IsOpened())
    {
        Close();
        POS_TRACE_INFO(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_DISPOSED)), "RocksDB Disposed (path : {})", pathName);
    }
    else
    {
        POS_TRACE_WARN(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_DISPOSED)), "RocksDB Disposed RocksDB was not opened (path : {})", pathName);
    }

    if (rocksJournal != nullptr)
    {
        delete rocksJournal;
        rocksJournal = nullptr;
    }
}

int
RocksDBLogBuffer::Create(uint64_t logBufferSize)
{
    int ret = _CreateDirectory();
    if (ret != 0)
    {
        return ret;
    }

    rocksdb::Options options;
    options.create_if_missing = true;
    rocksdb::Status status = rocksdb::DB::Open(options, pathName, &rocksJournal);

    if (status.ok() == true)
    {
        isOpened = true;
        std::string logBufferSizeKey = "logBufferSizeKey";
        rocksdb::Slice value(to_string(logBufferSize));
        rocksdb::Status addBufferSizeStatus = rocksJournal->Put(rocksdb::WriteOptions(), logBufferSizeKey, value);
        if (addBufferSizeStatus.ok())
        {
            POS_TRACE_INFO(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_CREATED)),
                "RocksDB Journal Created with logBufferSize {}, RocksDB buffer size insertion succeed", logBufferSize);
            this->logBufferSize = logBufferSize;
            return static_cast<int>(EID(SUCCESS));
        }
        else
        {
            POS_TRACE_ERROR(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_CREATION_FAILED)),
                "RocksDB Journal creation succeed but insert buffer size failed  (path : {}), RocksDB buffer size insertion status : {}", pathName, addBufferSizeStatus.code());
            return -1 * EID(ROCKSDB_LOG_BUFFER_CREATION_FAILED);
        }
    }
    else
    {
        POS_TRACE_ERROR(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_CREATION_FAILED)), "RocksDB Journal creation failed (path : {}), RocksDB create status : {}", pathName, status.code());
        return -1 * EID(ROCKSDB_LOG_BUFFER_CREATION_FAILED);
    }
}

int
RocksDBLogBuffer::Open(uint64_t& logBufferSize)
{
    // Do not Open RocksDB Twice (Create and Open is also same)
    rocksdb::Options options;
    rocksdb::Status status = rocksdb::DB::Open(options, pathName, &rocksJournal);

    if (status.ok() == true)
    {
        isOpened = true;
        std::string logBufferSizeKey = "logBufferSizeKey";
        std::string logBufferSizeString;
        rocksdb::Status readBufferSizeStatus = rocksJournal->Get(rocksdb::ReadOptions(), logBufferSizeKey, &logBufferSizeString);
        if (readBufferSizeStatus.ok())
        {
            logBufferSize = atoi(logBufferSizeString.c_str());
            this->logBufferSize = logBufferSize;
            POS_TRACE_INFO(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_OPENED)), "RocksDB Journal opened (path : {}) and logBufferSize : {}", pathName, logBufferSize);
            return static_cast<int>(EID(SUCCESS));
        }
        else
        {
            POS_TRACE_ERROR(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_OPEN_FAILED)),
                "RocksDB Journal opened but failed to read logbuffersize (path : {}), readBufferSizeStatus : {}", pathName, readBufferSizeStatus.code());
            return -1 * EID(ROCKSDB_LOG_BUFFER_OPEN_FAILED);
        }
    }
    else
    {
        POS_TRACE_ERROR(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_OPEN_FAILED)), "RocksDB Journal open failed (path : {}), RocksDB open status : {}", pathName, status.code());
        return -1 * EID(ROCKSDB_LOG_BUFFER_OPEN_FAILED);
    }
}

int
RocksDBLogBuffer::Close(void)
{
    isOpened = false;
    if (rocksJournal != nullptr)
    {
        delete rocksJournal;
        rocksJournal = nullptr;
        POS_TRACE_INFO(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_CLOSED)), "RocksDB Journal Closed (path :{}) ", pathName);
    }
    else
    {
        POS_TRACE_WARN(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_CLOSED)), "RocksDB Journal does not exist (path :{}) ", pathName);
    }
    return static_cast<int>(EID(SUCCESS));
}

int
RocksDBLogBuffer::ReadLogBuffer(int groupId, void* buffer)
{
    std::string keyToStart = _MakeRocksDbKey(groupId, 0);
    std::string keyToLimit = _MakeRocksDbKey(groupId + 1, 0);

    rocksdb::Iterator* it = rocksJournal->NewIterator(rocksdb::ReadOptions());
    uint64_t offset = 0;
    for (it->Seek(keyToStart); it->Valid() && it->key().ToString() < keyToLimit; it->Next())
    {
        std::string itValue = it->value().ToString();
        uint64_t size = itValue.size();
        // TODO(sang7.park) : must change this later because logGroupSize is proper in this context rather than logBufferSize.
        if (offset + size > this->logBufferSize)
        {
            POS_TRACE_ERROR(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_READ_LOG_BUFFER_FAILED_WRONG_BUFFER_OFFSET)),
                "RocksDB Read LogBuffer failed, size of read buffer is over logbuffersize (offset + size > logbuffersize) ({} + {} >= {}), logGroupID : {} (path : {})",
                offset, size, this->logBufferSize, groupId, pathName);
            return -1 * EID(ROCKSDB_LOG_BUFFER_READ_LOG_BUFFER_FAILED_WRONG_BUFFER_OFFSET);
        }
        memcpy((void*)((char*)buffer + offset), itValue.c_str(), size);
        offset += size;
    }

    if (it->status().ok())
    {
        POS_TRACE_DEBUG(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_READ_LOG_BUFFER_SUCCEED)),
            "RocksDB Read LogBuffer succeed logGroupID : {} (path : {})", groupId, pathName);
        return 0;
    }
    else
    {
        POS_TRACE_ERROR(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_READ_LOG_BUFFER_FAILED_ROCKSDB_SCAN_FAILED)),
            "RocksDB Read LogBuffer failed logGroupID : {} (path : {})", groupId, pathName);
        return -1 * EID(ROCKSDB_LOG_BUFFER_READ_LOG_BUFFER_FAILED_ROCKSDB_SCAN_FAILED);
    }
}

int
RocksDBLogBuffer::WriteLog(LogWriteContext* context)
{
    int logGroupId = context->GetLogGroupId();
    uint64_t fileOffset = context->fileOffset;
    std::string key = _MakeRocksDbKey(logGroupId, fileOffset);
    POS_TRACE_DEBUG(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_TRY_WRITE_LOG)),
        "RocksDB Key : {} (logGroupId : {}, fileOffset : {}) , Trying to Write Log (path : {})", key, logGroupId, fileOffset, pathName);

    std::string value(context->GetLog()->GetData(), context->GetLog()->GetSize());
    rocksdb::Status ret = rocksJournal->Put(rocksdb::WriteOptions(), key, value);
    if (ret.ok())
    {
        POS_TRACE_DEBUG(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_WRITE_LOG_DONE)), "RocksDB Key : {} insertion succeed", key);
        context->HandleIoComplete(context);
        return 0;
    }
    else
    {
        POS_TRACE_ERROR(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_WRITE_LOG_FAILED)),
            "RocksDB Key : {} (logGroupId : {}, fileOffset : {}) insertion failed by RocksDB, status code : {} (path : {})", key, logGroupId, fileOffset, ret.code(), pathName);
        return -1 * EID(ROCKSDB_LOG_BUFFER_WRITE_LOG_FAILED);
    }
}

int
RocksDBLogBuffer::SyncResetAll(void)
{
    int ret = 0;
    int numLogGroups = config->GetNumLogGroups();

    for (int groupId = 0; groupId < numLogGroups; groupId++)
    {
        EventSmartPtr callbackEvent(new LogGroupResetCompletedEvent(this, groupId));
        ret = AsyncReset(groupId, callbackEvent);
        if (ret != 0)
        {
            return ret;
        }
    }
    return 0;
}

int
RocksDBLogBuffer::AsyncReset(int id, EventSmartPtr callbackEvent)
{
    std::string keyStart = _MakeRocksDbKey(id, 0);
    std::string keyEnd = _MakeRocksDbKey(id + 1, 0);
    rocksdb::ColumnFamilyHandle* cf = rocksJournal->DefaultColumnFamily();
    rocksdb::Slice start(keyStart), end(keyEnd);
    rocksdb::Status ret = rocksJournal->DeleteRange(rocksdb::WriteOptions(), cf, start, end);
    if (ret.ok())
    {
        POS_TRACE_INFO(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_LOG_GROUP_RESET)), "RocksDB logs in logGroupId {} is reset ", id);
        const bool result = callbackEvent->Execute();
        POS_TRACE_DEBUG(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_LOG_GROUP_RESET)), "The result of the event is {}", result);
        return EID(SUCCESS);
    }
    else
    {
        POS_TRACE_ERROR(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_LOG_GROUP_RESET_FAILED)), "RocksDB logs in logGroupId {} is not reset ", id);
        return -1 * EID(ROCKSDB_LOG_BUFFER_LOG_GROUP_RESET_FAILED);
    }
}

int
RocksDBLogBuffer::InternalIo(LogBufferIoContext* context)
{
    int logGroupId = context->GetLogGroupId();
    uint64_t fileOffset = context->fileOffset;
    std::string key = _MakeRocksDbKey(logGroupId, fileOffset);

    std::string value(context->buffer, context->length);
    rocksdb::Status ret = rocksJournal->Put(rocksdb::WriteOptions(), key, value);
    if (ret.ok())
    {
        POS_TRACE_DEBUG(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_WRITE_LOG_DONE)), "RocksDB Key : {} insertion succeed", key);
        InternalIoDone(context);
        return 0;
    }
    else
    {
        POS_TRACE_ERROR(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_INTERNAL_IO_CONTEXT_NOT_EXIST)), "RocksDB Context does not exist (path : {})", pathName);
        return -1 * EID(ROCKSDB_LOG_BUFFER_INTERNAL_IO_CONTEXT_NOT_EXIST);
    }
}

void
RocksDBLogBuffer::InternalIoDone(AsyncMetaFileIoCtx* ctx)
{
    LogBufferIoContext* context = dynamic_cast<LogBufferIoContext*>(ctx);
    if (context != nullptr)
    {
        context->IoDone();
        delete context;
    }
    else
    {
        POS_TRACE_ERROR(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_INTERNAL_IO_DONE_FAILED)), "RocksDB internalIo (logGroupFooter write) done failed (path :{}) ", pathName);
    }
}

int
RocksDBLogBuffer::Delete(void)
{
    int ret = 0;
    if (DoesLogFileExist())
    {
        if (rocksJournal != nullptr)
        {
            POS_TRACE_INFO(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_DELETE_DB)), "RocksDB DB handler deleted (path : {})", pathName);
            delete rocksJournal;
            rocksJournal = nullptr;
        }
        ret = _DeleteDirectory();
        return ret;
    }
    else
    {
        POS_TRACE_WARN(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_DELETE_DB_FAILED_NOT_EXIST)), "RocksDB DB handler deleted failed (db not exists) (path : {})", pathName);
    }
    return 0;
}

void
RocksDBLogBuffer::LogGroupResetCompleted(int logGroupId)
{
    // nothing to do
}

bool
RocksDBLogBuffer::DoesLogFileExist(void)
{
    if (std::experimental::filesystem::exists(pathName))
    {
        POS_TRACE_INFO(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_DIR_EXISTS)), "RocksDB Log File Exists (path :{})", pathName);
        return true;
    }
    else
    {
        POS_TRACE_INFO(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_DIR_EXISTS)), "RocksDB Log File does not exist (path : {})", pathName);
        return false;
    }
}

bool
RocksDBLogBuffer::IsOpened(void)
{
    return isOpened;
}

int
RocksDBLogBuffer::_CreateDirectory(void)
{
    if (!std::experimental::filesystem::exists(basePathName))
    {
        bool ret = std::experimental::filesystem::create_directory(basePathName);
        if (ret != true)
        {
            POS_TRACE_ERROR(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_DIR_CREATION_FAILED)), "RocksDB directory creation failed (path :{}) ", pathName);
            return -1 * EID(ROCKSDB_LOG_BUFFER_DIR_CREATION_FAILED);
        }
    }
    if (std::experimental::filesystem::exists(pathName))
    {
        POS_TRACE_INFO(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_DIR_EXISTS)), "RocksDB directory already exists (path :{}) ", pathName);
        return 0;
    }

    bool ret = std::experimental::filesystem::create_directory(pathName);
    if (ret != true)
    {
        POS_TRACE_ERROR(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_DIR_CREATION_FAILED)), "RocksDB directory creation failed (path :{}) ", pathName);
        return -1 * EID(ROCKSDB_LOG_BUFFER_DIR_CREATION_FAILED);
    }

    POS_TRACE_INFO(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_DIR_CREATED)), "RocksDB directory created (path :{}) ", pathName);
    return 0;
}

// TODO(sang7.park) : This method is supposed to be used when array is removed.
int
RocksDBLogBuffer::_DeleteDirectory(void)
{
    bool ret = std::experimental::filesystem::remove_all(pathName);
    if (ret != true)
    {
        POS_TRACE_ERROR(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_DIR_DELETION_FAILED)), "RocksDB directory does not exists, so deletion failed (path :{}) ", pathName);
        return -1 * EID(ROCKSDB_LOG_BUFFER_DIR_DELETION_FAILED);
    }
    POS_TRACE_INFO(static_cast<int>(EID(ROCKSDB_LOG_BUFFER_DIR_DELETED)), "RocksDB directory deleted (path :{}) ", pathName);
    return 0;
}

} // namespace pos

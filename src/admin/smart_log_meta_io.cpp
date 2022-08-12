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
#include "src/admin/smart_log_meta_io.h"

#include "src/admin/meta_direction.h"
#include "src/admin/smart_log_mgr.h"
#include "src/meta_file_intf/mock_file_intf.h"
#include "src/metafs/metafs_file_intf.h"
#include "src/metafs/include/metafs_service.h"
#include "src/metafs/config/metafs_config_manager.h"
#include "src/meta_file_intf/rocksdb_metafs_intf.h"

namespace pos
{
SmartLogMetaIo::SmartLogMetaIo(uint32_t arrayIndex, SmartLogMgr* smartLogMgr)
: loaded(false),
  smartLogFile(nullptr),
  arrayId(arrayIndex),
  smartLogMgr(smartLogMgr),
  fileIoDone(new MetaIoDoneChecker),
  rocksDbEnabled(MetaFsServiceSingleton::Instance()->GetConfigManager()->IsRocksdbEnabled())
{
    fileName = "SmartLogPage.bin";
    if (rocksDbEnabled)
    {
        smartLogFile = new RocksDBMetaFsIntf(fileName, arrayId, MetaFileType::General);
        POS_TRACE_INFO(EID(SMART_LOG_META_INITIALIZED),
            "RocksDBMetaFsIntf for smartlogfile has been initialized , fileName : {} , arrayId : {} ", fileName, arrayId);
    }
    else
    {
        smartLogFile = new MetaFsFileIntf(fileName, arrayId, MetaFileType::General);
        POS_TRACE_INFO(EID(SMART_LOG_META_INITIALIZED),
            "MetaFsFileIntf for smartlogfile has been initialized , fileName : {} , arrayId : {} ", fileName, arrayId);
    }
}

SmartLogMetaIo::SmartLogMetaIo(uint32_t arrayIndex, SmartLogMgr* smartLogMgr, MetaFileIntf* metaFile, MetaIoDoneChecker* ioDone)
: loaded(false),
  smartLogFile(metaFile),
  arrayId(arrayIndex),
  smartLogMgr(smartLogMgr),
  fileIoDone(ioDone)
{
}

SmartLogMetaIo::~SmartLogMetaIo(void)
{
    if (nullptr != smartLogFile)
    {
        delete smartLogFile;
        smartLogFile = nullptr;
    }

    if (nullptr != fileIoDone)
    {
        delete fileIoDone;
        fileIoDone = nullptr;
    }
}

int
SmartLogMetaIo::Init(void)
{
    int result = 0;

    smartLogMgr->Init();
    if (smartLogMgr->GetSmartLogEnabled() == false)
    {
        return result;
    }

    if (smartLogFile->DoesFileExist())
    {
        do
        {
            _SetCheckerReady();
            result = _LoadLogData();
            if (result)
                break;

            _WaitForCheckerDone();
            result = _CloseFile();
        } while (0);
    }
    else
    {
        result = _CreateFile();
    }

    return result;
}

void
SmartLogMetaIo::Dispose(void)
{
    int result = 0;
    if (smartLogMgr->GetSmartLogEnabled() == false)
    {
        return;
    }

    _SetCheckerReady();
    result = _StoreLogData();
    if (result)
    {
        POS_TRACE_ERROR(EID(MFS_FILE_WRITE_FAILED),
            "Failed to save when unmounting, fileName:{}", fileName);
        return;
    }

    _WaitForCheckerDone();
    result = _CloseFile();
    if (result)
    {
        POS_TRACE_ERROR(EID(MFS_FILE_CLOSE_FAILED),
            "Failed to close when unmounting, fileName:{}", fileName);
    }
}

void
SmartLogMetaIo::_SetCheckerReady(void)
{
    fileIoDone->SetReady();
}

void
SmartLogMetaIo::_SetCheckerDone(void)
{
    fileIoDone->SetDone();
}

void
SmartLogMetaIo::_WaitForCheckerDone(void)
{
    while (!fileIoDone->IsDone())
    {
        usleep(1);
    }
}

int
SmartLogMetaIo::_CreateFile(void)
{
    uint64_t fileSize = MAX_VOLUME_COUNT * sizeof(struct SmartLogEntry);
    int ret = smartLogFile->Create(fileSize);

    if (0 != ret)
    {
        POS_TRACE_ERROR(EID(MFS_FILE_CREATE_FAILED),
            "Map file creation failed, fileName:{}", fileName);
    }
    else
    {
        loaded = true;
    }

    return ret;
}

int
SmartLogMetaIo::_OpenFile(void)
{
    int ret = smartLogFile->Open();
    return ret;
}
int
SmartLogMetaIo::_CloseFile(void)
{
    int ret = smartLogFile->Close();
    if (ret == 0)
    {
        POS_TRACE_INFO(EID(MFS_INFO_MESSAGE), "{} file has been closed", fileName);
    }
    return ret;
}

int
SmartLogMetaIo::_DoMfsOperation(int direction)
{
    ioError = 0;
    bool Isopened = smartLogFile->IsOpened();
    if (!Isopened)
    {
        _OpenFile();
    }
    LogPageFlushIoCtx* logpageFlushReq = new LogPageFlushIoCtx();
    if (direction == FLUSH)
        logpageFlushReq->opcode = MetaFsIoOpcode::Write;
    else
        logpageFlushReq->opcode = MetaFsIoOpcode::Read;
    logpageFlushReq->fd = smartLogFile->GetFd();
    logpageFlushReq->fileOffset = 0;
    logpageFlushReq->length = MAX_VOLUME_COUNT * sizeof(struct SmartLogEntry);
    logpageFlushReq->buffer = (char*)smartLogMgr->GetLogPages(arrayId);
    logpageFlushReq->callback = std::bind(&SmartLogMetaIo::_CompleteSmartLogIo, this, std::placeholders::_1);

    int ret = smartLogFile->AsyncIO(logpageFlushReq);
    if (ret < 0)
    {
        ioError = ret;
    }

    return ioError;
}

int
SmartLogMetaIo::_StoreLogData(void)
{
    return _DoMfsOperation(FLUSH);
}
int
SmartLogMetaIo::_LoadLogData(void)
{
    int ioError = _DoMfsOperation(LOAD);
    return ioError;
}
void
SmartLogMetaIo::_CompleteSmartLogIo(AsyncMetaFileIoCtx* ctx)
{
    LogPageFlushIoCtx* reqCtx = static_cast<LogPageFlushIoCtx*>(ctx);
    if (reqCtx->error != 0)
    {
        ioError = reqCtx->error;
        POS_TRACE_ERROR(EID(MFS_ASYNCIO_ERROR),
            "MFS AsyncIO error, ioError:{}  mpageNum:{}", ioError, reqCtx->mpageNum);
    }
    delete ctx;
    _SetCheckerDone();
}
void
SmartLogMetaIo::DeleteAsyncIoCtx(AsyncMetaFileIoCtx* ctx)
{
    _CompleteSmartLogIo(ctx);
}
void
SmartLogMetaIo::Flush(void)
{
    return;
}

void
SmartLogMetaIo::Shutdown(void)
{
    return;
}

} // namespace pos

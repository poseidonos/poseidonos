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

namespace pos
{
SmartLogMetaIo::SmartLogMetaIo(uint32_t arrayIndex, SmartLogMgr* smartLogMgr)
: loaded(false),
  smartLogFile(nullptr),
  arrayId(arrayIndex),
  smartLogMgr(smartLogMgr)
{
    fileName = "SmartLogPage.bin";
    smartLogFile = new MetaFsFileIntf(fileName, arrayId);
}
SmartLogMetaIo::SmartLogMetaIo(uint32_t arrayIndex, SmartLogMgr* smartLogMgr, MetaFileIntf* metaFile)
: loaded(false),
  smartLogFile(metaFile),
  arrayId(arrayIndex),
  smartLogMgr(smartLogMgr)
{
}
SmartLogMetaIo::~SmartLogMetaIo(void)
{
    if (nullptr != smartLogFile)
    {
        delete smartLogFile;
        smartLogFile = nullptr;
    }
}
int
SmartLogMetaIo::Init(void)
{
    smartLogMgr->Init();

    if (smartLogMgr->GetSmartLogEnabled() == false)
    {
        return 0;
    }
    _CreateSmartLogFile();
    return 0;
}
void
SmartLogMetaIo::Dispose(void)
{
    if (smartLogMgr->GetSmartLogEnabled() == false)
    {
        return;
    }
    _StoreLogData();
    SmartLogMgrSingleton::ResetInstance();
}
int
SmartLogMetaIo::_CreateSmartLogFile(void)
{
    int ret = 0;
    bool result = smartLogFile->DoesFileExist();
    if (result == false)
    {
        uint64_t fileSize = MAX_VOLUME_COUNT * sizeof(struct SmartLogEntry);
        ret = smartLogFile->Create(fileSize);
        if (ret < 0)
        {
            POS_TRACE_ERROR(EID(MFS_FILE_CREATE_FAILED),
                "Map file creation failed, fileName:{}", fileName);
        }
        else if (ret == 0)
        {
            loaded = true;
        }
    }
    else
    {
        _LoadLogData();
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
    _CloseFile();
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
        POS_TRACE_ERROR((int)POS_EVENT_ID::MFS_ASYNCIO_ERROR,
            "MFS AsyncIO error, ioError:{}  mpageNum:{}", ioError, reqCtx->mpageNum);
    }
    delete ctx;
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

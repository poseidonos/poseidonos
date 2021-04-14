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

#ifdef _ADMIN_ENABLED
#include "src/admin/smart_log_mgr.h"

#include <spdk/nvme_spec.h>

#include "meta_direction.h"
namespace pos
{
SmartLogMgr::SmartLogMgr(void)
: loaded(false)
{
    fileName = "SmartLogPage.bin";
}

SmartLogMgr::~SmartLogMgr(void)
{
}

// Meta file Interfaces
void
SmartLogMgr::Init(MetaFileIntf* metaFileIntf)
{
    memset(logPage, 0, MAX_VOLUME_COUNT * sizeof(struct SmartLogEntry));
    smartLogFile = metaFileIntf;
    CreateSmartLogFile();
}
int
SmartLogMgr::CreateSmartLogFile(void)
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
        LoadLogData();
    }
    return ret;
}
int
SmartLogMgr::OpenFile(void)
{
    int ret = smartLogFile->Open();
    return ret;
}
bool
SmartLogMgr::IsFileOpened(void)
{
    return smartLogFile->IsOpened();
}
int
SmartLogMgr::CloseFile(void)
{
    int ret = smartLogFile->Close();
    if (ret == 0)
    {
        POS_TRACE_INFO(EID(MFS_INFO_MESSAGE), "{} file has been closed", fileName);
    }
    return ret;
}
int
SmartLogMgr::DeleteSmartLogFile(void)
{
    int ret = 0;
    ret = smartLogFile->Delete();
    if (ret < 0)
    {
        POS_TRACE_ERROR(EID(MFS_FILE_DELETE_FAILED), "MFS File:{} delete failed",
            fileName);
        return ret;
    }
    delete smartLogFile;
    smartLogFile = nullptr;
    return ret;
}

int
SmartLogMgr::_DoMfsOperation(int direction)
{
    ioError = 0;
    bool Isopened = smartLogFile->IsOpened();
    if (!Isopened)
    {
        OpenFile();
    }
    LogPageFlushIoCtx* logpageFlushReq = new LogPageFlushIoCtx();
    if (direction == FLUSH)
        logpageFlushReq->opcode = MetaFsIoOpcode::Write;
    else
        logpageFlushReq->opcode = MetaFsIoOpcode::Read;
    logpageFlushReq->fd = smartLogFile->GetFd();
    logpageFlushReq->fileOffset = 0;
    logpageFlushReq->length = MAX_VOLUME_COUNT * sizeof(struct SmartLogEntry);
    logpageFlushReq->buffer = (char*)logPage;
    logpageFlushReq->callback = std::bind(&SmartLogMgr::CompleteSmartLogIo, this, std::placeholders::_1);
    int ret = smartLogFile->AsyncIO(logpageFlushReq);
    if (ret < 0)
    {
        // add log for error in mpage flush
        ioError = ret;
    }
    CloseFile();
    return ioError;
}
int
SmartLogMgr::StoreLogData(void)
{
    return _DoMfsOperation(FLUSH);
}
int
SmartLogMgr::LoadLogData(void)
{
    return _DoMfsOperation(LOAD);
}

void
SmartLogMgr::CompleteSmartLogIo(AsyncMetaFileIoCtx* ctx)
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
SmartLogMgr::IncreaseReadCmds(uint32_t volId)
{
    logPage[volId].hostReadCommands = logPage[volId].hostReadCommands + 1;
    return;
}
void
SmartLogMgr::IncreaseWriteCmds(uint32_t volId)
{
    logPage[volId].hostWriteCommands = logPage[volId].hostWriteCommands + 1;
    return;
}
uint64_t
SmartLogMgr::GetWriteCmds(uint32_t volId)
{
    return logPage[volId].hostWriteCommands;
}
uint64_t
SmartLogMgr::GetReadCmds(uint32_t volId)
{
    return logPage[volId].hostReadCommands;
}
void
SmartLogMgr::IncreaseReadBytes(uint64_t blkCnt, uint32_t volId)
{
    logPage[volId].bytesRead = logPage[volId].bytesRead + blkCnt * BLOCK_SIZE_SMART;
    return;
}

void
SmartLogMgr::IncreaseWriteBytes(uint64_t blkCnt, uint32_t volId)
{
    logPage[volId].bytesWritten = logPage[volId].bytesWritten + blkCnt * BLOCK_SIZE_SMART;
    return;
}

uint64_t
SmartLogMgr::GetReadBytes(uint32_t volId)
{
    return logPage[volId].bytesRead;
}

uint64_t
SmartLogMgr::GetWriteBytes(uint32_t volId)
{
    return logPage[volId].bytesWritten;
}

} // namespace pos
#endif

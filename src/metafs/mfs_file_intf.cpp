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

#include "mfs_file_intf.h"

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

namespace ibofos
{
MfsFileIntf::~MfsFileIntf(void)
{
}

int
MfsFileIntf::_Read(int fd, uint32_t fileOffset, uint32_t length, char* buffer)
{
    MetaFsReturnCode<IBOF_EVENT_ID> ioRC;

    if (!metaFsMgr.sys.IsMounted())
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_ERROR_UNMOUNTED,
            "MFS is unmounted");
        return -(int)IBOF_EVENT_ID::MFS_ERROR_UNMOUNTED;
    }

    ioRC = metaFsMgr.io.Read(fd, fileOffset, length, buffer);

    if (!ioRC.IsSuccess())
        return -(int)IBOF_EVENT_ID::MFS_FILE_READ_FAILED;

    return (int)IBOF_EVENT_ID::SUCCESS;
}

int
MfsFileIntf::_Write(int fd, uint32_t fileOffset, uint32_t length, char* buffer)
{
    MetaFsReturnCode<IBOF_EVENT_ID> ioRC;

    if (!metaFsMgr.sys.IsMounted())
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_ERROR_UNMOUNTED,
            "MFS is unmounted");
        return -(int)IBOF_EVENT_ID::MFS_ERROR_UNMOUNTED;
    }

    ioRC = metaFsMgr.io.Write(fd, fileOffset, length, buffer);

    if (!ioRC.IsSuccess())
        return -(int)IBOF_EVENT_ID::MFS_FILE_WRITE_FAILED;

    return (int)IBOF_EVENT_ID::SUCCESS;
}

int
MfsFileIntf::AsyncIO(AsyncMetaFileIoCtx* ctx)
{
    MetaFsReturnCode<IBOF_EVENT_ID> ioRC;

    if (!metaFsMgr.sys.IsMounted())
    {
        IBOF_TRACE_ERROR(EID(MFS_ERROR_UNMOUNTED), "MFS is unmounted");
        return -EID(MFS_ERROR_UNMOUNTED);
    }

    ctx->ioDoneCheckCallback =
        std::bind(&MfsFileIntf::CheckIoDoneStatus, this, std::placeholders::_1);

    MetaFsAioCbCxt* aioCb = new MetaFsAioCbCxt(ctx->opcode, ctx->fd,
        ctx->fileOffset, ctx->length, (void*)ctx->buffer,
        AsEntryPointParam1(&AsyncMetaFileIoCtx::HandleIoComplete, ctx));

    aioCb->SetUserIo(ctx->volumeIo);
    ioRC = metaFsMgr.io.SubmitIO(aioCb);

    if (!ioRC.IsSuccess())
        return -(int)ioRC.sc;

    return EID(SUCCESS);
}

int
MfsFileIntf::CheckIoDoneStatus(void* data)
{
    int error = (int)IBOF_EVENT_ID::SUCCESS;
    MetaFsAioCbCxt* asyncCtx = reinterpret_cast<MetaFsAioCbCxt*>(data);
    if (asyncCtx->CheckIOError())
    {
        error = -(int)IBOF_EVENT_ID::MFS_IO_FAILED_DUE_TO_ERROR;
    }

    delete asyncCtx;
    return error;
}

int
MfsFileIntf::Create(uint64_t fileSize, StorageOpt storageOpt)
{
    MetaFsReturnCode<IBOF_EVENT_ID> mgmtRC;

    MetaFilePropertySet createProp;
    if (storageOpt == StorageOpt::NVRAM)
    {
        createProp.ioAccPattern = MDFilePropIoAccessPattern::ByteIntensive;
        createProp.ioOpType = MDFilePropIoOpType::WriteDominant;
        createProp.integrity = MDFilePropIntegrity::Lvl0_Disable;
    }
    else
    {
        // Use default
    }

    mgmtRC = metaFsMgr.mgmt.Create(fileName, fileSize, createProp);
    if (!mgmtRC.IsSuccess())
    {
        return -(int)IBOF_EVENT_ID::MFS_FILE_CREATE_FAILED;
    }

    size = fileSize;

    return (int)IBOF_EVENT_ID::SUCCESS;
}

int
MfsFileIntf::Open(void)
{
    MetaFsReturnCode<IBOF_EVENT_ID> mgmtRC;
    mgmtRC = metaFsMgr.mgmt.Open(fileName);

    if (!mgmtRC.IsSuccess())
    {
        return -(int)IBOF_EVENT_ID::MFS_FILE_OPEN_FAILED;
    }

    fd = mgmtRC.returnData;
    return MetaFileIntf::Open();
}

int
MfsFileIntf::Close(void)
{
    MetaFsReturnCode<IBOF_EVENT_ID> mgmtRC;
    mgmtRC = metaFsMgr.mgmt.Close(fd);

    if (!mgmtRC.IsSuccess())
    {
        return -(int)IBOF_EVENT_ID::MFS_FILE_CLOSE_FAILED;
    }

    return MetaFileIntf::Close();
}

bool
MfsFileIntf::DoesFileExist(void)
{
    MetaFsReturnCode<IBOF_EVENT_ID> mgmtRC;
    mgmtRC = metaFsMgr.mgmt.CheckFileExist(fileName);

    return mgmtRC.IsSuccess();
}

int
MfsFileIntf::Delete(void)
{
    MetaFsReturnCode<IBOF_EVENT_ID> mgmtRC;
    mgmtRC = metaFsMgr.mgmt.Delete(fileName);

    if (!mgmtRC.IsSuccess())
    {
        return -(int)IBOF_EVENT_ID::MFS_FILE_DELETE_FAILED;
    }

    return (int)IBOF_EVENT_ID::SUCCESS;
}

uint32_t
MfsFileIntf::GetFileSize(void)
{
    return metaFsMgr.util.GetFileSize(fd);
}

} // namespace ibofos

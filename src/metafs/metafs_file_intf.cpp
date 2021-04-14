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

#include "metafs_file_intf.h"

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

namespace pos
{
MetaFsFileIntf::~MetaFsFileIntf(void)
{
}

int
MetaFsFileIntf::_Read(int fd, uint64_t fileOffset, uint64_t length, char* buffer)
{
    MetaFsReturnCode<POS_EVENT_ID> ioRC;

    if (!metaFs.mgmt.IsSystemMounted())
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::MFS_ERROR_UNMOUNTED,
            "MFS is unmounted");
        return -(int)POS_EVENT_ID::MFS_ERROR_UNMOUNTED;
    }

    ioRC = metaFs.io.Read(fd, arrayName, fileOffset, length, buffer);

    if (!ioRC.IsSuccess())
        return -(int)POS_EVENT_ID::MFS_FILE_READ_FAILED;

    return (int)POS_EVENT_ID::SUCCESS;
}

int
MetaFsFileIntf::_Write(int fd, uint64_t fileOffset, uint64_t length, char* buffer)
{
    MetaFsReturnCode<POS_EVENT_ID> ioRC;

    /*if (!metaFs.mgmt.IsSystemMounted())
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::MFS_ERROR_UNMOUNTED,
            "MFS is unmounted");
        return -(int)POS_EVENT_ID::MFS_ERROR_UNMOUNTED;
    }*/

    ioRC = metaFs.io.Write(fd, arrayName, fileOffset, length, buffer);

    if (!ioRC.IsSuccess())
        return -(int)POS_EVENT_ID::MFS_FILE_WRITE_FAILED;

    return (int)POS_EVENT_ID::SUCCESS;
}

int
MetaFsFileIntf::AsyncIO(AsyncMetaFileIoCtx* ctx)
{
    MetaFsReturnCode<POS_EVENT_ID> ioRC;

    if (!metaFs.mgmt.IsSystemMounted())
    {
        POS_TRACE_ERROR(EID(MFS_ERROR_UNMOUNTED), "MFS is unmounted");
        return -EID(MFS_ERROR_UNMOUNTED);
    }

    ctx->ioDoneCheckCallback =
        std::bind(&MetaFsFileIntf::CheckIoDoneStatus, this, std::placeholders::_1);

    MetaFsAioCbCxt* aioCb = new MetaFsAioCbCxt(ctx->opcode, ctx->fd,
        arrayName, ctx->fileOffset, ctx->length, (void*)ctx->buffer,
        AsEntryPointParam1(&AsyncMetaFileIoCtx::HandleIoComplete, ctx));

    ioRC = metaFs.io.SubmitIO(aioCb);

    if (!ioRC.IsSuccess())
        return -(int)ioRC.sc;

    return EID(SUCCESS);
}

int
MetaFsFileIntf::CheckIoDoneStatus(void* data)
{
    int error = (int)POS_EVENT_ID::SUCCESS;
    MetaFsAioCbCxt* asyncCtx = reinterpret_cast<MetaFsAioCbCxt*>(data);
    if (asyncCtx->CheckIOError())
    {
        error = -(int)POS_EVENT_ID::MFS_IO_FAILED_DUE_TO_ERROR;
    }

    delete asyncCtx;
    return error;
}

int
MetaFsFileIntf::Create(uint64_t fileSize, StorageOpt storageOpt)
{
    MetaFsReturnCode<POS_EVENT_ID> mgmtRC;

    MetaFilePropertySet createProp;
    if (storageOpt == StorageOpt::NVRAM)
    {
        createProp.ioAccPattern = MetaFileAccessPattern::ByteIntensive;
        createProp.ioOpType = MetaFileDominant::WriteDominant;
        createProp.integrity = MetaFileIntegrityType::Lvl0_Disable;
    }
    else
    {
        // Use default
    }

    mgmtRC = metaFs.ctrl.CreateVolume(fileName, arrayName, fileSize, createProp);
    if (!mgmtRC.IsSuccess())
    {
        return -(int)POS_EVENT_ID::MFS_FILE_CREATE_FAILED;
    }

    size = fileSize;

    return (int)POS_EVENT_ID::SUCCESS;
}

int
MetaFsFileIntf::Open(void)
{
    MetaFsReturnCode<POS_EVENT_ID> mgmtRC;
    mgmtRC = metaFs.ctrl.Open(fileName, arrayName);

    if (!mgmtRC.IsSuccess())
    {
        return -(int)POS_EVENT_ID::MFS_FILE_OPEN_FAILED;
    }

    fd = mgmtRC.returnData;
    return MetaFileIntf::Open();
}

int
MetaFsFileIntf::Close(void)
{
    MetaFsReturnCode<POS_EVENT_ID> mgmtRC;
    mgmtRC = metaFs.ctrl.Close(fd, arrayName);

    if (!mgmtRC.IsSuccess())
    {
        return -(int)POS_EVENT_ID::MFS_FILE_CLOSE_FAILED;
    }

    return MetaFileIntf::Close();
}

bool
MetaFsFileIntf::DoesFileExist(void)
{
    MetaFsReturnCode<POS_EVENT_ID> mgmtRC;
    mgmtRC = metaFs.ctrl.CheckFileExist(fileName, arrayName);

    return mgmtRC.IsSuccess();
}

int
MetaFsFileIntf::Delete(void)
{
    MetaFsReturnCode<POS_EVENT_ID> mgmtRC;
    mgmtRC = metaFs.ctrl.Delete(fileName, arrayName);

    if (!mgmtRC.IsSuccess())
    {
        return -(int)POS_EVENT_ID::MFS_FILE_DELETE_FAILED;
    }

    return (int)POS_EVENT_ID::SUCCESS;
}

uint64_t
MetaFsFileIntf::GetFileSize(void)
{
    return metaFs.ctrl.GetFileSize(fd, arrayName);
}

} // namespace pos

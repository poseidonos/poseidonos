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

#include "mfs_io_api_fake.h"

MetaFsReturnCode<IBOF_EVENT_ID>
MetaFsIoApiWrapperClass::Read(uint32_t fd, void* buf)
{
    MetaFsReturnCode<IBOF_EVENT_ID> rc;

    off_t fileSize = _GetFileSize(fd);

    bool isSuccess = _ReadFile(fd, buf, fileSize, 0);

    if (isSuccess)
    {
        rc.sc = IBOF_EVENT_ID::SUCCESS;
    }
    else
    {
        rc.sc = IBOF_EVENT_ID::MFS_IO_FAILED_DUE_TO_ERROR;
    }

    return rc;
}
MetaFsReturnCode<IBOF_EVENT_ID>
MetaFsIoApiWrapperClass::Read(uint32_t fd, uint64_t byteOffset, uint64_t byteSize, void* buf)
{
    MetaFsReturnCode<IBOF_EVENT_ID> rc;

    bool isSuccess = _ReadFile(fd, buf, byteSize, byteOffset);

    if (isSuccess)
    {
        rc.sc = IBOF_EVENT_ID::SUCCESS;
    }
    else
    {
        rc.sc = IBOF_EVENT_ID::MFS_IO_FAILED_DUE_TO_ERROR;
    }

    return rc;
}
MetaFsReturnCode<IBOF_EVENT_ID>
MetaFsIoApiWrapperClass::Write(uint32_t fd, void* buf)
{
    MetaFsReturnCode<IBOF_EVENT_ID> rc;

    off_t fileSize = _GetFileSize(fd);
    bool isSuccess = _WriteFile(fd, buf, fileSize, 0);

    if (isSuccess)
    {
        rc.sc = IBOF_EVENT_ID::SUCCESS;
    }
    else
    {
        rc.sc = IBOF_EVENT_ID::MFS_IO_FAILED_DUE_TO_ERROR;
    }

    return rc;
}
MetaFsReturnCode<IBOF_EVENT_ID>
MetaFsIoApiWrapperClass::Write(uint32_t fd, uint64_t byteOffset, uint64_t byteSize, void* buf)
{
    MetaFsReturnCode<IBOF_EVENT_ID> rc;

    bool isSuccess = _WriteFile(fd, buf, byteSize, byteOffset);

    if (isSuccess)
    {
        rc.sc = IBOF_EVENT_ID::SUCCESS;
    }
    else
    {
        rc.sc = IBOF_EVENT_ID::MFS_IO_FAILED_DUE_TO_ERROR;
    }

    return rc;
}

MetaFsReturnCode<IBOF_EVENT_ID>
MetaFsIoApiWrapperClass::SubmitIO(MetaFsAioCbCxt* cxt)
{
    MetaFsReturnCode<IBOF_EVENT_ID> rc;

    bool isSuccess = true;
    FileSizeType fd = cxt->GetFD();
    FileSizeType offset = cxt->GetOffset();
    FileSizeType fileSize = cxt->GetByteSize();

    if (offset == 0 && fileSize == 0)
    {
        fileSize = _GetFileSize(fd);
    }
    else
    {
        fileSize = cxt->GetByteSize();
    }
    switch (cxt->GetOpCode())
    {
        case MetaFsIoOpcode::Read:
        {
            isSuccess = _ReadFile(fd, cxt->GetBuffer(), fileSize, offset);
        }
        break;
        case MetaFsIoOpcode::Write:
        {
            isSuccess = _WriteFile(fd, cxt->GetBuffer(), fileSize, offset);
        }
        break;
        default:
        {
            assert(false);
        }
    }

    if (isSuccess)
    {
        cxt->InvokeCallback();
        rc.sc = IBOF_EVENT_ID::SUCCESS;
    }
    else
    {
        rc.sc = IBOF_EVENT_ID::MFS_IO_FAILED_DUE_TO_ERROR;
    }
    return rc;
}

FileSizeType
MetaFsIoApiWrapperClass::_GetFileSize(uint32_t fd)
{
    return lseek(fd, 0, SEEK_END);
}

bool
MetaFsIoApiWrapperClass::_ReadFile(uint32_t fd, void* buf, FileSizeType byteSize, FileSizeType byteOffset)
{
    ssize_t ioByte;
    if ((ioByte = pread(fd, buf, byteSize, byteOffset)) < 0)
    {
        MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_ERROR_MESSAGE,
            "pread failed...");
        return false;
    }
    return true;
}

bool
MetaFsIoApiWrapperClass::_WriteFile(uint32_t fd, void* buf, FileSizeType byteSize, FileSizeType byteOffset)
{
    ssize_t ioByte;
    if ((ioByte = pwrite(fd, buf, byteSize, byteOffset)) < 0)
    {
        MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_ERROR_MESSAGE,
            "pwrite failed...");
        return false;
    }
    return true;
}

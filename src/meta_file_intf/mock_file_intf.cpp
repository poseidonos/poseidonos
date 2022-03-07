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

#include "mock_file_intf.h"

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <thread>

namespace pos
{
int
MockFileIntf::_Read(int fd, uint64_t fileOffset, uint64_t length, char* buffer)
{
    ssize_t result = pread(fd, buffer, length, fileOffset);
    if (result < 0)
    {
        return -1;
    }
    return 0;
}

int
MockFileIntf::_Write(int fd, uint64_t fileOffset, uint64_t length, char* buffer)
{
    ssize_t result = pwrite(fd, buffer, length, fileOffset);
    if (result < 0)
    {
        return -1;
    }
    return 0;
}

int
MockFileIntf::AsyncIO(AsyncMetaFileIoCtx* ctx)
{
    ctx->ioDoneCheckCallback =
        std::bind(&MockFileIntf::CheckIoDoneStatus, this, std::placeholders::_1);

    _MockAsyncIo(ctx);

    return 0;
}

void
MockFileIntf::_MockAsyncIo(AsyncMetaFileIoCtx* ctx)
{
    if (ctx->opcode == MetaFsIoOpcode::Read)
    {
        std::thread asyncIo(&MockFileIntf::_MockAsyncRead, this, ctx);
        asyncIo.detach();
    }
    else if (ctx->opcode == MetaFsIoOpcode::Write)
    {
        if (volumeType == MetaVolumeType::NvRamVolume)
        {
            _MockAsyncWrite(ctx);
        }
        else
        {
            std::thread asyncIo(&MockFileIntf::_MockAsyncWrite, this, ctx);
            asyncIo.detach();
        }
    }
}

void
MockFileIntf::_MockAsyncRead(AsyncMetaFileIoCtx* ctx)
{
    ssize_t ret = pread(fd, ctx->buffer, ctx->length, ctx->fileOffset);

    int* ioError = new int;
    if (ret < 0)
    {
        *ioError = -1 * errno;
    }
    else
    {
        *ioError = 0;
    }

    ctx->HandleIoComplete(ioError);
}

void
MockFileIntf::_MockAsyncWrite(AsyncMetaFileIoCtx* ctx)
{
    ssize_t ret = pwrite(fd, ctx->buffer, ctx->length, ctx->fileOffset);

    int* ioError = new int;
    if (ret < 0)
    {
        *ioError = -1 * errno;
    }
    else
    {
        *ioError = 0;
    }

    ctx->HandleIoComplete(ioError);
}

int
MockFileIntf::CheckIoDoneStatus(void* data)
{
    int* retData = (int*)data;
    int ioError = *retData;
    delete retData;

    return ioError;
}

int
MockFileIntf::Create(uint64_t fileSize)
{
    size = fileSize;
    fd = creat(fileName.c_str(), 0777);

    if (fd < 0)
    {
        return -1;
    }

    char* data = (char*)malloc(size);
    memset(data, 0x0, size);
    int ret = pwrite(fd, data, size, 0);
    if (ret != -1) // pwrite() Error == -1
    {
        ret = 0;
    }
    free(data);

    close(fd);
    fd = -1;

    return ret;
}

int
MockFileIntf::Open(void)
{
    fd = open(fileName.c_str(), O_RDWR, 0777);
    if (fd != -1) // open() Error == -1
    {
        return MetaFileIntf::Open();
    }
    else
    {
        return -1;
    }
}

int
MockFileIntf::Close(void)
{
    int ret = close(fd);

    if (ret != 0) // close() Success == 0
    {
        ret = -1;
    }
    else
    {
        ret = MetaFileIntf::Close();
    }

    return ret;
}

bool
MockFileIntf::DoesFileExist(void)
{
    return (0 == access(fileName.c_str(), F_OK));
}

int
MockFileIntf::Delete(void)
{
    int ret = remove(fileName.c_str());
    if (ret != 0) // remove() Success == 0
    {
        ret = -1;
    }

    return ret;
}

uint64_t
MockFileIntf::GetFileSize(void)
{
    if (isOpened != true)
    {
        fd = open(fileName.c_str(), O_RDONLY, 0777);
    }

    if (fd < 0)
    {
        return 0;
    }

    size = lseek(fd, 0, SEEK_END);

    if (isOpened != true)
    {
        close(fd);
        fd = -1;
    }

    return size;
}

} // namespace pos

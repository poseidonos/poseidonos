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
#include "src/metafs/include/metafs_service.h"
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

namespace pos
{
MetaFsFileIntf::MetaFsFileIntf(std::string fname, std::string aname,
                                        StorageOpt storageOpt)
: MetaFileIntf(fname, aname, storageOpt),
  metaFs(nullptr)
{
    metaFs = MetaFsServiceSingleton::Instance()->GetMetaFs(aname);
    _SetFileProperty(storageOpt);
}

MetaFsFileIntf::MetaFsFileIntf(std::string fname, int arrayId,
                                        StorageOpt storageOpt)
: MetaFileIntf(fname, arrayId, storageOpt),
  metaFs(nullptr)
{
    metaFs = MetaFsServiceSingleton::Instance()->GetMetaFs(arrayId);
    _SetFileProperty(storageOpt);
}

// only for test
MetaFsFileIntf::MetaFsFileIntf(std::string fname, std::string aname,
                                        MetaFs* metaFs, StorageOpt storageOpt)
: MetaFileIntf(fname, aname, storageOpt),
  metaFs(metaFs)
{
    _SetFileProperty(storageOpt);
}

// only for test
MetaFsFileIntf::MetaFsFileIntf(std::string fname, int arrayId, MetaFs* metaFs,
                                        StorageOpt storageOpt)
: MetaFileIntf(fname, arrayId, storageOpt),
  metaFs(metaFs)
{
    _SetFileProperty(storageOpt);
}

MetaFsFileIntf::~MetaFsFileIntf(void)
{
}

int
MetaFsFileIntf::_Read(int fd, uint64_t fileOffset, uint64_t length, char* buffer)
{
    MetaStorageType storageType = MetaFileUtil::ConvertToMediaType(storage);
    POS_EVENT_ID rc = metaFs->io->Read(fd, fileOffset, length, buffer, storageType);

    if (POS_EVENT_ID::SUCCESS != rc)
        return -(int)POS_EVENT_ID::MFS_FILE_READ_FAILED;

    return (int)POS_EVENT_ID::SUCCESS;
}

int
MetaFsFileIntf::_Write(int fd, uint64_t fileOffset, uint64_t length, char* buffer)
{
    MetaStorageType storageType = MetaFileUtil::ConvertToMediaType(storage);
    POS_EVENT_ID rc = metaFs->io->Write(fd, fileOffset, length, buffer, storageType);

    if (POS_EVENT_ID::SUCCESS != rc)
        return -(int)POS_EVENT_ID::MFS_FILE_WRITE_FAILED;

    return (int)POS_EVENT_ID::SUCCESS;
}

int
MetaFsFileIntf::AsyncIO(AsyncMetaFileIoCtx* ctx)
{
    ctx->ioDoneCheckCallback =
        std::bind(&MetaFsFileIntf::CheckIoDoneStatus, this, std::placeholders::_1);

    MetaFsAioCbCxt* aioCb = new MetaFsAioCbCxt(ctx->opcode, ctx->fd,
        MetaFsServiceSingleton::Instance()->GetArrayId(arrayName),
        ctx->fileOffset, ctx->length, (void*)ctx->buffer,
        AsEntryPointParam1(&AsyncMetaFileIoCtx::HandleIoComplete, ctx));

    MetaStorageType storageType = MetaFileUtil::ConvertToMediaType(storage);
    POS_EVENT_ID rc = metaFs->io->SubmitIO(aioCb, storageType);

    if (POS_EVENT_ID::SUCCESS != rc)
        return -(int)rc;

    issuedCount++;

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

    issuedCount--;

    delete asyncCtx;
    return error;
}

int
MetaFsFileIntf::Create(uint64_t fileSize)
{
    POS_EVENT_ID rc = metaFs->ctrl->Create(fileName, fileSize, fileProperty, storage);
    if (POS_EVENT_ID::SUCCESS != rc)
    {
        return -(int)POS_EVENT_ID::MFS_FILE_CREATE_FAILED;
    }

    size = fileSize;

    return (int)POS_EVENT_ID::SUCCESS;
}

int
MetaFsFileIntf::Open(void)
{
    POS_EVENT_ID rc = metaFs->ctrl->Open(fileName, fd, storage);

    if (POS_EVENT_ID::SUCCESS != rc)
    {
        return -(int)POS_EVENT_ID::MFS_FILE_OPEN_FAILED;
    }

    return MetaFileIntf::Open();
}

int
MetaFsFileIntf::Close(void)
{
    POS_EVENT_ID rc = metaFs->ctrl->Close(fd, storage);

    while (0 != GetIssuedCount())
    {
        // wait for done
        usleep(1);
    }

    if (POS_EVENT_ID::SUCCESS != rc)
    {
        return -(int)POS_EVENT_ID::MFS_FILE_CLOSE_FAILED;
    }

    return MetaFileIntf::Close();
}

bool
MetaFsFileIntf::DoesFileExist(void)
{
    POS_EVENT_ID rc = metaFs->ctrl->CheckFileExist(fileName, storage);

    return (POS_EVENT_ID::SUCCESS == rc);
}

int
MetaFsFileIntf::Delete(void)
{
    POS_EVENT_ID rc = metaFs->ctrl->Delete(fileName, storage);

    if (POS_EVENT_ID::SUCCESS != rc)
    {
        return -(int)POS_EVENT_ID::MFS_FILE_DELETE_FAILED;
    }

    return (int)POS_EVENT_ID::SUCCESS;
}

uint64_t
MetaFsFileIntf::GetFileSize(void)
{
    return metaFs->ctrl->GetFileSize(fd, storage);
}

void
MetaFsFileIntf::_SetFileProperty(StorageOpt storageOpt)
{
    if (storageOpt == StorageOpt::NVRAM)
    {
        fileProperty.ioAccPattern = MetaFileAccessPattern::ByteIntensive;
        fileProperty.ioOpType = MetaFileDominant::WriteDominant;
        fileProperty.integrity = MetaFileIntegrityType::Lvl0_Disable;
    }
    else
    {
        // Use default
    }
}

} // namespace pos

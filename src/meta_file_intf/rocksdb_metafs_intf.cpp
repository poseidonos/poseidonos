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

#include "rocksdb_metafs_intf.h"

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <experimental/filesystem>

#include "rocksdb/db.h"
#include "src/array_mgmt/array_manager.h"
#include "src/array_models/interface/i_array_info.h"
#include "src/io_submit_interface/i_io_submit_handler.h"
#include "src/metafs/config/metafs_config_manager.h"
#include "src/metafs/include/metafs_service.h"
#include "src/metafs/nvram_io_completion.h"

namespace pos
{
RocksDBMetaFsIntf::RocksDBMetaFsIntf(const std::string fileName, const int arrayId,
    MetaFileType fileType, MetaVolumeType volumeType)
: MetaFileIntf(fileName, arrayId, fileType, volumeType),
  metaFs(MetaFsServiceSingleton::Instance()->GetMetaFs(arrayId)),
  blksPerStripe(0),
  baseLpn(UINT64_MAX),
  BYTE_ACCESS_ENABLED(MetaFsServiceSingleton::Instance()->GetConfigManager()->IsDirectAccessEnabled())
{
    rocksMeta = metaFs->GetRocksMeta();
    fileDescriptorAllocator = metaFs->GetFileDescriptorAllocator();
}

// only for test
RocksDBMetaFsIntf::RocksDBMetaFsIntf(std::string fileName, int arrayId, MetaFs* metafs,
    MetaFsConfigManager* configManager,
    const MetaFileType fileType,
    const MetaVolumeType volumeType)
: MetaFileIntf(fileName, arrayId, fileType, volumeType),
  metaFs(metafs),
  blksPerStripe(0),
  baseLpn(UINT64_MAX),
  BYTE_ACCESS_ENABLED(false)
{
}

RocksDBMetaFsIntf::~RocksDBMetaFsIntf(void)
{
}

int
RocksDBMetaFsIntf::_Read(int fd, uint64_t fileOffset, uint64_t length, char* buffer)
{
    if (rocksMeta == nullptr)
    {
        return ERRID(MFS_FILE_READ_FAILED);
    }

    std::string key = _MakeRocksDbKey(fd, fileOffset);
    std::string getValue;
    rocksdb::Status getValueStatus = rocksMeta->Get(rocksdb::ReadOptions(), key, &getValue);
    if (getValueStatus.ok())
    {
        uint64_t size = getValue.size();
        assert(length == size);
        memcpy((void*)buffer, getValue.c_str(), size);
        POS_TRACE_DEBUG(EID(ROCKSDB_MFS_INTERNAL_READ_SUCCEED), "RocksDBMetaFsIntf Internal Read succeed fd {} , fileOffset : {} , length : {}", fileName, fd, length);
        return EID(SUCCESS);
    }
    else
    {
        POS_TRACE_ERROR(EID(ROCKSDB_MFS_INTERNAL_READ_FAILED), "RocksDBMetaFsIntf Internal Read failed fd {} , fileOffset : {} , length : {}", fileName, fd, length);
        return ERRID(ROCKSDB_MFS_INTERNAL_READ_FAILED);
    }
}

int
RocksDBMetaFsIntf::_Write(int fd, uint64_t fileOffset, uint64_t length, char* buffer)
{
    if (rocksMeta == nullptr)
    {
        return ERRID(MFS_FILE_WRITE_FAILED);
    }
    std::string key = _MakeRocksDbKey(fd, fileOffset);
    std::string value(buffer, length);
    rocksdb::Status writeMetaStatus = rocksMeta->Put(rocksdb::WriteOptions(), key, value);
    if (writeMetaStatus.ok())
    {
        POS_TRACE_DEBUG(EID(ROCKSDB_MFS_INTERNAL_WRITE_SUCCEED), "RocksDBMetaFsIntf Internal Write succeed fd {} , fileOffset : {} , length : {}", fileName, fd, length);
        return EID(SUCCESS);
    }
    else
    {
        POS_TRACE_ERROR(EID(ROCKSDB_MFS_INTERNAL_WRITE_FAILED), "RocksDBMetaFsIntf Internal Write failed fd {} , fileOffset : {} , length : {}", fileName, fd, length);
        return ERRID(ROCKSDB_MFS_INTERNAL_WRITE_FAILED);
    }
}

int
RocksDBMetaFsIntf::AsyncIO(AsyncMetaFileIoCtx* ctx)
{
    if (ctx->fileOffset >= size)
    {
        POS_TRACE_ERROR(EID(ROCKSDB_MFS_ASYNCIO_OFFSET_ERROR), "RocksDBMetaFsIntf Internal Write failed fd {} , fileOffset : {} , length : {}", fileName, fd, size);
        return ERRID(ROCKSDB_MFS_ASYNCIO_OFFSET_ERROR);
    }

    ctx->ioDoneCheckCallback =
        std::bind(&RocksDBMetaFsIntf::ReleaseAsyncIoContext, this, std::placeholders::_1);

    if (ctx->opcode == MetaFsIoOpcode::Write)
    {
        return _AsyncIOWrite(ctx);
    }
    else
    {
        return _AsyncIORead(ctx);
    }
    return EID(SUCCESS);
}

int
RocksDBMetaFsIntf::CheckIoDoneStatus(void* data)
{
    // nothing to do
    return 0;
}

int
RocksDBMetaFsIntf::ReleaseAsyncIoContext(void* data)
{
    int error = EID(SUCCESS);
    MetaFsAioCbCxt* asyncCtx = reinterpret_cast<MetaFsAioCbCxt*>(data);
    error = EID(SUCCESS);
    delete asyncCtx;
    return error;
}

int
RocksDBMetaFsIntf::Create(uint64_t fileSize)
{
    fd = fileDescriptorAllocator->FindFdByName(fileName);
    if (MetaFsCommonConst::INVALID_FD == fd)
    {
        fd = fileDescriptorAllocator->Alloc(fileName);
    }

    std::string key = _MakeRocksDbInodeKey(fileName);
    std::string value = _MakeRocksDbInode(fd, fileSize);
    rocksdb::Status status = rocksMeta->Put(rocksdb::WriteOptions(), key, value);

    if (status.ok())
    {
        size = fileSize;
        POS_TRACE_INFO(EID(ROCKSDB_MFS_CREATE_SUCCEED), "RocksDBMetaFsIntf Create succeed, filename : {} , fd : {} , filesize : {}", fileName, fd, size);
        return EID(SUCCESS);
    }
    else
    {
        POS_TRACE_ERROR(EID(ROCKSDB_MFS_CREATE_FAILED), "RocksDBMetaFsIntf Create file failed name : {}", fileName);
        return ERRID(ROCKSDB_MFS_CREATE_FAILED);
    }
}

int
RocksDBMetaFsIntf::Open(void)
{
    std::string key = _MakeRocksDbInodeKey(fileName);
    std::string value;
    rocksdb::Status status = rocksMeta->Get(rocksdb::ReadOptions(), key, &value);
    if (status.ok())
    {
        pair<FileDescriptorType, uint64_t> fdSize = _GetValueFromInode(value);
        fd = fdSize.first;
        size = fdSize.second;
        StringHashType fileKey = MetaFileUtil::GetHashKeyFromFileName(fileName);
        fileDescriptorAllocator->UpdateLookupMap(fileKey, fd);
        POS_TRACE_INFO(EID(ROCKSDB_MFS_OPEN_SUCCEED), "RocksDBMetaFsIntf Open succeed filename : {} , fd : {}", fileName, fd);
        return MetaFileIntf::Open();
    }
    else
    {
        POS_TRACE_ERROR(EID(ROCKSDB_MFS_OPEN_FAILED), "RocksDBMetaFsIntf Open file failed name : {} , code : {} ", fileName, status.code());
        return ERRID(ROCKSDB_MFS_OPEN_FAILED);
    }
}

int
RocksDBMetaFsIntf::Close(void)
{
    return MetaFileIntf::Close();
}

bool
RocksDBMetaFsIntf::DoesFileExist(void)
{
    std::string key = _MakeRocksDbInodeKey(fileName);
    rocksdb::Slice slice(key);
    std::string tmp;
    rocksdb::Status doesExist = rocksMeta->Get(rocksdb::ReadOptions(), slice, &tmp);
    if (doesExist.ok())
    {
        POS_TRACE_INFO(EID(ROCKSDB_MFS_FILE_EXIST), "RocksDBMetaFsIntf File Does exist filename : {}", fileName);
        return true;
    }
    else
    {
        POS_TRACE_INFO(EID(ROCKSDB_MFS_FILE_NOT_EXIST), "RocksDBMetaFsIntf File Does not exist filename : {}", fileName);
        return false;
    }
}

int
RocksDBMetaFsIntf::Delete(void)
{
    std::string key = _MakeRocksDbInodeKey(fileName);
    FileDescriptorType fdValue = fileDescriptorAllocator->FindFdByName(fileName);
    fileDescriptorAllocator->Free(fileName, fdValue);

    rocksdb::WriteBatch batch;
    batch.Delete(key);

    std::string keyToStart = _MakeRocksDbKey(fd, 0);
    std::string keyToEnd = _MakeRocksDbKey(fd + 1, 0);
    rocksdb::Slice start(keyToStart), end(keyToEnd);
    batch.DeleteRange(start, end);

    rocksdb::Status status = rocksMeta->Write(rocksdb::WriteOptions(), &batch);

    // Delete Metafiles atomically
    if (status.ok())
    {
        POS_TRACE_INFO(EID(ROCKSDB_MFS_FILE_DELETE_SUCCEED), "RocksDBMetaFsIntf Delete succeed filename : {}", fileName);
        return EID(SUCCESS);
    }
    else
    {
        POS_TRACE_ERROR(EID(ROCKSDB_MFS_FILE_DELETE_FAILED), "RocksDBMetaFsIntf Delete failed filename : {}", fileName);
        return ERRID(ROCKSDB_MFS_FILE_DELETE_FAILED);
    }
}

uint64_t
RocksDBMetaFsIntf::GetFileSize(void)
{
    std::string key = _MakeRocksDbInodeKey(fileName);
    std::string value;
    rocksdb::Status status = rocksMeta->Get(rocksdb::ReadOptions(), key, &value);
    if (status.ok())
    {
        uint64_t fileSize = _GetValueFromInode(value).second;
        return fileSize;
    }
    else
    {
        POS_TRACE_ERROR(EID(ROCKSDB_MFS_GET_FILE_SIZE_FAILED), "RocksDBMetaFsIntf GetFileSize failed filename : {}", fileName);
        return ERRID(ROCKSDB_MFS_GET_FILE_SIZE_FAILED);
    }
}

int
RocksDBMetaFsIntf::_AsyncIOWrite(AsyncMetaFileIoCtx* ctx)
{
    rocksdb::WriteBatch writeBatch;
    uint64_t writeSize = ctx->GetLength();
    uint64_t pageSize = 4032;
    for (uint64_t writtenSize = 0; writtenSize < writeSize; writtenSize += pageSize)
    {
        uint64_t fragmentSize = (writeSize - writtenSize < 4032) ? writeSize - writtenSize : pageSize;
        std::string fragmentKey = _MakeRocksDbKey(ctx->fd, ctx->fileOffset + writtenSize); // fileoffset + writtenSize == startOffset of fragment
        std::string fragmentValue(&ctx->buffer[writtenSize], &ctx->buffer[writtenSize + fragmentSize]);
        writeBatch.Put(fragmentKey, fragmentValue);
    }

    rocksdb::Status status = rocksMeta->Write(rocksdb::WriteOptions(), &writeBatch);
    if (status.ok())
    {
        POS_TRACE_DEBUG(EID(ROCKSDB_MFS_ASYNCIO_WRITE_SUCCEED), "RocksDBMetaFsIntf AsyncIO write succeed, fd : {} , offset : {}, size : {}", ctx->fd, ctx->fileOffset, ctx->GetLength());
        MetaFsAioCbCxt* aioCb = new MetaFsAioCbCxt(ctx, arrayId);
        aioCb->SetCallbackCount(1);
        aioCb->InvokeCallback();
        return EID(SUCCESS);
    }
    else
    {
        POS_TRACE_ERROR(EID(ROCKSDB_MFS_ASYNCIO_WRITE_FAILED), "RocksDBMetaFsIntf::AsyncIO error write failed fd : {} , offset : {}, size : {}", ctx->fd, ctx->fileOffset, ctx->GetLength());
        return ERRID(ROCKSDB_MFS_ASYNCIO_WRITE_FAILED);
    }
}

int
RocksDBMetaFsIntf::_AsyncIORead(AsyncMetaFileIoCtx* ctx)
{
    bool isFullFileIo = (ctx->fileOffset == 0 && ctx->GetLength() == 0);
    if (isFullFileIo)
    {
        std::string keyToStart = _MakeRocksDbKey(ctx->fd, 0);
        std::string keyToLimit = _MakeRocksDbKey(ctx->fd + 1, 0);

        rocksdb::Iterator* it = rocksMeta->NewIterator(rocksdb::ReadOptions());
        uint64_t offset = 0;
        uint64_t prevOffset = 0;
        for (it->Seek(keyToStart); it->Valid() && it->key().ToString() < keyToLimit; it->Next())
        {
            std::string itValue = it->value().ToString();
            uint64_t valueSize = itValue.size();
            uint64_t currOffset = _GetOffsetFromKey(it->key().ToString());
            if (currOffset != prevOffset && currOffset != offset)
            {
                POS_TRACE_ERROR(EID(ROCKSDB_MFS_OFFSET_IS_NOT_CONSECUTIVE), "RocksDBMetaFsIntf AsyncIO read failed current offset is not equal with expected offset, fd : {}, current offset : {}, expected offset : {}", ctx->fd, currOffset, offset);
                assert(false);
            }
            if (offset + valueSize > size)
            {
                uint64_t bufferOverflowSize = offset + valueSize;
                POS_TRACE_ERROR(EID(ROCKSDB_MFS_ASYNCIO_READ_FULLIO_READ_OVERFLOW), "RocksDBMetaFsIntf AsyncIO read failed try to read offset which is over total size, fd : {}, current write size : {}, max write size : {} ", fd, bufferOverflowSize, size);
                return -1;
            }
            memcpy((void*)(ctx->buffer + offset), itValue.c_str(), valueSize);
            offset += valueSize;
            prevOffset = currOffset;
        }

        if (it->status().ok())
        {
            MetaFsAioCbCxt* aioCb = new MetaFsAioCbCxt(ctx, arrayId);
            POS_TRACE_DEBUG(EID(ROCKSDB_MFS_ASYNCIO_READ_FULLIO_READ_SUCCEED), "RocksDBMetaFsIntf AsyncIO fullio read succeed, fd : {} , offset : {}, size : {}", ctx->fd, ctx->fileOffset, ctx->GetLength());
            aioCb->SetCallbackCount(1);
            aioCb->InvokeCallback();
            return EID(SUCCESS);
        }
        else
        {
            POS_TRACE_ERROR(EID(ROCKSDB_MFS_ASYNCIO_READ_FULLIO_READ_FAILED), "RocksDBMetaFsIntf AsyncIO fullio read failed fd : {} , offset : {}, size : {}", ctx->fd, ctx->fileOffset, ctx->GetLength());
            return ERRID(ROCKSDB_MFS_ASYNCIO_READ_FULLIO_READ_FAILED);
        }
    }
    else
    {
        uint64_t readSize = ctx->GetLength();
        std::string keyToStart = _MakeRocksDbKey(ctx->fd, ctx->fileOffset);
        std::string keyToLimit = _MakeRocksDbKey(ctx->fd, ctx->fileOffset + readSize);

        rocksdb::Iterator* it = rocksMeta->NewIterator(rocksdb::ReadOptions());
        uint64_t offset = 0;
        uint64_t prevOffset = ctx->fileOffset;
        it->Seek(keyToStart);
        for (it->Seek(keyToStart); it->Valid() && it->key().ToString() < keyToLimit; it->Next())
        {
            std::string itValue = it->value().ToString();
            uint64_t valueSize = itValue.size();
            uint64_t currOffset = _GetOffsetFromKey(it->key().ToString());
            if (currOffset != prevOffset && currOffset != offset)
            {
                POS_TRACE_ERROR(EID(ROCKSDB_MFS_OFFSET_IS_NOT_CONSECUTIVE), "RocksDBMetaFsIntf AsyncIO read failed current offset is not equal to expected offset, fd : {}, current offset : {}, expected offset : {}", ctx->fd, currOffset, offset);
                assert(false);
            }
            if (offset + valueSize > size)
            {
                uint64_t bufferOverflowSize = offset + valueSize;
                POS_TRACE_ERROR(EID(ROCKSDB_MFS_ASYNCIO_READ_FULLIO_READ_OVERFLOW), "RocksDBMetaFsIntf AsyncIO read failed try to read offset which is over total size, fd : {}, current write size : {}, max write size : {} ", fd, bufferOverflowSize, size);
                return -1;
            }
            memcpy((void*)(ctx->buffer + offset), itValue.c_str(), valueSize);
            offset += valueSize;
            prevOffset = currOffset;
        }

        if (offset != readSize)
        {
            POS_TRACE_ERROR(EID(ROCKSDB_MFS_READ_SIZE_IS_NOT_EQUAL_TO_EXPECTED_READ_SIZE), "RocksDBMetaFsIntf::AsyncIO partial io read failed, expected read size is different with actual read size, fd : {}, current read size : {}, expected read size : {}", ctx->fd, offset, readSize);
        }
        assert(offset == readSize);
        if (it->status().ok())
        {
            POS_TRACE_DEBUG(EID(ROCKSDB_MFS_ASYNCIO_PARTIAL_READ_SUCCEED), "RocksDBMetaFsIntf AsyncIO partial io read succeed , fd : {}, fileoffset : {} ", ctx->fd, ctx->fileOffset);
            MetaFsAioCbCxt* aioCb = new MetaFsAioCbCxt(ctx, arrayId);
            aioCb->SetCallbackCount(1);
            aioCb->InvokeCallback();
            return EID(SUCCESS);
        }
        else
        {
            POS_TRACE_ERROR(EID(ROCKSDB_MFS_ASYNCIO_PARTIAL_READ_FAILED), "RocksDBMetaFsIntf::AsyncIO partial io read failed , fd : {}, fileoffset : {} ", ctx->fd, ctx->fileOffset);
            return ERRID(ROCKSDB_MFS_ASYNCIO_PARTIAL_READ_FAILED);
        }
    }
}

int
RocksDBMetaFsIntf::CreateDirectory(std::string pathName)
{
    this->pathName = pathName;
    if (std::experimental::filesystem::exists(pathName))
    {
        POS_TRACE_INFO(EID(ROCKSDB_MFS_DIR_EXIST), "RocksDB mfs directory already exists (path :{}) ", pathName);
        return 0;
    }

    bool ret = std::experimental::filesystem::create_directory(pathName);
    if (ret != true)
    {
        POS_TRACE_ERROR(EID(ROCKSDB_MFS_DIR_CREATION_FAILED), "RocksDB mfs directory creation failed (path :{}) ", pathName);
        return ERRID(ROCKSDB_MFS_DIR_CREATION_FAILED);
    }

    POS_TRACE_INFO(EID(ROCKSDB_MFS_DIR_CREATION_SUCCEED), "RocksDB mfs directory created (path :{}) ", pathName);
    return 0;
}

int
RocksDBMetaFsIntf::DeleteDirectory(void)
{
    bool ret = std::experimental::filesystem::remove_all(pathName);
    if (ret != true)
    {
        POS_TRACE_ERROR(EID(ROCKSDB_MFS_DIR_DELETION_FAILED), "RocksDB mfs directory does not exists, so deletion failed (path :{}) ", pathName);
        return ERRID(ROCKSDB_LOG_BUFFER_DIR_DELETION_FAILED);
    }
    POS_TRACE_INFO(EID(ROCKSDB_MFS_DIR_DELETION_SUCCEED), "RocksDB mfs directory deleted (path :{}) ", pathName);
    return EID(SUCCESS);
}

// Following methods are used only in integration test
int
RocksDBMetaFsIntf::SetRocksMeta(std::string pathName)
{
    rocksdb::Options options;
    options.create_if_missing = true;
    rocksdb::Status status = rocksdb::DB::Open(options, pathName, &rocksMeta);
    fileDescriptorAllocator = new FileDescriptorAllocator();
    if (status.ok())
    {
        POS_TRACE_INFO(EID(ROCKSDB_MFS_SET_ROCKS_META_SUCCEED), "RocksDB mfs set RocksMeta Succeed, filename : {} ", pathName);
        return EID(SUCCESS);
    }
    else
    {
        POS_TRACE_ERROR(EID(ROCKSDB_MFS_SET_ROCKS_META_FAILED), "RocksDB mfs set RocksMeta Failed, filename : {} ", pathName);
        return ERRID(ROCKSDB_MFS_SET_ROCKS_META_FAILED);
    }
}

int
RocksDBMetaFsIntf::DeleteRocksMeta(void)
{
    if (nullptr != rocksMeta)
    {
        delete rocksMeta;
        rocksMeta = nullptr;
    }

    if (nullptr != fileDescriptorAllocator)
    {
        delete fileDescriptorAllocator;
        fileDescriptorAllocator = nullptr;
    }
    POS_TRACE_INFO(EID(ROCKSDB_MFS_DELETE_ROCKS_META), "RocksDB mfs delete RocksMeta , filename : {} ", pathName);

    return 0;
}

} // namespace pos

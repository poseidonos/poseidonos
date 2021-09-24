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

#include "mss_ramdisk.h"

#include <errno.h>

#include "metafs_config.h"
#include "mss_complete_handler.h"
#include "mss_utils.h"
#include "os_header.h"
#include "src/logger/logger.h"

namespace pos
{

MssRamdisk::MssRamdisk(int arrayId)
: MetaStorageSubsystem(arrayId)
{
    for (int i = 0; i < static_cast<int>(MetaStorageType::Max); i++)
    {
        fd_.push_back(-1);
        path_.push_back("");
        capacity_.push_back(0);
    }
}

MssRamdisk::~MssRamdisk(void)
{
    // TODO(munseop.lim): need to fix
    _Finalize();
}


void
MssRamdisk::_Finalize(void)
{
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Closing File Descriptor of Meta storage subsystem.");
    for (int fd : fd_)
    {
        if (fd != -1)
        {
            close(fd);
        }
    }

    std::string freeCommand, unmountCommand;
    int status = 0;
    bool opSuccess = true;

    for (std::string path : path_)
    {
        if (!path.empty())
        {
            unmountCommand = "umount " + path;

            opSuccess = false;
            // repeat unmout call since there might be obsolete storages which possibly exist in case of ibofOS crash
            do
            {
                status = system(unmountCommand.c_str());

                if (0 == status)
                {
                    opSuccess = true;
                }
            } while (status != 0);

            if (!opSuccess)
            {
                MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_ERROR_MESSAGE,
                    "Unmount ramdisk failed due to some reasons...");
            }
            else
            {
                MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
                    "Completed to unmount meta storage.");
            }
            freeCommand = "rm -rf " + path + FILE_NAME + "." + to_string(arrayId);
            status = system(freeCommand.c_str());
            if (status != 0)
            {
                MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_ERROR_MESSAGE,
                    "File Delete operation failed due to some reasons...");
            }
            else
            {
                MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
                    "Completed File Delete in  meta storage.");
            }
        }
    }

    for (int i = 0; i < static_cast<int>(MetaStorageType::Max); i++)
    {
        if (!path_[i].empty())
        {
            path_[i].clear();
        }
    }
}

/*
 * Create Meta Storage using Ramdisk
 * This function should not fail. Creates tmpfs filesystem.
 *
 * @mediaType  NVRAM/SDD type of media
 * @capacity  size of ramdisk to be created in bytes
 *
 * @return Success(0)/Failure(-1)
 */

POS_EVENT_ID
MssRamdisk::CreateMetaStore(int arrayId, MetaStorageType mediaType, uint64_t capacity, bool formatFlag)
{
    std::string mountCommand;
    std::string createDir;
    std::string fileName;
    int status = 0;
    int index = static_cast<int>(mediaType);

    if (!path_[index].empty())
    {
        MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "You attempt to create meta storage again. media type={}",
            (int)mediaType);
        Open();
        return POS_EVENT_ID::SUCCESS; // just return success
    }

    switch (mediaType)
    {
        case MetaStorageType::NVRAM:
            path_[index] = RAMDISK_PATH + "NVRAM/";
            break;
        case MetaStorageType::SSD:
            path_[index] = RAMDISK_PATH + "SSD/";
            break;
        default:
            MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_INVALID_PARAMETER,
                "Wrong Media Type Provided");
            assert(false);
    }

    capacity_[index] = (uint64_t)capacity;

    if (fd_[index] == -1 && CheckFileExists(path_[index] + FILE_NAME))
    {
        // if resource was not freed last time. free it now.
        if (false == DeleteFile(path_[index] + FILE_NAME))
        {
            MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_FILE_DELETE_FAILED,
                "Delete file {} manually.",
                path_[index] + FILE_NAME);
            return POS_EVENT_ID::MFS_FILE_DELETE_FAILED;
        }
    }

    createDir = "mkdir -p " + path_[index];
    status = system(createDir.c_str());
    assert(0 == status);

    mountCommand = "mount -o size=" + std::to_string(BTOMB(capacity)) + "M -t tmpfs none " + path_[index];

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
        "{}", mountCommand.c_str());
    status = system(mountCommand.c_str());

    if (status != 0)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_MEDIA_MOUNT_FAILED,
            "Mount ramdisk failed. Please run as root permission or sudo option...");
    }
    assert(0 == status);

    fileName = path_[index] + FILE_NAME;
    fd_[index] = open(fileName.c_str(), O_RDWR | O_CREAT, 644);

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Opened File Descriptor: {}", fd_[index]);
    assert(fd_[index] > 2);
    assert(capacity != 0 && (capacity % MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES == 0));

    Open();

    if (formatFlag)
    {
        EraseAllData(mediaType);
    }

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MssRamdisk::EraseAllData(MetaStorageType mediaType)
{
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Start to erase all data in Meta storage..");
    int index = static_cast<int>(mediaType);
    const uint64_t bufferByteSize = 1024 * 1024; // 1MB
    uint64_t wBytesReqested, bytesWritten;
    void* buffer = calloc(bufferByteSize / MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES,
        MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES);

    wBytesReqested = bufferByteSize;
    uint64_t iter = capacity_[index] / wBytesReqested;
    for (uint64_t idx = 0; idx < iter; idx++)
    {
        bytesWritten = pwrite(fd_[index], buffer, bufferByteSize, idx * bufferByteSize);
        if (bytesWritten != wBytesReqested)
        {
            MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_MEDIA_WROTE_SIZE_NOT_MATCHED,
                "Error occurred during AllData erase.");
            free(buffer);
            return POS_EVENT_ID::MFS_MEDIA_WROTE_SIZE_NOT_MATCHED;
        }
    }
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Rollback lseek offset to 0");
    off_t off = lseek(fd_[index], 0, SEEK_SET);
    if (off == -1)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_MEDIA_SEEK_FAILED,
            "lseek failed.");
        free(buffer);
        return POS_EVENT_ID::MFS_MEDIA_SEEK_FAILED;
    }
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Erase completed");
    free(buffer);

    return POS_EVENT_ID::SUCCESS;
}

/**
 * Get Total Capacity
 *
 * @mediaType  NVRAM/SDD type of media
 *
 * @return capacity of ramdisk
 */

uint64_t
MssRamdisk::GetCapacity(MetaStorageType mediaType)
{
    int index = static_cast<int>(mediaType);
    return capacity_[index];
}

/**
 * read given page into buffer form ramdisk
 *
 * @mediaType  NVRAM/SDD type of media
 * @pageNumber     Address of page to read. Must be 4K aligned.
 * @buffer  Destination memory to write data.
 * @count   Number of bytes in multiple of 4K pages.
 *
 * @return  Success(0)/Failure(-1)
 */

POS_EVENT_ID
MssRamdisk::ReadPage(MetaStorageType mediaType, MetaLpnType pageNumber, void* buffer, MetaLpnType count)
{
    int64_t bytesRead;
    int64_t rByteOffset = (int64_t)pageNumber * MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES;
    int64_t rBytesRequested = (int64_t)count * MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES;
    int index = static_cast<int>(mediaType);

    std::unique_lock<std::mutex> lock(lock_io);

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[RamD][Read page  ] mediaType={}, pageNum={}, OffsetInPread={}, BytesReq={}",
        (int)mediaType, pageNumber, rByteOffset, rBytesRequested);

    bytesRead = pread(fd_[index], buffer, rBytesRequested, rByteOffset);

    if (bytesRead != rBytesRequested)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_MEDIA_READ_FAILED,
            "Read failed. return val={}, bytes requested={}",
            bytesRead, rBytesRequested);
        if (bytesRead == -1)
        {
            MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_ERROR_MESSAGE,
                "ERROR: errno={}", errno);
        }
        return POS_EVENT_ID::MFS_MEDIA_READ_FAILED;
    }
    return POS_EVENT_ID::SUCCESS;
}

/**
 * Write given page from buffer to ramdisk
 *
 * @mediaType  NVRAM/SDD type of media
 * @pageNumber     Address of page to read. Must be 4K aligned.
 * @buffer  Source memory to read data.
 * @count   Number of bytes in multiple of 4K pages.
 *
 * @return  Success(0)/Failure(-1)
 */

POS_EVENT_ID
MssRamdisk::WritePage(MetaStorageType mediaType, MetaLpnType pageNumber, void* buffer, MetaLpnType count)
{
    int64_t bytesWritten = 0;
    int64_t wByteOffset = (int64_t)pageNumber * MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES;
    int64_t wBytesRequested = (int64_t)count * MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES;
    int index = static_cast<int>(mediaType);

    std::unique_lock<std::mutex> lock(lock_io);

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[RamD][Write page ] mediaType={}, pageNum={}, OffsetInPread={}, BytesReq={}",
        (int)mediaType, pageNumber, wByteOffset, wBytesRequested);

    // 4KB size unit
    bytesWritten = pwrite(fd_[index], buffer, wBytesRequested, wByteOffset);

    if (bytesWritten != wBytesRequested)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_MEDIA_WRITE_FAILED,
            "Write failed. return val={}, bytes requested={}",
            bytesWritten, wBytesRequested);
        if (bytesWritten == -1)
        {
            MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_ERROR_MESSAGE,
                "ERROR: errno={}", errno);
        }
        return POS_EVENT_ID::MFS_MEDIA_WRITE_FAILED;
    }
    return POS_EVENT_ID::SUCCESS;
}


POS_EVENT_ID
MssRamdisk::Open(void)
{
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Meta storage system is opened");

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MssRamdisk::Close(void)
{
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Meta storage system is closed");

    _Finalize();

    return POS_EVENT_ID::SUCCESS;
}

bool
MssRamdisk::IsAIOSupport(void)
{
    return false;
}

POS_EVENT_ID
MssRamdisk::ReadPageAsync(MssAioCbCxt* cb)
{
    assert(false); // not supported
}

POS_EVENT_ID
MssRamdisk::WritePageAsync(MssAioCbCxt* cb)
{
    assert(false); // not supported
}

POS_EVENT_ID
MssRamdisk::TrimFileData(MetaStorageType mediaType, MetaLpnType startLpn, void* buffer, MetaLpnType numPages)
{
    int media = static_cast<int>(mediaType);
    uint64_t ioSize = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES;
    uint64_t trimReqSize = numPages * ioSize;
    uint64_t trimedSize = 0;
    uint64_t remainedSize = 0;

    for (uint64_t idx = 0; idx < trimReqSize; idx++)
    {
        trimedSize = pwrite(fd_[media], buffer, ioSize, idx * ioSize);

        if (trimedSize != ioSize)
        {
            MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_FILE_TRIM_FAILED,
                "Error occurred during triming file");
            free(buffer);
            return POS_EVENT_ID::MFS_FILE_TRIM_FAILED;
        }

        remainedSize = trimReqSize - ioSize;
        if (remainedSize <= ioSize)
        {
            ioSize = remainedSize;
        }
    }

    // move back cursor
    uint64_t startPos = startLpn * MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE; // 4032
    off_t off = lseek(fd_[media], startPos, SEEK_SET);

    if (off == -1)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_MEDIA_SEEK_FAILED,
            "lseek failed.");

        return POS_EVENT_ID::MFS_MEDIA_SEEK_FAILED;
    }

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Trim completed");

    return POS_EVENT_ID::SUCCESS;
}

LogicalBlkAddr
MssRamdisk::TranslateAddress(MetaStorageType type, MetaLpnType theLpn)
{
    LogicalBlkAddr addr = {0, 0};

    return addr;
}
} // namespace pos

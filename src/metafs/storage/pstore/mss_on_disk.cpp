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

#include "src/metafs/storage/pstore/mss_on_disk.h"
#include "src/metafs/storage/pstore/mss_disk_inplace.h"
#include "src/metafs/config/metafs_config.h"
#include "src/metafs/log/metafs_log.h"
#include "src/include/array_config.h"

#include <list>
#include <utility>

#include "src/include/memory.h"

namespace pos
{
/**
 * Constructor
 */
MssOnDisk::MssOnDisk(int arrayId)
: MetaStorageSubsystem(arrayId)
{
    for (int i = 0; i < static_cast<int>(MetaStorageType::Max); i++)
    {
        mssDiskPlace.push_back(nullptr);
        totalBlks.push_back(0);
        metaCapacity.push_back(0);
        maxPageCntLimitPerIO.push_back(0);
    }
}

/**
 * Destructor
 */
MssOnDisk::~MssOnDisk(void)
{
    _Finalize();
}

void
MssOnDisk::_Finalize(void)
{
    for (int i = mssDiskPlace.size() - 1; i >=0 ; --i)
    {
        if (mssDiskPlace[i] != nullptr)
        {
            delete mssDiskPlace[i];
            mssDiskPlace[i]= nullptr;
        }
    }
}

/**
 * Create Meta Storage Using fault tolerance layer.
 *
 * @mediaType  NVRAM/SDD type of media
 * @capacity size of area to use for meta filesystem
 *
 * @return Success(0)/Failure(-1)
 */
POS_EVENT_ID
MssOnDisk::CreateMetaStore(int arrayId, MetaStorageType mediaType, uint64_t capacity, bool formatFlag)
{
    // 4KB aligned
    assert(capacity % MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES == 0);
    int index = static_cast<int>(mediaType);

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Mss pstore CreateMetaStore called...");
    if (mssDiskPlace[index] != nullptr)
    {
        MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "You attempt to create meta storage again. media type={}",
            (int)mediaType);
        Open();
        return POS_EVENT_ID::SUCCESS; // just return success
    }

    // Can also choose update type base on mediaType
    if (INPLACE)
    {
        mssDiskPlace[index] = new MssDiskInplace(arrayId, mediaType, capacity);
    }
    else
    {
        // out of place
    }
    if (mssDiskPlace[index] == nullptr)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_MEDIA_NULL,
            "Mss Disk Place is null");
        return POS_EVENT_ID::MFS_MEDIA_NULL;
    }

    metaCapacity[index] = mssDiskPlace[index]->GetMetaDiskCapacity();
    totalBlks[index] = metaCapacity[index] / ArrayConfig::BLOCK_SIZE_BYTE;
    maxPageCntLimitPerIO[index] = MAX_DATA_TRANSFER_BYTE_SIZE / ArrayConfig::BLOCK_SIZE_BYTE;

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Media type(ssd=0, nvram=1)={}, Total free blocks={}, Capacity={}",
        (int)mediaType, totalBlks[index], metaCapacity[index]);

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Meta storage mgmt is opened");

    if (formatFlag)
    {
        // Format(mediaType, capacity);
    }

    Open();

    retryIoCnt = 0;

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MssOnDisk::Open(void)
{
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Meta storage mgmt is ready");

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MssOnDisk::Close(void)
{
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Meta storage mgmt is closed");

    _Finalize();

    return POS_EVENT_ID::SUCCESS;
}

/**
 * Get Total Meta Capacity
  
 * @mediaType  NVRAM/SDD type of media
 * 
 * @return capacity of area reserved for meta
 */

uint64_t
MssOnDisk::GetCapacity(MetaStorageType mediaType)
{
    int index = static_cast<int>(mediaType);
    return metaCapacity[index];
}

POS_EVENT_ID
MssOnDisk::_SendSyncRequest(IODirection direction, MetaStorageType mediaType, MetaLpnType pageNumber, MetaLpnType numPages, void* buffer)
{
    POS_EVENT_ID status = POS_EVENT_ID::SUCCESS;
    int index = static_cast<int>(mediaType);

    if (true == _CheckSanityErr(pageNumber, totalBlks[index]))
    {
        return POS_EVENT_ID::MFS_INVALID_PARAMETER;
    }

    _AdjustPageIoToFitTargetPartition(mediaType, pageNumber, numPages);

    MssDiskPlace* storagelld = mssDiskPlace[(int)mediaType];

    // per-stripe io
    MetaLpnType currLpn = pageNumber, currLpnCnt = numPages;
    const MetaLpnType maxLpnCntPerIO = storagelld->GetMaxLpnCntPerIOSubmit();
    uint8_t* currBuf = static_cast<uint8_t*>(buffer);
    while (numPages > 0)
    {
        pos::LogicalBlkAddr blkAddr =
            storagelld->CalculateOnDiskAddress(currLpn); // get physical address
        currLpnCnt = (numPages > (maxLpnCntPerIO - blkAddr.offset)) ? (maxLpnCntPerIO - blkAddr.offset) : numPages;

        BufferEntry buffEntry(currBuf, currLpnCnt);
        std::list<BufferEntry> bufferList;
        bufferList.push_back(buffEntry);

        IOSubmitHandlerStatus ioStatus;
        do
        {
            ioStatus = IIOSubmitHandler::GetInstance()->SyncIO(direction, bufferList,
                blkAddr, currLpnCnt, storagelld->GetPartitionType(), arrayId);

            if (ioStatus == IOSubmitHandlerStatus::SUCCESS ||
                ioStatus == IOSubmitHandlerStatus::FAIL_IN_SYSTEM_STOP)
            {
                break;
            }
            else
            {
                // IOSubmitHandlerStatus::FAIL
                // IOSubmitHandlerStatus::TRYLOCK_FAIL
                MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
                    "[SyncReq] Retry I/O Submit Hanlder...");
            }
        } while (1);

        if (ioStatus == IOSubmitHandlerStatus::FAIL_IN_SYSTEM_STOP)
        {
            MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
                "[MFS IO FAIL] Fail I/O Dut to System Stop State");

            status = POS_EVENT_ID::MFS_IO_FAILED_DUE_TO_STOP_STATE;

            break;
        }

        currLpn += currLpnCnt;
        if (IODirection::TRIM != direction)
        {
            currBuf += (currLpnCnt * MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES);
        }
        numPages -= currLpnCnt;
    }

    return status;
}

/**
 * read given page into buffer form disk
 *
 * @mediaType  NVRAM/SDD type of media
 * @pageNumber     Address of page to read. Must be 4K aligned.
 * @buffer  Destination memory to write data.
 * @numPages   Number of bytes in multiple of 4K pages.
 *
 * @return  Success(0)/Failure(-1)
 */

POS_EVENT_ID
MssOnDisk::ReadPage(MetaStorageType mediaType, MetaLpnType pageNumber, void* buffer, MetaLpnType numPages)
{
    return _SendSyncRequest(IODirection::READ, mediaType, pageNumber, numPages, buffer);
}

/**
 * Write given page from buffer to disk
 *
 * @mediaType  NVRAM/SDD type of media
 * @pageNumber    Address of page to write. Must be 4K aligned.
 * @buffer  Source memory to read data.
 * @numPages  Number of bytes in multiple of 4K pages.
 *
 * @return  Success(0)/Failure(-1)
 */

POS_EVENT_ID
MssOnDisk::WritePage(MetaStorageType mediaType, MetaLpnType pageNumber, void* buffer, MetaLpnType numPages)
{
    return _SendSyncRequest(IODirection::WRITE, mediaType, pageNumber, numPages, buffer);
}

void
MssOnDisk::_AdjustPageIoToFitTargetPartition(MetaStorageType mediaType, MetaLpnType& targetPage, MetaLpnType& targetNumPages)
{
    targetPage = targetPage * MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES / ArrayConfig::BLOCK_SIZE_BYTE;
    targetNumPages = targetNumPages * MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES / ArrayConfig::BLOCK_SIZE_BYTE;
}

bool
MssOnDisk::_CheckSanityErr(MetaLpnType pageNumber, uint64_t arrayCapacity)
{
    if (pageNumber >= arrayCapacity)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_ERROR_MESSAGE,
            "Out of boundary: pageNumber={}, totalBlks={}",
            pageNumber, arrayCapacity);
        return true;
    }

    return false;
}

POS_EVENT_ID
MssOnDisk::_SendAsyncRequest(IODirection direction, MssAioCbCxt* cb)
{
    MssAioData* aioData = reinterpret_cast<MssAioData*>(cb->GetAsycCbCxt());
    assert(aioData != nullptr);

    MetaLpnType pageNumber = aioData->metaLpn;
    void* buffer = aioData->buf;
    MetaLpnType numPages = aioData->lpnCnt;
    MetaStorageType mediaType = aioData->media;
    POS_EVENT_ID status = POS_EVENT_ID::SUCCESS;
    int index = static_cast<int>(mediaType);

    assert(pos::BLOCK_SIZE == MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES);

    if (true == _CheckSanityErr(pageNumber, totalBlks[index]))
    {
        return POS_EVENT_ID::MFS_INVALID_PARAMETER;
    }
    _AdjustPageIoToFitTargetPartition(mediaType, pageNumber, numPages);

    assert(numPages == 1);
    // FIXME: when async for multi-pages gets supported,
    // code below has to be revisited in order to send aio per stripe basis

    MssDiskPlace* storagelld = mssDiskPlace[(int)mediaType];

    CallbackSmartPtr callback(new MssIoCompletion(cb));

    BufferEntry buffEntry(buffer, 1 /*4KB*/);
    std::list<BufferEntry> bufferList;
    bufferList.push_back(buffEntry);

    pos::LogicalBlkAddr blkAddr =
        storagelld->CalculateOnDiskAddress(pageNumber); // get physical address

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[MssDisk][SendReq ] type={}, req.tagId={}, mpio_id={}, stripe={}, offsetInDisk={}, buf[0]={}",
        (int)direction, aioData->tagId, aioData->mpioId,
        blkAddr.stripeId, blkAddr.offset, *(uint32_t*)buffer);

    retryIoCnt = 0;
    IOSubmitHandlerStatus ioStatus;
    do
    {
        ioStatus = IIOSubmitHandler::GetInstance()->SubmitAsyncIO(direction, bufferList,
            blkAddr, numPages /*4KB*/, storagelld->GetPartitionType(),
            callback, arrayId);
        if (ioStatus == IOSubmitHandlerStatus::SUCCESS ||
            ioStatus == IOSubmitHandlerStatus::FAIL_IN_SYSTEM_STOP)
        {
            retryIoCnt = 0;
            break;
        }

        if (ioStatus == IOSubmitHandlerStatus::TRYLOCK_FAIL ||
            ioStatus == IOSubmitHandlerStatus::FAIL)
        {
            MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
                "[AsyncReq] Retry I/O Submit Hanlder... Cnt={}",
                retryIoCnt);
            retryIoCnt++;
        }
    } while (1);

    if (ioStatus == IOSubmitHandlerStatus::FAIL_IN_SYSTEM_STOP)
    {
        MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "[MFS IO FAIL] Fail I/O Dut to System Stop State, type={}, req.tagId={}, mpio_id={}",
            (int)direction, aioData->tagId, aioData->mpioId);

        status = POS_EVENT_ID::MFS_IO_FAILED_DUE_TO_STOP_STATE;
    }

    return status;
}

LogicalBlkAddr
MssOnDisk::TranslateAddress(MetaStorageType type, MetaLpnType theLpn)
{
    MssDiskPlace* storagelld = mssDiskPlace[(int)type];
    pos::LogicalBlkAddr blkAddr = storagelld->CalculateOnDiskAddress(theLpn);

    return blkAddr;
}

std::vector<MssDiskPlace*>&
MssOnDisk::GetMssDiskPlace(void)
{
    return mssDiskPlace;
}

bool
MssOnDisk::IsAIOSupport(void)
{
    return true;
}

/**
 * Issue Asynchornous read call to read page from disk
 *
 * @mediaType  NVRAM/SDD type of media
 * @pageNumber page number to write
 * @buffer output buffer for data that we are reading from disk
 * @numPages number of pages to write
 * @callback notification mechanisam for request's status
 *
 * @return Success(0)/Failure(-1)
 */
POS_EVENT_ID
MssOnDisk::ReadPageAsync(MssAioCbCxt* cb)
{
    return _SendAsyncRequest(IODirection::READ, cb);
}

/**
 * Issue Asynchornous write call to write page to disk
 *
 * @mediaType  NVRAM/SDD type of media
 * @pageNumber page number to write
 * @buffer input buffer data that need to be written to disk
 * @numPages number of pages to write
 * @callback notification mechanisam for request's status
 *
 * @return Success(0)/Failure(-1)
 */

POS_EVENT_ID
MssOnDisk::WritePageAsync(MssAioCbCxt* cb)
{
    return _SendAsyncRequest(IODirection::WRITE, cb);
}

POS_EVENT_ID
MssOnDisk::TrimFileData(MetaStorageType mediaType, MetaLpnType pageNumber, void* buffer, MetaLpnType numPages)
{
    return _SendSyncRequest(IODirection::TRIM, mediaType, pageNumber, numPages, buffer);
}
} // namespace pos

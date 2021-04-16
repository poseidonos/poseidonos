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

#include "mss_on_disk.h"

#include "metafs_config.h"
#include "metafs_log.h"
#include "mss_disk_inplace.h"
#include "src/include/array_config.h"
#ifdef LEGACY_IOPATH
#include "mss_disk_handler.h"
#endif

#include <list>
#include <utility>

#include "src/include/memory.h"

namespace pos
{
MssInfo::MssInfo(std::string& arrayName)
: arrayName(arrayName),
  retryIoCnt(0)
{
    for (int i = 0; i < static_cast<int>(MetaStorageType::Max); i++)
    {
        mssDiskPlace.push_back(nullptr);
        totalBlks.push_back(0);
        metaCapacity.push_back(0);
        maxPageCntLimitPerIO.push_back(0);
    }
}

MssInfo::~MssInfo(void)
{
    for (int i = 0; i < static_cast<int>(MetaStorageType::Max); i++)
    {
        if (mssDiskPlace[i] != nullptr)
        {
            delete mssDiskPlace[i];
            mssDiskPlace[i]= nullptr;
        }
    }
}

/**
 * Constructor
 */
MssOnDisk::MssOnDisk(void)
: mssBitmap(nullptr)
{
    for (uint32_t i = 0; i < MetaFsConfig::MAX_ARRAY_CNT; i++)
    {
        mssInfo[i] = nullptr;
    }
    mssBitmap = new BitMap(MetaFsConfig::MAX_ARRAY_CNT);
    mssMap.clear();
}

/**
 * Destructor
 */
MssOnDisk::~MssOnDisk(void)
{
    _FinalizeAll();

    if (nullptr != mssBitmap)
        delete mssBitmap;

    _SetState(MetaSsState::Close);
}

void
MssOnDisk::_Finalize(std::string arrayName)
{
    auto it = mssMap.find(arrayName);
    assert(it != mssMap.end());

    uint32_t index = it->second;
    mssBitmap->ClearBit(index);

    mssMap.erase(it);
    delete mssInfo[index];
    mssInfo[index]= nullptr;
}

void
MssOnDisk::_FinalizeAll(void)
{
    for (uint32_t i = 0; i < MetaFsConfig::MAX_ARRAY_CNT; i++)
    {
        if (mssInfo[i] != nullptr)
        {
            delete mssInfo[i];
            mssInfo[i]= nullptr;
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
MssOnDisk::CreateMetaStore(std::string arrayName, MetaStorageType mediaType, uint64_t capacity, bool formatFlag)
{
    uint32_t mssIndex = 0;
    auto it = mssMap.find(arrayName);
    if (it == mssMap.end())
    {
        mssIndex = mssBitmap->FindFirstZero();
        mssBitmap->SetBit(mssIndex);
        mssInfo[mssIndex] = new MssInfo(arrayName);
    }
    else
    {
        mssIndex = it->second;
    }

    // 4KB aligned
    assert(capacity % MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES == 0);
    int index = static_cast<int>(mediaType);

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Mss pstore CreateMetaStore called...");
    if (mssInfo[mssIndex]->mssDiskPlace[index] != nullptr)
    {
        MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "You attempt to create meta storage again. media type={}",
            (int)mediaType);
        Open(arrayName);
        return POS_EVENT_ID::SUCCESS; // just return success
    }

    // Can also choose update type base on mediaType
    if (INPLACE)
    {
        mssInfo[mssIndex]->mssDiskPlace[index] = new MssDiskInplace(arrayName, mediaType, capacity);
    }
    else
    {
        // out of place
    }
    if (mssInfo[mssIndex]->mssDiskPlace[index] == nullptr)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_MEDIA_NULL,
            "Mss Disk Place is null");
        return POS_EVENT_ID::MFS_MEDIA_NULL;
    }

    mssInfo[mssIndex]->metaCapacity[index] = mssInfo[mssIndex]->mssDiskPlace[index]->GetMetaDiskCapacity();
    mssInfo[mssIndex]->totalBlks[index] = mssInfo[mssIndex]->metaCapacity[index] / ArrayConfig::BLOCK_SIZE_BYTE;
    mssInfo[mssIndex]->maxPageCntLimitPerIO[index] = MAX_DATA_TRANSFER_BYTE_SIZE / ArrayConfig::BLOCK_SIZE_BYTE;

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Media type(ssd=0, nvram=1)={}, Total free blocks={}, Capacity={}",
        (int)mediaType, totalBlks[index], metaCapacity[index]);

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Meta storage mgmt is opened");
    _SetState(MetaSsState::Ready);

    if (formatFlag)
    {
        // Format(mediaType, capacity);
    }

    mssMap.insert(std::pair<std::string, uint32_t>(arrayName, mssIndex));

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MssOnDisk::Open(std::string arrayName)
{
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Meta storage mgmt is ready");
    _SetState(MetaSsState::Ready);

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MssOnDisk::Close(std::string arrayName)
{
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Meta storage mgmt is closed");

    _Finalize(arrayName);

    bool isTheLast = (0 == mssMap.size()) ? true : false;
    if (isTheLast)
        _SetState(MetaSsState::Close);

    return POS_EVENT_ID::SUCCESS;
}

/**
 * Get Total Meta Capacity
  
 * @mediaType  NVRAM/SDD type of media
 * 
 * @return capacity of area reserved for meta
 */

uint64_t
MssOnDisk::GetCapacity(std::string arrayName, MetaStorageType mediaType)
{
    int index = static_cast<int>(mediaType);
    auto it = mssMap.find(arrayName);
    assert(it != mssMap.end());
    return mssInfo[it->second]->metaCapacity[index];
}

POS_EVENT_ID
MssOnDisk::_SendSyncRequest(IODirection direction, std::string arrayName, MetaStorageType mediaType, MetaLpnType pageNumber, MetaLpnType numPages, void* buffer)
{
    POS_EVENT_ID status = POS_EVENT_ID::SUCCESS;
    int index = static_cast<int>(mediaType);
    auto it = mssMap.find(arrayName);
    assert(it != mssMap.end());

    if (true == _CheckSanityErr(arrayName, pageNumber, mssInfo[it->second]->totalBlks[index]))
    {
        return POS_EVENT_ID::MFS_INVALID_PARAMETER;
    }

    _AdjustPageIoToFitTargetPartition(mediaType, pageNumber, numPages);

    MssDiskPlace* storagelld = mssInfo[it->second]->mssDiskPlace[(int)mediaType];

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
                blkAddr, currLpnCnt, storagelld->GetPartitionType(), arrayName);

            if (ioStatus == IOSubmitHandlerStatus::SUCCESS ||
                ioStatus == IOSubmitHandlerStatus::FAIL_IN_SYSTEM_STOP)
            {
                break;
            }

            if (ioStatus == IOSubmitHandlerStatus::TRYLOCK_FAIL ||
                ioStatus == IOSubmitHandlerStatus::FAIL)
            {
                if (IODirection::TRIM == direction)
                {
                    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
                        "[SyncReq] Trim failed");
                    status = POS_EVENT_ID::MFS_FILE_TRIM_FAILED;
                    break;
                }
                else
                {
                    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
                        "[SyncReq] Retry I/O Submit Hanlder...");
                }
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
        currBuf += (currLpnCnt * MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES);
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
MssOnDisk::ReadPage(std::string arrayName, MetaStorageType mediaType, MetaLpnType pageNumber, void* buffer, MetaLpnType numPages)
{
    return _SendSyncRequest(IODirection::READ, arrayName, mediaType, pageNumber, numPages, buffer);
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
MssOnDisk::WritePage(std::string arrayName, MetaStorageType mediaType, MetaLpnType pageNumber, void* buffer, MetaLpnType numPages)
{
    return _SendSyncRequest(IODirection::WRITE, arrayName, mediaType, pageNumber, numPages, buffer);
}

void
MssOnDisk::_AdjustPageIoToFitTargetPartition(MetaStorageType mediaType, MetaLpnType& targetPage, MetaLpnType& targetNumPages)
{
    targetPage = targetPage * MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES / ArrayConfig::BLOCK_SIZE_BYTE;
    targetNumPages = targetNumPages * MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES / ArrayConfig::BLOCK_SIZE_BYTE;
}

bool
MssOnDisk::_CheckSanityErr(std::string arrayName, MetaLpnType pageNumber, uint64_t arrayCapacity)
{
    if (pageNumber >= arrayCapacity)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_ERROR_MESSAGE,
            "Out of boundary: pageNumber={}, totalBlks={}",
            pageNumber, arrayCapacity);
        return true;
    }
    if (!IsReady())
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_ERROR_MESSAGE,
            "IO failed. Meta storage subsystem is not ready yet.");
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
    auto it = mssMap.find(cb->GetArrayName());
    assert(it != mssMap.end());

    assert(pos::BLOCK_SIZE == MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES);

    if (true == _CheckSanityErr(cb->GetArrayName(), pageNumber, mssInfo[it->second]->totalBlks[index]))
    {
        return POS_EVENT_ID::MFS_INVALID_PARAMETER;
    }
    _AdjustPageIoToFitTargetPartition(mediaType, pageNumber, numPages);

    assert(numPages == 1);
    // FIXME: when async for multi-pages gets supported,
    // code below has to be revisited in order to send aio per stripe basis

    MssDiskPlace* storagelld = mssInfo[it->second]->mssDiskPlace[(int)mediaType];

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

    mssInfo[it->second]->retryIoCnt = 0;
    IOSubmitHandlerStatus ioStatus;
    do
    {
        ioStatus = IIOSubmitHandler::GetInstance()->SubmitAsyncIO(direction, bufferList,
            blkAddr, numPages /*4KB*/, storagelld->GetPartitionType(),
            callback, cb->GetArrayName());
        if (ioStatus == IOSubmitHandlerStatus::SUCCESS ||
            ioStatus == IOSubmitHandlerStatus::FAIL_IN_SYSTEM_STOP)
        {
            mssInfo[it->second]->retryIoCnt = 0;
            break;
        }

        if (ioStatus == IOSubmitHandlerStatus::TRYLOCK_FAIL ||
            ioStatus == IOSubmitHandlerStatus::FAIL)
        {
            MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
                "[AsyncReq] Retry I/O Submit Hanlder... Cnt={}",
                retryIoCnt);
            mssInfo[it->second]->retryIoCnt++;
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

bool
MssOnDisk::IsAIOSupport()
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
MssOnDisk::TrimFileData(std::string arrayName, MetaStorageType mediaType, MetaLpnType pageNumber, void* buffer, MetaLpnType numPages)
{
    return _SendSyncRequest(IODirection::TRIM, arrayName, mediaType, pageNumber, numPages, buffer);
}
} // namespace pos

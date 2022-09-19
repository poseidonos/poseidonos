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

#include "mss_on_disk.h"

#include <algorithm>
#include <list>
#include <utility>

#include "src/include/array_config.h"
#include "src/metafs/config/metafs_config.h"
#include "src/metafs/log/metafs_log.h"
#include "src/metafs/storage/pstore/issue_write_event.h"
#include "src/metafs/storage/pstore/mss_disk_inplace.h"

namespace pos
{
MssOnDisk::MssOnDisk(const int arrayId)
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

MssOnDisk::~MssOnDisk(void)
{
    _Finalize();
}

void
MssOnDisk::_Finalize(void)
{
    for (size_t i = 0; i < mssDiskPlace.size(); ++i)
    {
        if (mssDiskPlace[i] != nullptr)
        {
            delete mssDiskPlace[i];
            mssDiskPlace[i] = nullptr;
        }
    }
}

POS_EVENT_ID
MssOnDisk::CreateMetaStore(const int arrayId, const MetaStorageType mediaType,
    const uint64_t capacity, const bool formatFlag)
{
    if (capacity % MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES)
    {
        POS_TRACE_ERROR(EID(MFS_INVALID_PARAMETER),
            "The capacity should be aligned to 4kb, capacity: {}", capacity);
        assert(0);
    }

    const int index = static_cast<int>(mediaType);

    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "Mss pstore CreateMetaStore called...");

    if (mssDiskPlace[index])
    {
        MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
            "You attempt to create meta storage again. mediaType: {}",
            index);
        Open();
        return EID(SUCCESS); // just return success
    }

    // Can also choose update type base on mediaType
    if (INPLACE)
    {
        mssDiskPlace[index] = new MssDiskInplace(arrayId, mediaType, capacity);
    }
    else
    {
        POS_TRACE_ERROR(EID(MFS_ERROR_MESSAGE),
            "Invalid Option");
        assert(0);
    }

    metaCapacity[index] = mssDiskPlace[index]->GetMetaDiskCapacity();
    totalBlks[index] = metaCapacity[index] / ArrayConfig::BLOCK_SIZE_BYTE;
    maxPageCntLimitPerIO[index] = MAX_DATA_TRANSFER_BYTE_SIZE / ArrayConfig::BLOCK_SIZE_BYTE;

    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "Media type(ssd=0, nvram=1)={}, Total free blocks={}, Capacity={}",
        (int)mediaType, totalBlks[index], metaCapacity[index]);

    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "Meta storage mgmt is opened");

    if (formatFlag)
    {
        // TODO (munseop.lim): need to implement
        POS_TRACE_WARN(EID(MFS_WARNING_MESSAGE),
            "formatFlag isn't supported yet");
    }

    Open();

    return EID(SUCCESS);
}

POS_EVENT_ID
MssOnDisk::Open(void)
{
    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "Meta storage mgmt is ready");

    return EID(SUCCESS);
}

POS_EVENT_ID
MssOnDisk::Close(void)
{
    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "Meta storage mgmt is closed");

    _Finalize();

    return EID(SUCCESS);
}

uint64_t
MssOnDisk::GetCapacity(const MetaStorageType mediaType)
{
    int index = static_cast<int>(mediaType);
    return metaCapacity[index];
}

POS_EVENT_ID
MssOnDisk::_SendSyncRequest(const IODirection direction, const MetaStorageType mediaType,
    const MetaLpnType pageNumber, const MetaLpnType numPages, void* buffer)
{
    MetaLpnType currLpn = pageNumber;
    MetaLpnType requestLpnCount = numPages;

    if (_CheckSanityErr(currLpn, totalBlks[static_cast<int>(mediaType)]))
    {
        return EID(MFS_INVALID_PARAMETER);
    }

    _AdjustPageIoToFitTargetPartition(mediaType, currLpn, requestLpnCount);

    MssDiskPlace* storagelld = mssDiskPlace[(int)mediaType];

    const MetaLpnType maxLpnCntPerIO = storagelld->GetMaxLpnCntPerIOSubmit();
    uint8_t* currBuf = static_cast<uint8_t*>(buffer);
    while (requestLpnCount > 0)
    {
        pos::LogicalBlkAddr blkAddr =
            storagelld->CalculateOnDiskAddress(currLpn); // get physical address
        MetaLpnType currLpnCnt = std::min(requestLpnCount, maxLpnCntPerIO - blkAddr.offset);

        std::list<BufferEntry> bufferList = _GetBufferList(mediaType, blkAddr.offset, currLpnCnt, currBuf);
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
                MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
                    "[SyncReq] Retry I/O Submit Hanlder...");
            }
        } while (1);

        if (ioStatus == IOSubmitHandlerStatus::FAIL_IN_SYSTEM_STOP)
        {
            MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
                "[MFS IO FAIL] Fail I/O Dut to System Stop State");

            return EID(MFS_IO_FAILED_DUE_TO_STOP_STATE);
        }

        if (IODirection::TRIM != direction)
        {
            currBuf += (currLpnCnt * MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES);
        }
        currLpn += currLpnCnt;
        requestLpnCount -= currLpnCnt;
    }

    return EID(SUCCESS);
}

std::list<BufferEntry>
MssOnDisk::_GetBufferList(const MetaStorageType mediaType, const uint64_t offset, const uint64_t count, uint8_t* buffer)
{
    std::list<BufferEntry> list;
    const uint64_t BLOCKS_PER_CHUNK = mssDiskPlace[(int)mediaType]->GetLpnCntPerChunk();
    const uint64_t START_CHUNK = offset / BLOCKS_PER_CHUNK;
    const uint64_t LAST_CHUNK = (offset + count - 1) / BLOCKS_PER_CHUNK;
    uint64_t blocksInThisChunk = (START_CHUNK == LAST_CHUNK) ? count : (((START_CHUNK + 1) * BLOCKS_PER_CHUNK) - offset);
    uint8_t* currentBuffer = buffer;
    uint64_t remainBlocks = count;

    for (uint64_t i = START_CHUNK; i <= LAST_CHUNK; ++i)
    {
        BufferEntry buffEntry(currentBuffer, blocksInThisChunk);
        list.push_back(buffEntry);
        currentBuffer += (blocksInThisChunk * MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES);
        remainBlocks -= blocksInThisChunk;
        blocksInThisChunk = std::min(BLOCKS_PER_CHUNK, remainBlocks);
    }

    return list;
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
MssOnDisk::ReadPage(const MetaStorageType mediaType, const MetaLpnType pageNumber,
    void* buffer, const MetaLpnType numPages)
{
    return _SendSyncRequest(IODirection::READ, mediaType, pageNumber, numPages, buffer);
}

POS_EVENT_ID
MssOnDisk::WritePage(const MetaStorageType mediaType, const MetaLpnType pageNumber,
    void* buffer, const MetaLpnType numPages)
{
    return _SendSyncRequest(IODirection::WRITE, mediaType, pageNumber, numPages, buffer);
}

void
MssOnDisk::_AdjustPageIoToFitTargetPartition(const MetaStorageType mediaType,
    MetaLpnType& targetPage, MetaLpnType& targetNumPages)
{
    targetPage = targetPage * MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES / ArrayConfig::BLOCK_SIZE_BYTE;
    targetNumPages = targetNumPages * MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES / ArrayConfig::BLOCK_SIZE_BYTE;
}

bool
MssOnDisk::_CheckSanityErr(const MetaLpnType pageNumber, const uint64_t arrayCapacity)
{
    if (pageNumber >= arrayCapacity)
    {
        MFS_TRACE_ERROR(EID(MFS_ERROR_MESSAGE),
            "Out of boundary: pageNumber={}, totalBlks={}",
            pageNumber, arrayCapacity);
        return true;
    }

    return false;
}

POS_EVENT_ID
MssOnDisk::_SendAsyncRequest(const IODirection direction, MssAioCbCxt* cb)
{
    MssAioData* aioData = reinterpret_cast<MssAioData*>(cb->GetAsycCbCxt());

    void* buffer = aioData->GetBuffer();
    MetaLpnType startLpn = aioData->GetMetaLpn();
    MetaLpnType requestLpnCount = aioData->GetLpnCount();
    MetaStorageType mediaType = aioData->GetStorageType();
    POS_EVENT_ID status = EID(SUCCESS);
    const bool isJournal = (mediaType != MetaStorageType::SSD) ? true : false;

    assert(pos::BLOCK_SIZE == MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES);

    if (true == _CheckSanityErr(startLpn, totalBlks[(int)mediaType]))
    {
        return EID(MFS_INVALID_PARAMETER);
    }

    _AdjustPageIoToFitTargetPartition(mediaType, startLpn, requestLpnCount);

    if (requestLpnCount != 1)
    {
        // TODO (munseop.lim): need to support multi-pages IO
        POS_TRACE_ERROR(EID(MFS_ERROR_MESSAGE),
            "MetaFs does not support multi-pages IO.");
        assert(false);
    }

    MssDiskPlace* storagelld = mssDiskPlace[(int)mediaType];

    CallbackSmartPtr callback(new MssIoCompletion(cb, isJournal));

    BufferEntry buffEntry(buffer, 1 /*4KB*/);
    std::list<BufferEntry> bufferList;
    bufferList.push_back(buffEntry);

    pos::LogicalBlkAddr blkAddr =
        storagelld->CalculateOnDiskAddress(startLpn); // get physical address

    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "[MssDisk][SendReq ] type={}, req.tagId={}, mpio_id={}, stripe={}, offsetInDisk={}, buf[0]={}",
        (int)direction, aioData->GetTagId(), aioData->GetMpioId(),
        blkAddr.stripeId, blkAddr.offset, *(uint32_t*)buffer);

    IOSubmitHandlerStatus ioStatus = IIOSubmitHandler::GetInstance()->SubmitAsyncIO(direction, bufferList,
            blkAddr, requestLpnCount, storagelld->GetPartitionType(), callback, arrayId);

    if (ioStatus == IOSubmitHandlerStatus::FAIL_IN_SYSTEM_STOP)
    {
        MFS_TRACE_DEBUG(EID(MFS_IO_FAILED_DUE_TO_STOP_STATE),
            "[MFS IO FAIL] Fail I/O Due to System Stop State, type={}, req.tagId={}, mpio_id={}",
            (int)direction, aioData->GetTagId(), aioData->GetMpioId());

        status = EID(MFS_IO_FAILED_DUE_TO_STOP_STATE);
    }
    else if (ioStatus == IOSubmitHandlerStatus::TRYLOCK_FAIL)
    {
        MFS_TRACE_DEBUG(EID(MFS_IO_FAILED_DUE_TO_TRYLOCK_FAIL),
            "[MFS IO FAIL] Trylock failed, type={}, req.tagId={}, mpio_id={}",
            (int)direction, aioData->GetTagId(), aioData->GetMpioId());

        status = EID(MFS_IO_FAILED_DUE_TO_TRYLOCK_FAIL);
    }
    else if (ioStatus == IOSubmitHandlerStatus::FAIL)
    {
        POS_TRACE_DEBUG(EID(MFS_IO_FAILED_DUE_TO_BUSY),
            "[MFS IO FAIL] Submission failed, type={}, req.tagId={}, mpio_id={}",
            (int)direction, aioData->GetTagId(), aioData->GetMpioId());

        status = EID(MFS_IO_FAILED_DUE_TO_BUSY);
    }

    return status;
}

LogicalBlkAddr
MssOnDisk::TranslateAddress(const MetaStorageType type, const MetaLpnType theLpn)
{
    return mssDiskPlace[(int)type]->CalculateOnDiskAddress(theLpn);
}

POS_EVENT_ID
MssOnDisk::ReadPageAsync(MssAioCbCxt* cb)
{
    return _SendAsyncRequest(IODirection::READ, cb);
}

POS_EVENT_ID
MssOnDisk::WritePageAsync(MssAioCbCxt* cb)
{
    MssAioData* aioData = reinterpret_cast<MssAioData*>(cb->GetAsycCbCxt());
    if (aioData->GetStorageType() == MetaStorageType::SSD)
    {
        MssRequestFunction handler = std::bind(&MssOnDisk::_SendAsyncRequest, this, std::placeholders::_1, std::placeholders::_2);
        EventSmartPtr event = std::make_shared<IssueWriteEvent>(handler, cb);
        EventSchedulerSingleton::Instance()->EnqueueEvent(event);
        return EID(SUCCESS);
    }
    else
    {
        return _SendAsyncRequest(IODirection::WRITE, cb);
    }
}

POS_EVENT_ID
MssOnDisk::TrimFileData(const MetaStorageType mediaType, const MetaLpnType pageNumber,
    void* buffer, const MetaLpnType numPages)
{
    return _SendSyncRequest(IODirection::TRIM, mediaType, pageNumber, numPages, buffer);
}
} // namespace pos

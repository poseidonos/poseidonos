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

#include "stripe_partition.h"

#include <cassert>
#include <iostream>
#include <list>
#include <string>
#include <vector>

#include "../config/array_config.h"
#include "../device/array_device.h"
#include "../ft/method.h"
#include "../ft/rebuild_read_complete_handler.h"
#include "../ft/rebuild_read_intermediate_complete_handler.h"
#include "../ft/rebuilder.h"
#include "src/allocator/allocator.h"
#include "src/device/ublock_device.h"
#include "src/device/unvme/unvme_ssd.h"
#include "src/include/ibof_event_id.h"
#include "src/io/general_io/block_alignment.h"
#include "src/io/general_io/ubio.h"
#include "src/logger/logger.h"
#include "src/scheduler/event_argument.h"
#include "src/state/state_manager.h"

namespace ibofos
{
const char* PARTITION_TYPE_STR[4] = {
    "META_NVM",
    "WRITE_BUFFER",
    "META_SSD",
    "USER_DATA"};

StripePartition::StripePartition(PartitionType type,
    PartitionPhysicalSize physicalSize,
    vector<ArrayDevice*> devs,
    Method* method)
: Partition(type, physicalSize, devs, method)
{
    _SetLogicalSize();
}

int
StripePartition::_SetLogicalSize()
{
    const FtSizeInfo* ftSize = nullptr;
    ftSize = method_->GetSizeInfo();
    if (nullptr == ftSize)
    {
        return -1;
    }
    logicalSize_ = {
        .minWriteBlkCnt = ftSize->minWriteBlkCnt,
        .blksPerChunk = ftSize->blksPerChunk,
        .blksPerStripe = ftSize->blksPerStripe - ftSize->backupBlkCnt,
        .chunksPerStripe = ftSize->chunksPerStripe -
            (ftSize->backupBlkCnt / ftSize->blksPerChunk),
        .stripesPerSegment = physicalSize_.stripesPerSegment,
        .totalStripes =
            physicalSize_.stripesPerSegment * physicalSize_.totalSegments,
        .totalSegments = physicalSize_.totalSegments};
    return 0;
}

StripePartition::~StripePartition()
{
}

int
StripePartition::Translate(PhysicalBlkAddr& dst, const LogicalBlkAddr& src)
{
    if (false == _IsValidAddress(src))
    {
        int error = (int)IBOF_EVENT_ID::ARRAY_INVALID_ADDRESS_ERROR;
        IBOF_TRACE_ERROR(error, "Invalid Address Error");
        return error;
    }

    FtBlkAddr fsa;
    method_->Translate(fsa, src);

    dst = _F2PTranslate(fsa);
    return 0;
}

PhysicalBlkAddr
StripePartition::_F2PTranslate(const FtBlkAddr& fsa)
{
    PhysicalBlkAddr psa;
    uint32_t chunkIndex = fsa.offset / physicalSize_.blksPerChunk;
    psa.dev = devs_.at(chunkIndex);
    psa.lba = physicalSize_.startLba +
        (fsa.stripeId * physicalSize_.blksPerChunk + fsa.offset % physicalSize_.blksPerChunk) * ArrayConfig::SECTORS_PER_BLOCK;

    return psa;
}

FtBlkAddr
StripePartition::_P2FTranslate(const PhysicalBlkAddr& pba)
{
    int chunkIndex = -1;
    for (uint32_t i = 0; i < devs_.size(); i++)
    {
        if (pba.dev == devs_.at(i))
        {
            chunkIndex = i;
            break;
        }
    }
    if (-1 == chunkIndex)
    {
        assert(0);
        // TODO Error
    }

    uint64_t ptnBlkOffset =
        (pba.lba - physicalSize_.startLba) / ArrayConfig::SECTORS_PER_BLOCK;
    FtBlkAddr fsa = {
        .stripeId = (uint32_t)(ptnBlkOffset / physicalSize_.blksPerChunk),
        .offset =
            (chunkIndex * physicalSize_.blksPerChunk) + (ptnBlkOffset % physicalSize_.blksPerChunk)};

    return fsa;
}

int
StripePartition::Convert(list<PhysicalWriteEntry>& dst,
    const LogicalWriteEntry& src)
{
    if (false == _IsValidEntry(src))
    {
        int error = (int)IBOF_EVENT_ID::ARRAY_INVALID_ADDRESS_ERROR;
        IBOF_TRACE_ERROR(error, "Invalid Address Error");
        return error;
    }
    dst.clear();

    list<FtWriteEntry> ftEntries;
    method_->Convert(ftEntries, src);
    for (FtWriteEntry& ftEntry : ftEntries)
    {
        int ret = 0;
        ret = _ConvertToPhysical(dst, ftEntry);
        if (0 != ret)
        {
            // TODO
            assert(0);
        }
    }

    return 0;
}

list<PhysicalBlkAddr>
StripePartition::_GetRebuildGroup(FtBlkAddr fba)
{
    list<FtBlkAddr> ftAddrs = method_->GetRebuildGroup(fba);
    list<PhysicalBlkAddr> ret;
    for (FtBlkAddr fsa : ftAddrs)
    {
        PhysicalBlkAddr pba = _F2PTranslate(fsa);
        ret.push_back(pba);
    }
    return ret;
}

int
StripePartition::_ConvertToPhysical(list<PhysicalWriteEntry>& dst,
    FtWriteEntry& ftEntry)
{
    const uint32_t chunkSize = physicalSize_.blksPerChunk;

    const uint32_t firstOffset = ftEntry.addr.offset;
    const uint32_t lastOffset = firstOffset + ftEntry.blkCnt - 1;
    const uint32_t chunkStart = firstOffset / chunkSize;
    const uint32_t chunkEnd = lastOffset / chunkSize;

    const uint64_t stripeLba = physicalSize_.startLba + ((uint64_t)ftEntry.addr.stripeId * chunkSize * ArrayConfig::SECTORS_PER_BLOCK);

    uint32_t startOffset = firstOffset - chunkSize * chunkStart;
    uint32_t endOffset = lastOffset - chunkSize * chunkEnd;

    uint32_t bufferOffset = 0;
    for (uint32_t i = chunkStart; i <= chunkEnd; i++)
    {
        PhysicalWriteEntry physicalEntry;
        physicalEntry.addr = {
            .dev = devs_.at(i),
            .lba = stripeLba + startOffset * ArrayConfig::SECTORS_PER_BLOCK};
        if (i != chunkEnd)
        {
            physicalEntry.blkCnt = chunkSize - startOffset;
            startOffset = 0;
        }
        else
        {
            physicalEntry.blkCnt = endOffset - startOffset + 1;
        }
        physicalEntry.buffers =
            _SpliceBuffer(ftEntry.buffers, bufferOffset, physicalEntry.blkCnt);
        bufferOffset += physicalEntry.blkCnt;
        dst.push_back(physicalEntry);
    }

    return 0;
}

list<BufferEntry>
StripePartition::_SpliceBuffer(list<BufferEntry>& src, uint32_t start, uint32_t remain)
{
    using Block = char[ArrayConfig::BLOCK_SIZE_BYTE];
    list<BufferEntry> dst;

    uint32_t offset = 0;
    for (BufferEntry& buffer : src)
    {
        uint32_t blkCnt = buffer.GetBlkCnt();
        if (offset <= start && offset + blkCnt > start)
        {
            BufferEntry split = buffer;

            uint32_t gap = start - offset;
            split.SetBuffer(reinterpret_cast<Block*>(buffer.GetBufferEntry()) + gap);
            blkCnt = blkCnt - gap;
            if (blkCnt >= remain)
            {
                split.SetBlkCnt(remain);
                dst.push_back(split);
                break;
            }
            else
            {
                split.SetBlkCnt(blkCnt);
                remain -= blkCnt;
                dst.push_back(split);
            }
        }

        offset += buffer.GetBlkCnt();
    }
    return dst;
}

int
StripePartition::Rebuild(RebuildBehavior* behavior)
{
    using namespace std::placeholders;
    Restorer restorer = std::bind(&StripePartition::RebuildRead, this, _1);
    F2PTranslator translator = std::bind(&StripePartition::_F2PTranslate, this, _1);

    string partName = PARTITION_TYPE_STR[type_];
    RebuildContext* ctx = behavior->GetContext();
    ctx->id = partName;
    ctx->size = GetPhysicalSize();
    ctx->result = RebuildState::REBUILDING;
    ctx->restore = restorer;
    ctx->translate = translator;
    EventSmartPtr rebuilder(new Rebuilder(behavior));
    EventArgument::GetEventScheduler()->EnqueueEvent(rebuilder);
    return 0;
}

int
StripePartition::RebuildRead(UbioSmartPtr ubio)
{
    if (nullptr == ubio)
    {
        return -1;
    }
    // Chunk Aliging check
    const uint32_t sectorSize = ArrayConfig::SECTOR_SIZE_BYTE;
    const uint32_t sectorsPerBlock = ArrayConfig::SECTORS_PER_BLOCK;
    const uint32_t blockSize = ArrayConfig::BLOCK_SIZE_BYTE;

    PhysicalBlkAddr originPba = ubio->GetPba();
    BlockAlignment blockAlignment(originPba.lba * sectorSize, ubio->GetSize());
    originPba.lba = blockAlignment.GetHeadBlock() * sectorsPerBlock;
    FtBlkAddr fba = _P2FTranslate(originPba);
    list<PhysicalBlkAddr> rebuildGroup = _GetRebuildGroup(fba);

    uint32_t readSize = blockAlignment.GetBlockCount() * blockSize;
    uint32_t sectorCnt = rebuildGroup.size() * readSize / sectorSize;
    uint32_t memSize = sectorSize * sectorCnt;
    void* mem = Memory<sectorSize>::Alloc(sectorCnt);
    if (nullptr == mem)
    {
        // TODO : this will be considered after reference based handling
        return -1; // Memory Alloc failed
    }
    memset(mem, 0, memSize);

    UbioSmartPtr rebuildUbio(new Ubio(mem, sectorCnt));
    rebuildUbio->SetRetry(true);
    RebuildFunc rebuildFunc = method_->GetRebuildFunc();

    CallbackSmartPtr rebuildCompletion(
        new RebuildReadCompleteHandler(rebuildUbio, rebuildFunc, readSize));

    rebuildUbio->SetCallback(rebuildCompletion);
    rebuildUbio->SetOriginUbio(ubio);
#if defined QOS_ENABLED_BE
    rebuildCompletion->SetEventType(ubio->GetEventType());
    rebuildUbio->SetEventType(ubio->GetEventType());
#endif
    rebuildUbio->SetCallback(rebuildCompletion);
    rebuildUbio->SetOriginUbio(ubio);

    list<UbioSmartPtr> splitList;
    for (auto pba : rebuildGroup)
    {
        bool isTail = false;
        UbioSmartPtr split =
            rebuildUbio->Split(ChangeByteToSector(readSize), isTail);
        split->SetPba(pba);
        CallbackSmartPtr event(
            new RebuildReadIntermediateCompleteHandler(split));
        event->SetCallee(rebuildCompletion);
#if defined QOS_ENABLED_BE
        event->SetEventType(ubio->GetEventType());
        split->SetEventType(ubio->GetEventType());
#endif
        split->SetCallback(event);
        split->SetOriginUbio(rebuildUbio);
        splitList.push_back(split);
    }

    rebuildCompletion->SetWaitingCount(splitList.size());

    for (auto split : splitList)
    {
        IODispatcher* ioDispatcher = EventArgument::GetIODispatcher();
        ioDispatcher->Submit(split);
    }

    return 0;
}

void
StripePartition::Format()
{
    _Trim();
}

void
StripePartition::_Trim()
{
    uint32_t blkCount = GetPhysicalSize()->totalSegments *
        GetPhysicalSize()->stripesPerSegment *
        GetPhysicalSize()->blksPerChunk;
    uint64_t totalUnitCount = blkCount * Ubio::UNITS_PER_BLOCK;
    uint32_t unitCount;
    uint64_t startLba = GetPhysicalSize()->startLba;
    uint32_t ubioUnit = UINT32_MAX;
    int result = 0;
    int trimResult = 0;
    uint8_t dummyBuffer[Ubio::BYTES_PER_UNIT];

    do
    {
        if (totalUnitCount >= ubioUnit)
        {
            unitCount = ubioUnit;
        }
        else
        {
            unitCount = totalUnitCount % ubioUnit;
        }

        UbioSmartPtr ubio(new Ubio(dummyBuffer, unitCount));
        ubio->dir = UbioDir::Deallocate;

        for (uint32_t i = 0; i < devs_.size(); i++)
        {
            if (devs_[i]->GetState() == ArrayDeviceState::FAULT)
            {
                continue;
            }

            PhysicalBlkAddr pba = {.dev = devs_[i],
                .lba = startLba};
            ubio->SetPba(pba);
            IODispatcher* ioDispatcher = EventArgument::GetIODispatcher();
            result = ioDispatcher->Submit(ubio, true);
            IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::ARRAY_PARTITION_TRIM,
                "Try to trim from {} for {} on {}",
                pba.lba, unitCount, devs_[i]->uBlock->GetName());

            if (result < 0 || ubio->GetError() != CallbackError::SUCCESS)
            {
                IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::ARRAY_PARTITION_TRIM,
                    "Trim Failed on {}", devs_[i]->uBlock->GetName());
                trimResult = 1;
            }
        }

        totalUnitCount -= unitCount;
        startLba += unitCount;

    } while (totalUnitCount != 0);

    if (trimResult == 0)
    {
        result = _CheckTrimValue();

        if (result == 0)
        {
            IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::ARRAY_PARTITION_TRIM,
                "Trim Succeeded from {} ", startLba);
        }
        else
        {
            IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::ARRAY_PARTITION_TRIM,
                "Trim Succeeded with wrong value from {}", startLba);
            // To Do : Write All Zeroes
        }
    }
    else
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::ARRAY_PARTITION_TRIM, "Trim Failed on some devices");
        // To Do : Write All Zeroes
    }
}

int
StripePartition::_CheckTrimValue()
{
    uint64_t startLba = GetPhysicalSize()->startLba;
    uint64_t readUnitCount = 1;
    void* readbuffer = Memory<Ubio::BYTES_PER_UNIT>::Alloc(readUnitCount);
    void* zerobuffer = Memory<Ubio::BYTES_PER_UNIT>::Alloc(readUnitCount);
    memset(zerobuffer, 0, Ubio::BYTES_PER_UNIT);
    int result = 0;
    int nonZeroResult = 0;

    UbioSmartPtr readUbio(new Ubio(readbuffer, readUnitCount));
    for (uint32_t i = 0; i < devs_.size(); i++)
    {
        PhysicalBlkAddr pba = {.dev = devs_[i],
            .lba = startLba};
        readUbio->SetPba(pba);
        IODispatcher* ioDispatcher = EventArgument::GetIODispatcher();
        ioDispatcher->Submit(readUbio, true);
        result = memcmp(readUbio->GetBuffer(), zerobuffer, Ubio::BYTES_PER_UNIT);

        if (result != 0)
        {
            IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::ARRAY_PARTITION_TRIM,
                "Trim Value is not Zero on {}", devs_[i]->uBlock->GetName());
            nonZeroResult = 1;
        }
    }

    Memory<Ubio::BYTES_PER_UNIT>::Free(readbuffer);
    Memory<Ubio::BYTES_PER_UNIT>::Free(zerobuffer);

    return nonZeroResult;
}

} // namespace ibofos

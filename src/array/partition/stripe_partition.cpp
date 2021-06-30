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

#include "src/bio/ubio.h"
#include "src/device/base/ublock_device.h"
#include "src/include/pos_event_id.h"
#include "src/include/array_config.h"
#include "src/io_scheduler/io_dispatcher.h"
#include "src/lib/block_alignment.h"
#include "src/logger/logger.h"
#include "src/state/state_manager.h"

namespace pos
{
const char* PARTITION_TYPE_STR[4] = {
    "META_NVM",
    "WRITE_BUFFER",
    "META_SSD",
    "USER_DATA"};

StripePartition::StripePartition(
    string array,
    uint32_t arrayIndex,
    PartitionType type,
    PartitionPhysicalSize physicalSize,
    vector<ArrayDevice*> devs,
    Method* method)
: StripePartition(array, arrayIndex, type, physicalSize, devs, method, IODispatcherSingleton::Instance())
{}

StripePartition::StripePartition(
    string array,
    uint32_t arrayIndex,
    PartitionType type,
    PartitionPhysicalSize physicalSize,
    vector<ArrayDevice*> devs,
    Method* method,
    IODispatcher* ioDispatcher)
: Partition(array, arrayIndex, type, physicalSize, devs, method),
  ioDispatcher_(ioDispatcher)
{
    _SetLogicalSize();
}

StripePartition::~StripePartition(void)
{
}

int
StripePartition::_SetLogicalSize(void)
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

int
StripePartition::Translate(PhysicalBlkAddr& dst, const LogicalBlkAddr& src)
{
    if (false == _IsValidAddress(src))
    {
        int error = (int)POS_EVENT_ID::ARRAY_INVALID_ADDRESS_ERROR;
        POS_TRACE_ERROR(error, "Invalid Address Error");
        return error;
    }

    FtBlkAddr fsa;
    method_->Translate(fsa, src);

    dst = _F2PTranslate(fsa);

    return 0;
}

int
StripePartition::ByteTranslate(PhysicalByteAddr& dst, const LogicalByteAddr& src)
{
    return -1;
}

PhysicalBlkAddr
StripePartition::_F2PTranslate(const FtBlkAddr& fba)
{
    PhysicalBlkAddr pba;
    uint32_t chunkIndex = fba.offset / physicalSize_.blksPerChunk;
    pba.arrayDev = devs_.at(chunkIndex);
    pba.lba = physicalSize_.startLba +
        (fba.stripeId * physicalSize_.blksPerChunk + fba.offset % physicalSize_.blksPerChunk) * ArrayConfig::SECTORS_PER_BLOCK;

    return pba;
}

FtBlkAddr
StripePartition::_P2FTranslate(const PhysicalBlkAddr& pba)
{
    int chunkIndex = -1;
    for (uint32_t i = 0; i < devs_.size(); i++)
    {
        if (pba.arrayDev == devs_.at(i))
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
        int error = (int)POS_EVENT_ID::ARRAY_INVALID_ADDRESS_ERROR;
        POS_TRACE_ERROR(error, "Invalid Address Error");
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

int
StripePartition::ByteConvert(list<PhysicalByteWriteEntry> &dst,
    const LogicalByteWriteEntry &src)
{
    return -1;
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
            .lba = stripeLba + startOffset * ArrayConfig::SECTORS_PER_BLOCK,
            .arrayDev = devs_.at(i) };
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
            split.SetBuffer(reinterpret_cast<Block*>(buffer.GetBufferPtr()) + gap);
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
StripePartition::GetRecoverMethod(UbioSmartPtr ubio, RecoverMethod& out)
{
    uint64_t lba = ubio->GetLba();
    ArrayDevice* dev = static_cast<ArrayDevice*>(ubio->GetArrayDev());
    if (IsValidLba(lba) && (FindDevice(dev) >= 0))
    {
        // Chunk Aliging check
        const uint32_t sectorSize = ArrayConfig::SECTOR_SIZE_BYTE;
        const uint32_t sectorsPerBlock = ArrayConfig::SECTORS_PER_BLOCK;

        PhysicalBlkAddr originPba = ubio->GetPba();
        BlockAlignment blockAlignment(originPba.lba * sectorSize, ubio->GetSize());
        originPba.lba = blockAlignment.GetHeadBlock() * sectorsPerBlock;
        FtBlkAddr fba = _P2FTranslate(originPba);
        out.srcAddr = _GetRebuildGroup(fba);
        out.recoverFunc = method_->GetRecoverFunc();

        return (int)POS_EVENT_ID::SUCCESS;
    }
    return (int)POS_EVENT_ID::RECOVER_INVALID_LBA;
}

unique_ptr<RebuildContext>
StripePartition::GetRebuildCtx(ArrayDevice* fault)
{
    int index = FindDevice(fault);
    if (index >= 0)
    {
        unique_ptr<RebuildContext> ctx(new RebuildContext());
        ctx->raidType = method_->GetRaidType();
        ctx->array = arrayName_;
        ctx->arrayIndex = arrayIndex_;
        ctx->part = PARTITION_TYPE_STR[type_];
        ctx->faultIdx = index;
        ctx->faultDev = fault;
        ctx->stripeCnt = logicalSize_.totalStripes;
        ctx->result = RebuildState::READY;
        ctx->size = GetPhysicalSize();
        {
            using namespace std::placeholders;
            F2PTranslator trns = std::bind(&StripePartition::_F2PTranslate, this, _1);
            ctx->translate = trns;
        }
        return ctx;
    }
    return nullptr;
}

void
StripePartition::Format(void)
{
    _Trim();
}

bool
StripePartition::IsByteAccessSupported(void)
{
    return false;
}

void
StripePartition::_Trim(void)
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

        UbioSmartPtr ubio(new Ubio(dummyBuffer, unitCount, arrayIndex_));
        ubio->dir = UbioDir::Deallocate;

        for (uint32_t i = 0; i < devs_.size(); i++)
        {
            if (devs_[i]->GetState() == ArrayDeviceState::FAULT)
            {
                continue;
            }

            PhysicalBlkAddr pba = {
                .lba = startLba,
                .arrayDev = devs_[i] };
            ubio->SetPba(pba);
            ubio->SetUblock(devs_[i]->GetUblock());
            result = ioDispatcher_->Submit(ubio, true);
            POS_TRACE_DEBUG((int)POS_EVENT_ID::ARRAY_PARTITION_TRIM,
                "Try to trim from {} for {} on {}",
                pba.lba, unitCount, devs_[i]->GetUblock()->GetName());

            if (result < 0 || ubio->GetError() != IOErrorType::SUCCESS)
            {
                POS_TRACE_ERROR((int)POS_EVENT_ID::ARRAY_PARTITION_TRIM,
                    "Trim Failed on {}", devs_[i]->GetUblock()->GetName());
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
            POS_TRACE_DEBUG((int)POS_EVENT_ID::ARRAY_PARTITION_TRIM,
                "Trim Succeeded from {} ", startLba);
        }
        else
        {
            POS_TRACE_ERROR((int)POS_EVENT_ID::ARRAY_PARTITION_TRIM,
                "Trim Succeeded with wrong value from {}", startLba);
            // To Do : Write All Zeroes
        }
    }
    else
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::ARRAY_PARTITION_TRIM, "Trim Failed on some devices");
        // To Do : Write All Zeroes
    }
}

int
StripePartition::_CheckTrimValue(void)
{
    uint64_t startLba = GetPhysicalSize()->startLba;
    uint64_t readUnitCount = 1;
    void* readbuffer = Memory<Ubio::BYTES_PER_UNIT>::Alloc(readUnitCount);
    void* zerobuffer = Memory<Ubio::BYTES_PER_UNIT>::Alloc(readUnitCount);
    memset(zerobuffer, 0, Ubio::BYTES_PER_UNIT);
    int result = 0;
    int nonZeroResult = 0;

    UbioSmartPtr readUbio(new Ubio(readbuffer, readUnitCount, arrayIndex_));
    for (uint32_t i = 0; i < devs_.size(); i++)
    {
        PhysicalBlkAddr pba = {
            .lba = startLba,
            .arrayDev = devs_[i] };
        readUbio->SetPba(pba);
        ioDispatcher_->Submit(readUbio, true);
        result = memcmp(readUbio->GetBuffer(), zerobuffer, Ubio::BYTES_PER_UNIT);

        if (result != 0)
        {
            POS_TRACE_ERROR((int)POS_EVENT_ID::ARRAY_PARTITION_TRIM,
                "Trim Value is not Zero on {}", devs_[i]->GetUblock()->GetName());
            nonZeroResult = 1;
        }
    }

    Memory<Ubio::BYTES_PER_UNIT>::Free(readbuffer);
    Memory<Ubio::BYTES_PER_UNIT>::Free(zerobuffer);

    return nonZeroResult;
}

} // namespace pos

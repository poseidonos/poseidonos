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

#include <cassert>
#include <iostream>
#include <memory>

#include "stripe_partition.h"
#include "src/helper/enumerable/query.h"
#include "src/bio/ubio.h"
#include "src/device/base/ublock_device.h"
#include "src/include/pos_event_id.h"
#include "src/include/array_config.h"
#include "src/lib/block_alignment.h"
#include "src/logger/logger.h"
#include "src/array/ft/raid10.h"
#include "src/array/ft/raid5.h"
#include "src/array/ft/raid0.h"
#include "src/array/ft/raid_none.h"
#include "src/helper/calc/calc.h"

namespace pos
{

StripePartition::StripePartition(
    PartitionType type,
    vector<ArrayDevice*> devs,
    RaidTypeEnum raid)
: Partition(devs, type),
  RebuildTarget(type),
  raidType(raid)
{
}

StripePartition::~StripePartition(void)
{
    delete method;
    method = nullptr;
}

int
StripePartition::Create(uint64_t startLba, uint32_t segCnt, uint64_t totalNvmBlks)
{
    POS_TRACE_INFO(EID(ARRAY_DEBUG_MSG), "StripePartition::Create, RaidType:{}", RaidType(raidType).ToString());

    if (raidType == RaidTypeEnum::RAID10 && 0 != devs.size() % 2)
    {
        devs.pop_back();
    }
    int ret = _SetPhysicalAddress(startLba, segCnt);
    if (ret != 0)
    {
        return ret;
    }
    Partition::_UpdateLastLba();
    ret = _SetMethod(totalNvmBlks);
    if (ret != 0)
    {
        return ret;
    }
    _SetLogicalAddress();
    return 0;
}

void
StripePartition::RegisterService(IPartitionServices* svc)
{
    POS_TRACE_DEBUG(EID(ARRAY_DEBUG_MSG), "StripePartition::RegisterService");
    svc->AddTranslator(type, this);
    if (method->IsRecoverable() == true)
    {
        svc->AddRebuildTarget(this);
        svc->AddRecover(type, this);
    }
    else
    {
        POS_TRACE_INFO(EID(ARRAY_DEBUG_MSG), "{} partition (RaidType: {}) is excluded from rebuild target", 
            PARTITION_TYPE_STR[type], RaidType(raidType).ToString());
    }
}

int
StripePartition::_SetPhysicalAddress(uint64_t startLba, uint32_t segCnt)
{
    physicalSize.startLba = startLba;
    physicalSize.blksPerChunk = ArrayConfig::BLOCKS_PER_CHUNK;
    physicalSize.chunksPerStripe = devs.size();
    physicalSize.stripesPerSegment = ArrayConfig::STRIPES_PER_SEGMENT;
    physicalSize.totalSegments = segCnt;
    POS_TRACE_DEBUG(EID(ARRAY_DEBUG_MSG), "StripePartition::_SetPhysicalAddress, StartLba:{}, RaidType:{}, SegCnt:{}",
        startLba, RaidType(raidType).ToString(), physicalSize.totalSegments);

    return 0;
}

void
StripePartition::_SetLogicalAddress(void)
{
    const FtSizeInfo* ftSize = method->GetSizeInfo();
    logicalSize = {
        .minWriteBlkCnt = ftSize->minWriteBlkCnt,
        .blksPerChunk = ftSize->blksPerChunk,
        .blksPerStripe = ftSize->blksPerStripe - ftSize->backupBlkCnt,
        .chunksPerStripe = ftSize->chunksPerStripe -
            (ftSize->backupBlkCnt / ftSize->blksPerChunk),
        .stripesPerSegment = physicalSize.stripesPerSegment,
        .totalStripes =
            physicalSize.stripesPerSegment * physicalSize.totalSegments,
        .totalSegments = physicalSize.totalSegments};
}

int
StripePartition::_SetMethod(uint64_t totalNvmBlks)
{
    if (raidType == RaidTypeEnum::RAID0)
    {
        Raid0* raid0 = new Raid0(&physicalSize);
        method = raid0;
    }
    else if (raidType == RaidTypeEnum::RAID10)
    {
        Raid10* raid10 = new Raid10(&physicalSize);
        method = raid10;
    }
    else if (raidType == RaidTypeEnum::RAID5)
    {
        Raid5* raid5 = new Raid5(&physicalSize);
        uint64_t blksPerStripe = static_cast<uint64_t>(physicalSize.blksPerChunk) * physicalSize.chunksPerStripe;
        uint64_t totalNvmStripes = totalNvmBlks / blksPerStripe;
        uint64_t maxGcStripes = 2048;
        POS_TRACE_INFO(EID(ARRAY_DEBUG_MSG), "Alloc parity pool, size:{}", totalNvmStripes + maxGcStripes);
        if (raid5->AllocParityPools(totalNvmStripes + maxGcStripes) == false)
        {
            delete raid5;
            int eventId = EID(ARRAY_PARTITION_CREATION_ERROR);
            POS_TRACE_ERROR(eventId, "Failed to create partition \"USER_DATA\". Buffer pool allocation failed.");
            return eventId;
        }
        method = raid5;
    }
    else if (raidType == RaidTypeEnum::NONE)
    {
        RaidNone* raidNone = new RaidNone(&physicalSize);
        method = raidNone;
    }
    else
    {
        int eventId = EID(ARRAY_WRONG_FT_METHOD);
        POS_TRACE_ERROR(eventId, "Failed to set FT method because {} isn't supported", RaidType(raidType).ToString());
        return eventId;
    }
    size_t required = method->GetMinimumNumberOfDevices();
    size_t actual = devs.size();
    if (required > actual)
    {
        int eventId = EID(ARRAY_DEVICE_COUNT_ERROR);
        POS_TRACE_ERROR(eventId, "Failed to set FT method because there are not enough devices, need:{}, actual:{}",
            required, actual);
        delete method;
        return eventId;
    }
    return 0;
}

int
StripePartition::Translate(PhysicalBlkAddr& dst, const LogicalBlkAddr& src)
{
    if (false == _IsValidAddress(src))
    {
        int error = EID(ARRAY_INVALID_ADDRESS_ERROR);
        POS_TRACE_ERROR(error, "{} partition detects invalid address during translate. raidtype:{}, stripeId:{}, offset:{}, totalStripes:{}, totalBlksPerStripe:{}",
            PARTITION_TYPE_STR[type], raidType, src.stripeId, src.offset, logicalSize.totalStripes, logicalSize.blksPerStripe);
        return error;
    }

    FtBlkAddr fsa;
    method->Translate(fsa, src);

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
    uint32_t chunkIndex = fba.offset / physicalSize.blksPerChunk;
    pba.arrayDev = devs.at(chunkIndex);
    pba.lba = physicalSize.startLba +
        (fba.stripeId * physicalSize.blksPerChunk + fba.offset % physicalSize.blksPerChunk) * ArrayConfig::SECTORS_PER_BLOCK;

    return pba;
}

FtBlkAddr
StripePartition::_P2FTranslate(const PhysicalBlkAddr& pba)
{
    int chunkIndex = -1;
    for (uint32_t i = 0; i < devs.size(); i++)
    {
        if (pba.arrayDev == devs.at(i))
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
        (pba.lba - physicalSize.startLba) / ArrayConfig::SECTORS_PER_BLOCK;
    FtBlkAddr fsa = {
        .stripeId = (uint32_t)(ptnBlkOffset / physicalSize.blksPerChunk),
        .offset =
            (chunkIndex * physicalSize.blksPerChunk) + (ptnBlkOffset % physicalSize.blksPerChunk)};

    return fsa;
}

int
StripePartition::Convert(list<PhysicalWriteEntry>& dst,
    const LogicalWriteEntry& src)
{
    if (false == _IsValidEntry(src))
    {
        int error = EID(ARRAY_INVALID_ADDRESS_ERROR);
        POS_TRACE_ERROR(error, "{} partition detects invalid address during convert. raidtype:{}, stripeId:{}, offset:{}, blkCnt:{}, totalStripes:{}, totalBlksPerStripe:{}, minWriteBlkCnt:{}",
            PARTITION_TYPE_STR[type], raidType, src.addr.stripeId, src.addr.offset, src.blkCnt, logicalSize.totalStripes, logicalSize.blksPerStripe, logicalSize.minWriteBlkCnt);
        return error;
    }
    dst.clear();
    list<FtWriteEntry> ftEntries;
    method->Convert(ftEntries, src);
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
    list<FtBlkAddr> ftAddrs = method->GetRebuildGroup(fba);
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
    const uint32_t chunkSize = physicalSize.blksPerChunk;

    const uint32_t firstOffset = ftEntry.addr.offset;
    const uint32_t lastOffset = firstOffset + ftEntry.blkCnt - 1;
    const uint32_t chunkStart = firstOffset / chunkSize;
    const uint32_t chunkEnd = lastOffset / chunkSize;

    const uint64_t stripeLba = physicalSize.startLba + ((uint64_t)ftEntry.addr.stripeId * chunkSize * ArrayConfig::SECTORS_PER_BLOCK);

    uint32_t startOffset = firstOffset - chunkSize * chunkStart;
    uint32_t endOffset = lastOffset - chunkSize * chunkEnd;

    uint32_t bufferOffset = 0;
    for (uint32_t i = chunkStart; i <= chunkEnd; i++)
    {
        PhysicalWriteEntry physicalEntry;
        physicalEntry.addr = {
            .lba = stripeLba + startOffset * ArrayConfig::SECTORS_PER_BLOCK,
            .arrayDev = devs.at(i) };
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
        out.recoverFunc = method->GetRecoverFunc();

        return (int)POS_EVENT_ID::SUCCESS;
    }
    return (int)POS_EVENT_ID::RECOVER_INVALID_LBA;
}

unique_ptr<RebuildContext>
StripePartition::GetRebuildCtx(ArrayDevice* fault)
{
    int index = FindDevice(fault);
    POS_TRACE_DEBUG(EID(REBUILD_DEBUG_MSG), "GetRebuildCtx devIndex:{}, index");
    if (index >= 0)
    {
        unique_ptr<RebuildContext> ctx(new RebuildContext());
        ctx->raidType = raidType;
        ctx->part = PARTITION_TYPE_STR[type];
        ctx->faultIdx = index;
        ctx->faultDev = fault;
        ctx->stripeCnt = logicalSize.totalStripes;
        ctx->size = GetPhysicalSize();
        {
            using namespace std::placeholders;
            F2PTranslator trns = std::bind(&StripePartition::_F2PTranslate, this, _1);
            ctx->translate = trns;
        }
        return ctx;
    }
    POS_TRACE_INFO(EID(REBUILD_DEBUG_MSG), "GetRebuildCtx return nullptr");
    return nullptr;
}

bool
StripePartition::IsByteAccessSupported(void)
{
    return false;
}

RaidState
StripePartition::GetRaidState(void)
{
    auto&& deviceStateList = Enumerable::Select(devs,
        [](auto d) { return d->GetState(); });

    return method->GetRaidState(deviceStateList);
}
} // namespace pos

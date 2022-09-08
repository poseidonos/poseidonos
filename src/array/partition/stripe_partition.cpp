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
#include "src/event/event_manager.h"
#include "src/include/array_config.h"
#include "src/lib/block_alignment.h"
#include "src/logger/logger.h"
#include "src/array/ft/raid10.h"
#include "src/array/ft/raid5.h"
#include "src/array/ft/raid0.h"
#include "src/array/ft/raid_none.h"
#include "src/array/ft/raid6.h"
#include "src/helper/calc/calc.h"

namespace pos
{

StripePartition::StripePartition(
    PartitionType type,
    vector<ArrayDevice*> devs,
    RaidTypeEnum raid)
: Partition(devs, type),
  RebuildTarget(type == PartitionType::USER_DATA),
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
    POS_TRACE_INFO(EID(CREATE_ARRAY_DEBUG_MSG), "StripePartition::Create, RaidType:{}", RaidType(raidType).ToString());

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
    POS_TRACE_DEBUG(EID(MOUNT_ARRAY_DEBUG_MSG), "StripePartition::RegisterService");
    svc->AddTranslator(type, this);
    if (method->IsRecoverable() == true)
    {
        svc->AddRebuildTarget(this);
        svc->AddRecover(type, this);
    }
    else
    {
        POS_TRACE_INFO(EID(MOUNT_ARRAY_DEBUG_MSG), "{} partition (RaidType: {}) is excluded from rebuild target",
            PARTITION_TYPE_STR[type], RaidType(raidType).ToString());
    }
}

int
StripePartition::Translate(list<PhysicalEntry>& pel, const LogicalEntry& le)
{
    if (false == _IsValidEntry(le.addr.stripeId, le.addr.offset, le.blkCnt))
    {
        int error = EID(ADDRESS_TRANSLATION_INVALID_LBA);
        POS_TRACE_ERROR(error, "{} partition detects invalid address during translate. raidtype:{}, stripeId:{}, offset:{}, totalStripes:{}, totalBlksPerStripe:{}",
            PARTITION_TYPE_STR[type], RaidType(raidType).ToString(), le.addr.stripeId, le.addr.offset, logicalSize.totalStripes, logicalSize.blksPerStripe);
        return error;
    }

    list<FtEntry> feList = _L2FTranslate(le);
    pel = _F2PTranslate(feList);

    return 0;
}

int
StripePartition::GetParityList(list<PhysicalWriteEntry>& parityList, const LogicalWriteEntry& src)
{
    if (false == _IsValidEntry(src.addr.stripeId, src.addr.offset, src.blkCnt))
    {
        int error = EID(ADDRESS_TRANSLATION_INVALID_LBA);
        POS_TRACE_ERROR(error, "{} partition detects invalid address during making parity. raidtype:{}, stripeId:{}, offset:{}, totalStripes:{}, totalBlksPerStripe:{}",
            PARTITION_TYPE_STR[type], raidType, src.addr.stripeId, src.addr.offset, logicalSize.totalStripes, logicalSize.blksPerStripe);
        return error;
    }
    if (src.blkCnt < logicalSize.minWriteBlkCnt)
    {
        int error = EID(ADDRESS_TRANSLATION_INVALID_BLK_CNT);
        POS_TRACE_ERROR(error, "{} partition detects invalid address during making parity 2. raidtype:{}, stripeId:{}, offset:{}, totalStripes:{}, totalBlksPerStripe:{}",
            PARTITION_TYPE_STR[type], raidType, src.addr.stripeId, src.addr.offset, logicalSize.totalStripes, logicalSize.blksPerStripe);
        return error;
    }

    list<FtWriteEntry> fweList;
    method->MakeParity(fweList, src);
    parityList = _F2PTranslate(fweList);

    return 0;
}

int
StripePartition::ByteTranslate(PhysicalByteAddr& dst, const LogicalByteAddr& src)
{
    return -1;
}

int
StripePartition::ByteConvert(list<PhysicalByteWriteEntry> &dst,
    const LogicalByteWriteEntry &src)
{
    return -1;
}

int
StripePartition::_SetPhysicalAddress(uint64_t startLba, uint32_t segCnt)
{
    physicalSize.startLba = startLba;
    physicalSize.blksPerChunk = ArrayConfig::BLOCKS_PER_CHUNK;
    physicalSize.chunksPerStripe = devs.size();
    physicalSize.stripesPerSegment = ArrayConfig::STRIPES_PER_SEGMENT;
    physicalSize.totalSegments = segCnt;
    POS_TRACE_DEBUG(EID(CREATE_ARRAY_DEBUG_MSG), "StripePartition::_SetPhysicalAddress, StartLba:{}, RaidType:{}, SegCnt:{}",
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
        uint64_t blksPerStripe = static_cast<uint64_t>(physicalSize.blksPerChunk) * physicalSize.chunksPerStripe;
        uint64_t totalNvmStripes = totalNvmBlks / blksPerStripe;
        uint64_t maxGcStripes = 2048;
        uint64_t reqBuffersPerNuma = totalNvmStripes + maxGcStripes;
        Raid5* raid5 = new Raid5(&physicalSize, reqBuffersPerNuma);
        bool result = raid5->AllocParityPools(reqBuffersPerNuma);
        if (result == false)
        {
            POS_TRACE_WARN(EID(REBUILD_DEBUG_MSG),
                "Failed to alloc ParityPools for RAID5, request:{}", reqBuffersPerNuma);
        }
        method = raid5;
    }
    else if (raidType == RaidTypeEnum::RAID6)
    {
        uint64_t blksPerStripe = static_cast<uint64_t>(physicalSize.blksPerChunk) * physicalSize.chunksPerStripe;
        uint64_t totalNvmStripes = totalNvmBlks / blksPerStripe;
        uint64_t maxGcStripes = 2048;
        uint64_t parityCnt = 2;
        uint64_t reqBuffersPerNuma = (totalNvmStripes + maxGcStripes) * parityCnt;
        Raid6* raid6 = new Raid6(&physicalSize, reqBuffersPerNuma);
        method = raid6;
    }
    else if (raidType == RaidTypeEnum::NONE)
    {
        RaidNone* raidNone = new RaidNone(&physicalSize);
        method = raidNone;
    }
    else
    {
        int eventId = EID(CREATE_ARRAY_NOT_SUPPORTED_RAIDTYPE);
        POS_TRACE_WARN(eventId, "raidtype: {} ", RaidType(raidType).ToString());
        return eventId;
    }

    size_t numofDevs = devs.size();
    if (method->CheckNumofDevsToConfigure(numofDevs) == false)
    {
        int eventId = EID(CREATE_ARRAY_RAID_INVALID_SSD_CNT);
        POS_TRACE_WARN(eventId, "requested num of ssds: {}", numofDevs);
        delete method;
        return eventId;
    }
    return 0;
}

list<FtEntry>
StripePartition::_L2FTranslate(const LogicalEntry& le)
{
    return method->Translate(le);
}

list<PhysicalEntry>
StripePartition::_F2PTranslate(const list<FtEntry>& fel)
{
    list<PhysicalEntry> peList;
    const uint32_t chunkSize = physicalSize.blksPerChunk;
    for (FtEntry fe : fel)
    {
        const BlkOffset firstOffset = fe.addr.offset;
        const BlkOffset lastOffset = firstOffset + fe.blkCnt - 1;
        const uint32_t chunkStart = firstOffset / chunkSize;
        const uint32_t chunkEnd = lastOffset / chunkSize;
        BlkOffset startOffset = firstOffset - chunkSize * chunkStart;
        BlkOffset endOffset = lastOffset - chunkSize * chunkEnd;
        const uint64_t stripeLba = physicalSize.startLba + ((uint64_t)fe.addr.stripeId * chunkSize * ArrayConfig::SECTORS_PER_BLOCK);

        for (uint32_t i = chunkStart; i <= chunkEnd; i++)
        {
            PhysicalEntry physicalEntry;
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

            peList.push_back(physicalEntry);
        }
    }
    return peList;
}

list<PhysicalWriteEntry>
StripePartition::_F2PTranslate(const list<FtWriteEntry>& fwel)
{
    list<PhysicalWriteEntry> pweList;
    const uint32_t chunkSize = physicalSize.blksPerChunk;
    for (FtWriteEntry fwe : fwel)
    {
        const BlkOffset firstOffset = fwe.addr.offset;
        const BlkOffset lastOffset = firstOffset + fwe.blkCnt - 1;
        const uint32_t chunkStart = firstOffset / chunkSize;
        const uint32_t chunkEnd = lastOffset / chunkSize;
        BlkOffset startOffset = firstOffset - chunkSize * chunkStart;
        BlkOffset endOffset = lastOffset - chunkSize * chunkEnd;
        const uint64_t stripeLba = physicalSize.startLba + ((uint64_t)fwe.addr.stripeId * chunkSize * ArrayConfig::SECTORS_PER_BLOCK);

        uint32_t bufferOffset = 0;
        for (uint32_t i = chunkStart; i <= chunkEnd; i++)
        {
            PhysicalWriteEntry pwe;
            pwe.addr = {
                .lba = stripeLba + startOffset * ArrayConfig::SECTORS_PER_BLOCK,
                .arrayDev = devs.at(i) };
            if (i != chunkEnd)
            {
                pwe.blkCnt = chunkSize - startOffset;
                startOffset = 0;
            }
            else
            {
                pwe.blkCnt = endOffset - startOffset + 1;
            }
            pwe.buffers = _SpliceBuffer(fwe.buffers, bufferOffset, pwe.blkCnt);
            bufferOffset += pwe.blkCnt;
            pweList.push_back(pwe);
        }
    }
    return pweList;
}

PhysicalBlkAddr
StripePartition::_Fba2Pba(const FtBlkAddr& fba)
{
    PhysicalBlkAddr pba;
    uint32_t chunkIndex = fba.offset / physicalSize.blksPerChunk;
    pba.arrayDev = devs.at(chunkIndex);
    pba.lba = physicalSize.startLba +
        (fba.stripeId * physicalSize.blksPerChunk + fba.offset % physicalSize.blksPerChunk) * ArrayConfig::SECTORS_PER_BLOCK;

    return pba;
}

FtBlkAddr
StripePartition::_Pba2Fba(const PhysicalBlkAddr& pba)
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
    FtBlkAddr fba = {
        .stripeId = (uint32_t)(ptnBlkOffset / physicalSize.blksPerChunk),
        .offset =
            (chunkIndex * physicalSize.blksPerChunk) + (ptnBlkOffset % physicalSize.blksPerChunk)};

    return fba;
}

list<PhysicalBlkAddr>
StripePartition::_GetRebuildGroup(FtBlkAddr fba, const vector<uint32_t>& abnormals)
{
    list<FtBlkAddr> ftAddrs = method->GetRebuildGroup(fba, abnormals);
    list<PhysicalBlkAddr> ret;
    for (FtBlkAddr fba : ftAddrs)
    {
        PhysicalBlkAddr pba = _Fba2Pba(fba);
        ret.push_back(pba);
    }
    return ret;
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
    vector<uint32_t> abnormals = _GetAbnormalDeviceIndex();

    if (IsValidLba(lba))
    {
        int devIdx = FindDevice(dev);
        if (devIdx >= 0)
        {
            // Chunk Aliging check
            const uint32_t sectorSize = ArrayConfig::SECTOR_SIZE_BYTE;
            const uint32_t sectorsPerBlock = ArrayConfig::SECTORS_PER_BLOCK;

            PhysicalBlkAddr originPba = ubio->GetPba();
            BlockAlignment blockAlignment(originPba.lba * sectorSize, ubio->GetSize());
            originPba.lba = blockAlignment.GetHeadBlock() * sectorsPerBlock;
            FtBlkAddr fba = _Pba2Fba(originPba);
            out.srcAddr = _GetRebuildGroup(fba, abnormals);
            out.recoverFunc = method->GetRecoverFunc(vector<uint32_t>{(uint32_t)devIdx}, abnormals);
            return EID(SUCCESS);
        }
        else
        {
            int error = EID(RECOVER_REQ_DEV_NOT_FOUND);
            POS_TRACE_ERROR(error, "Failed to find device {} in {} partition, lba:{}", dev->GetName(), PARTITION_TYPE_STR[type], lba);
            return error;
        }
    }
    else
    {
        int eid = EID(RECOVER_INVALID_LBA);
        return eid;
    }
}

unique_ptr<RebuildContext>
StripePartition::GetRebuildCtx(const vector<IArrayDevice*>& fault)
{
    unique_ptr<RebuildContext> ctx(new RebuildContext());
    _SetRebuildPair(fault, ctx->rp);
    if (ctx->rp.size() == 0)
    {
        POS_TRACE_INFO(EID(REBUILD_DEBUG_MSG), "GetRebuildCtx returns nullptr, part:{}", PARTITION_TYPE_STR[type]);
        ctx.reset();
        return nullptr;
    }
    POS_TRACE_INFO(EID(REBUILD_DEBUG_MSG),
        "GetRebuildCtx, pairCnt:{}, part:{}", ctx->rp.size(), PARTITION_TYPE_STR[type]);
    ctx->part = type;
    ctx->stripeCnt = logicalSize.totalStripes;
    ctx->size = GetPhysicalSize();
    return ctx;
}

unique_ptr<RebuildContext>
StripePartition::GetQuickRebuildCtx(const QuickRebuildPair& rebuildPair)
{
    unique_ptr<QuickRebuildContext> ctx(new QuickRebuildContext());
    ctx->rebuildType = RebuildTypeEnum::QUICK;
    _SetQuickRebuildPair(rebuildPair, ctx->rp, ctx->secondaryRp);
    if (ctx->rp.size() == 0)
    {
        POS_TRACE_INFO(EID(REBUILD_DEBUG_MSG), "GetQuickRebuildCtx returns nullptr, part:{}", PARTITION_TYPE_STR[type]);
        ctx.reset();
        return nullptr;
    }
    
    ctx->part = type;
    ctx->stripeCnt = logicalSize.totalStripes;
    ctx->size = GetPhysicalSize();
    return ctx;
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

void 
StripePartition::_SetRebuildPair(const vector<IArrayDevice*>& fault, RebuildPairs& rp)
{
    vector<uint32_t> rebuildTargetIndexs;
    for (IArrayDevice* dev : fault)
    {
        int index = FindDevice(dev);
        if (index >= 0)
        {
            POS_TRACE_DEBUG(EID(REBUILD_DEBUG_MSG), "SetRebuildPair, find device, device {} is included at this partition {}",
                dev->GetName(), PARTITION_TYPE_STR[type]);
            rebuildTargetIndexs.push_back((uint32_t)index);
        }
    }
    if (rebuildTargetIndexs.size() == 0)
    {
        POS_TRACE_INFO(EID(REBUILD_DEBUG_MSG), "SetRebuildPair, no target found in this partition {}",
            PARTITION_TYPE_STR[type]);
        return;
    }

    vector<pair<vector<uint32_t>, vector<uint32_t>>> rg =
        method->GetRebuildGroupPairs(rebuildTargetIndexs);
    POS_TRACE_INFO(EID(REBUILD_DEBUG_MSG), "SetRebuildPair, count:{}, raidType:{}", rg.size(), GetRaidType());
    vector<uint32_t> abnormalDeviceIndex = _GetAbnormalDeviceIndex();
    for (pair<vector<uint32_t>, vector<uint32_t>> group : rg)
    {
        vector<uint32_t> srcs = group.first;
        vector<uint32_t> dsts = group.second;
         POS_TRACE_DEBUG(EID(REBUILD_DEBUG_MSG),
            "SetRebuildPair, srcCnt:{}, dstCnt:{}", srcs.size(), dsts.size());

        vector<IArrayDevice*> srcDevs;
        vector<IArrayDevice*> dstDevs;
        for (uint32_t i : srcs)
        {
            srcDevs.push_back(devs.at(i));
        }
        for (uint32_t i : dsts)
        {
            dstDevs.push_back(devs.at(i));
            POS_TRACE_DEBUG(EID(REBUILD_DEBUG_MSG), "SetRebuildPair, dsts:{}", i);
        }
        for (uint32_t i : abnormalDeviceIndex)
        {
            POS_TRACE_DEBUG(EID(REBUILD_DEBUG_MSG), "SetRebuildPair, abnormals:{}", i);
        }
        
        RecoverFunc func = method->GetRecoverFunc(dsts, abnormalDeviceIndex);
        rp.emplace_back(new RebuildPair(srcDevs, dstDevs, func));
    }
}

void 
StripePartition::_SetQuickRebuildPair(const QuickRebuildPair& quickRebuildPair, RebuildPairs& rp,
        RebuildPairs& backupRp)
{
    vector<IArrayDevice*> fault;
    for (auto qrp : quickRebuildPair)
    {
        IArrayDevice* dst = qrp.second;
        POS_TRACE_DEBUG(EID(REBUILD_DEBUG_MSG), "SetQuickRebuildPair, dev:{}", qrp.second->GetName());
        int index = FindDevice(dst);
        if (index >= 0)
        {
            fault.push_back(dst);
            POS_TRACE_DEBUG(EID(REBUILD_DEBUG_MSG), "SetQuickRebuildPair, device {} is included in this partition {}",
                dst->GetName(), PARTITION_TYPE_STR[type]);
            RecoverFunc func = bind(memcpy, placeholders::_1, placeholders::_2, placeholders::_3);
            rp.emplace_back(new RebuildPair(vector<IArrayDevice*>{qrp.first}, vector<IArrayDevice*>{qrp.second}, func));
        }
    }
    if (rp.size() == 0)
    {
        POS_TRACE_INFO(EID(REBUILD_DEBUG_MSG), "SetQuickRebuildPair, no target found at this partition {}",
            PARTITION_TYPE_STR[type]);
        return;
    }

    POS_TRACE_INFO(EID(REBUILD_DEBUG_MSG),
        "QuickRebuildPairs, pairCnt:{}, part:{}", rp.size(), PARTITION_TYPE_STR[type]);
    _SetRebuildPair(fault, backupRp);
    POS_TRACE_INFO(EID(REBUILD_DEBUG_MSG),
        "QuickRebuildPairsBackup, pairCnt:{}, part:{}", backupRp.size(), PARTITION_TYPE_STR[type]);
}

vector<uint32_t>
StripePartition::_GetAbnormalDeviceIndex(void)
{
    auto&& abnormalDevIndex = Enumerable::SelectWhere(devs,
        [](auto d) { return d->GetDataIndex(); }, [](auto i) { return i->GetState() != ArrayDeviceState::NORMAL; });

    return abnormalDevIndex;
}

} // namespace pos

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

#include "nvm_partition.h"

#include <vector>

#include "src/include/array_config.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/device/base/ublock_device.h"
#include "src/helper/calc/calc.h"

namespace pos
{
NvmPartition::NvmPartition(
    PartitionType type,
    vector<ArrayDevice*> devs)
: Partition(devs, type)
{
}

NvmPartition::~NvmPartition()
{
}

int
NvmPartition::Create(uint64_t startLba, uint32_t blksPerChunk)
{
    int ret = _SetPhysicalAddress(startLba, blksPerChunk);
    if (ret != 0)
    {
        return ret;
    }
    Partition::_UpdateLastLba();
    _SetLogicalAddress();
    return ret;
}

void
NvmPartition::RegisterService(IPartitionServices* svc)
{
    POS_TRACE_DEBUG(EID(ARRAY_DEBUG_MSG), "NvmPartition::RegisterService");
    svc->AddTranslator(type, this);
}

int
NvmPartition::Translate(list<PhysicalEntry>& pel, const LogicalEntry& le)
{
    if (false == _IsValidEntry(le.addr.stripeId, le.addr.offset, le.blkCnt))
    {
        int error = EID(ARRAY_INVALID_ADDRESS_ERROR);
        POS_TRACE_ERROR(error, "Invalid Address Error");
        return error;
    }

    PhysicalEntry pe;
    pe.addr.arrayDev = devs.front();
    pe.addr.lba = physicalSize.startLba +
        ((uint64_t)le.addr.stripeId * logicalSize.blksPerStripe + le.addr.offset) *
        ArrayConfig::SECTORS_PER_BLOCK;
    pe.blkCnt = le.blkCnt;
    pel.push_back(pe);
    return 0;
}

int
NvmPartition::GetParityList(list<PhysicalWriteEntry>& parity, const LogicalWriteEntry& src)
{
    parity.clear();
    return 0;
}

int
NvmPartition::ByteTranslate(PhysicalByteAddr& dst, const LogicalByteAddr& src)
{
    if (false == _IsValidByteAddress(src))
    {
        int error = EID(ARRAY_INVALID_ADDRESS_ERROR);
        POS_TRACE_ERROR(error, "Invalid Address Error");
        return error;
    }

    dst.arrayDev = devs.front();
    void* base = dst.arrayDev->GetUblockPtr()->GetByteAddress();
    dst.byteAddress = reinterpret_cast<uint64_t>(base) + (physicalSize.startLba +
        (src.blkAddr.stripeId * logicalSize.blksPerStripe + src.blkAddr.offset) *
        ArrayConfig::SECTORS_PER_BLOCK) * ArrayConfig::SECTOR_SIZE_BYTE +
        src.byteOffset;

    return 0;
}

int
NvmPartition::ByteConvert(list<PhysicalByteWriteEntry>& dst,
    const LogicalByteWriteEntry& src)
{
    if (false == _IsValidByteEntry(src))
    {
        int error = EID(ARRAY_INVALID_ADDRESS_ERROR);
        POS_TRACE_ERROR(error, "Invalid Address Error");
        return error;
    }

    PhysicalByteWriteEntry physicalByteEntry;

    ByteTranslate(physicalByteEntry.addr, src.addr);
    physicalByteEntry.byteCnt = src.byteCnt;
    physicalByteEntry.buffers = *(src.buffers);

    dst.clear();
    dst.push_back(physicalByteEntry);

    return 0;
}

bool
NvmPartition::IsByteAccessSupported(void)
{
    return true;
}

int
NvmPartition::_SetPhysicalAddress(uint64_t startLba, uint32_t blksPerChunk)
{
    POS_TRACE_DEBUG(EID(ARRAY_DEBUG_MSG), "NvmPartition::_SetPhysicalAddress, startLba:{}, blksPerChunk:{}, devsize:{}, devName:{}",
        startLba, blksPerChunk, devs.size(), devs.front()->GetUblock()->GetName());
    physicalSize.startLba = startLba;
    physicalSize.blksPerChunk = blksPerChunk;
    physicalSize.chunksPerStripe = ArrayConfig::NVM_DEVICE_COUNT;
    physicalSize.totalSegments = ArrayConfig::NVM_SEGMENT_SIZE;
    if (type == PartitionType::WRITE_BUFFER)
    {
        physicalSize.stripesPerSegment = (devs.front()->GetUblock()->GetSize() / ArrayConfig::BLOCK_SIZE_BYTE -
            DIV_ROUND_UP(physicalSize.startLba, (uint64_t)ArrayConfig::SECTORS_PER_BLOCK)) / blksPerChunk;
        POS_TRACE_DEBUG(EID(ARRAY_DEBUG_MSG), "NvmPartition::_SetPhysicalAddress, WRITE_BUFFER, startLba:{}, blksPerChunk:{}, strPerSeg:{}",
            startLba, blksPerChunk, physicalSize.stripesPerSegment);
    }
    else if (type == PartitionType::META_NVM)
    {
        physicalSize.stripesPerSegment = ArrayConfig::META_NVM_SIZE /
            (ArrayConfig::BLOCK_SIZE_BYTE * physicalSize.blksPerChunk * physicalSize.chunksPerStripe);
        POS_TRACE_DEBUG(EID(ARRAY_DEBUG_MSG), "NvmPartition::_SetPhysicalAddress, META_NVM, startLba:{}, blksPerChunk:{}, strPerSeg:{}",
            startLba, blksPerChunk, physicalSize.stripesPerSegment);
    }

    return 0;
}

void
NvmPartition::_SetLogicalAddress(void)
{
    logicalSize.minWriteBlkCnt = 1;
    logicalSize.blksPerChunk = physicalSize.blksPerChunk;
    logicalSize.blksPerStripe =
        physicalSize.blksPerChunk * physicalSize.chunksPerStripe;
    logicalSize.totalStripes =
        physicalSize.stripesPerSegment * physicalSize.totalSegments;
    logicalSize.totalSegments = physicalSize.totalSegments;
    logicalSize.chunksPerStripe = physicalSize.chunksPerStripe;
    logicalSize.stripesPerSegment =
        logicalSize.totalStripes / logicalSize.totalSegments;
}

bool
NvmPartition::_IsValidByteAddress(const LogicalByteAddr& lsa)
{
    uint32_t block_size = ArrayConfig::BLOCK_SIZE_BYTE;
    uint64_t start_blk_offset = lsa.blkAddr.offset + lsa.byteOffset / block_size;
    uint64_t start_stripe_id = lsa.blkAddr.stripeId + start_blk_offset / logicalSize.blksPerStripe;
    uint64_t end_blk_offset = lsa.blkAddr.offset + (lsa.byteOffset + lsa.byteSize) / block_size;
    uint64_t end_stripe_id = lsa.blkAddr.stripeId + end_blk_offset / logicalSize.blksPerStripe;

    if (lsa.blkAddr.stripeId < logicalSize.totalStripes &&
        lsa.blkAddr.offset < logicalSize.blksPerStripe)
    {
        if (start_blk_offset < logicalSize.blksPerStripe &&
            end_blk_offset < logicalSize.blksPerStripe &&
            lsa.byteOffset < ArrayConfig::BLOCK_SIZE_BYTE)
        {
            if (start_stripe_id < logicalSize.totalStripes &&
                end_stripe_id < logicalSize.totalStripes)
            {
                if (start_stripe_id == end_stripe_id)
                {
                    return true;
                }
                else
                {
                    POS_TRACE_DEBUG((int)POS_EVENT_ID::ARRAY_PARTITION_TRANSLATED_OVER_STRIPE,
                        "Translated Stripe ID is different from original");
                }
            }
        }
    }
    return false;
}

bool
NvmPartition::_IsValidByteEntry(const LogicalByteWriteEntry& entry)
{
    uint32_t block_size = ArrayConfig::BLOCK_SIZE_BYTE;
    uint64_t start_blk_offset = entry.addr.blkAddr.offset + entry.addr.byteOffset / block_size;
    uint64_t start_stripe_id = entry.addr.blkAddr.stripeId + start_blk_offset / logicalSize.blksPerStripe;
    uint64_t end_blk_offset = entry.addr.blkAddr.offset + (entry.addr.byteOffset + entry.addr.byteSize) / block_size;
    uint64_t end_stripe_id = entry.addr.blkAddr.stripeId + end_blk_offset / logicalSize.blksPerStripe;

    if (entry.addr.blkAddr.stripeId < logicalSize.totalStripes &&
        entry.addr.blkAddr.offset < logicalSize.blksPerStripe)
    {
        if (end_blk_offset < logicalSize.blksPerStripe &&
            entry.addr.byteOffset < ArrayConfig::BLOCK_SIZE_BYTE)
        {
            if (end_stripe_id < logicalSize.totalStripes)
            {
                if (start_stripe_id == end_stripe_id)
                {
                    return true;
                }
                else
                {
                    POS_TRACE_DEBUG((int)POS_EVENT_ID::ARRAY_PARTITION_TRANSLATED_OVER_STRIPE,
                        "Translated Stripe ID is different from original");
                }
            }
        }
    }
    return false;
}

} // namespace pos

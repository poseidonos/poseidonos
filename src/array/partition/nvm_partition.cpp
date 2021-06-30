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

#include "nvm_partition.h"

#include <vector>

#include "src/include/array_config.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
namespace pos
{
NvmPartition::NvmPartition(string array,
    uint32_t arrayIndex,
    PartitionType type,
    PartitionPhysicalSize physicalSize,
    vector<ArrayDevice*> devs)
: Partition(array, arrayIndex, type, physicalSize, devs, nullptr)
{
    logicalSize_.minWriteBlkCnt = 1;
    logicalSize_.blksPerChunk = physicalSize.blksPerChunk;
    logicalSize_.blksPerStripe =
        physicalSize.blksPerChunk * physicalSize.chunksPerStripe;
    logicalSize_.totalStripes =
        physicalSize.stripesPerSegment * physicalSize.totalSegments;
    logicalSize_.totalSegments = physicalSize.totalSegments;
    logicalSize_.chunksPerStripe = physicalSize.chunksPerStripe;
    logicalSize_.stripesPerSegment =
        logicalSize_.totalStripes / logicalSize_.totalSegments;
}

NvmPartition::~NvmPartition()
{
}

int
NvmPartition::Translate(PhysicalBlkAddr& dst, const LogicalBlkAddr& src)
{
    if (false == _IsValidAddress(src))
    {
        int error = (int)POS_EVENT_ID::ARRAY_INVALID_ADDRESS_ERROR;
        POS_TRACE_ERROR(error, "Invalid Address Error");
        return error;
    }

    dst.arrayDev = devs_.front();
    dst.lba = physicalSize_.startLba +
        (src.stripeId * logicalSize_.blksPerStripe + src.offset) *
            ArrayConfig::SECTORS_PER_BLOCK;

    return 0;
}

int
NvmPartition::ByteTranslate(PhysicalByteAddr& dst, const LogicalByteAddr& src)
{
    if (false == _IsValidByteAddress(src))
    {
        int error = (int)POS_EVENT_ID::ARRAY_INVALID_ADDRESS_ERROR;
        POS_TRACE_ERROR(error, "Invalid Address Error");
        return error;
    }

    dst.arrayDev = devs_.front();
    dst.byteAddress = (physicalSize_.startLba +
        (src.blkAddr.stripeId * logicalSize_.blksPerStripe + src.blkAddr.offset) *
        ArrayConfig::SECTORS_PER_BLOCK) * ArrayConfig::SECTOR_SIZE_BYTE +
        src.byteOffset;

    return 0;
}

int
NvmPartition::Convert(list<PhysicalWriteEntry>& dst,
    const LogicalWriteEntry& src)
{
    if (false == _IsValidEntry(src))
    {
        int error = (int)POS_EVENT_ID::ARRAY_INVALID_ADDRESS_ERROR;
        POS_TRACE_ERROR(error, "Invalid Address Error");
        return error;
    }

    PhysicalWriteEntry physicalEntry;
    Translate(physicalEntry.addr, src.addr);
    physicalEntry.blkCnt = src.blkCnt;
    physicalEntry.buffers = *(src.buffers);

    dst.clear();
    dst.push_back(physicalEntry);

    return 0;
}

int
NvmPartition::ByteConvert(list<PhysicalByteWriteEntry>& dst,
    const LogicalByteWriteEntry& src)
{
    if (false == _IsValidByteEntry(src))
    {
        int error = (int)POS_EVENT_ID::ARRAY_INVALID_ADDRESS_ERROR;
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

bool
NvmPartition::_IsValidByteAddress(const LogicalByteAddr& lsa)
{
    uint32_t block_size = ArrayConfig::BLOCK_SIZE_BYTE;
    uint64_t start_blk_offset = lsa.blkAddr.offset + lsa.byteOffset / block_size;
    uint64_t start_stripe_id = lsa.blkAddr.stripeId + start_blk_offset / logicalSize_.blksPerStripe;
    uint64_t end_blk_offset = lsa.blkAddr.offset + (lsa.byteOffset + lsa.byteSize) / block_size;
    uint64_t end_stripe_id = lsa.blkAddr.stripeId + end_blk_offset / logicalSize_.blksPerStripe;

    if (lsa.blkAddr.stripeId < logicalSize_.totalStripes &&
        lsa.blkAddr.offset < logicalSize_.blksPerStripe)
    {
        if (start_blk_offset < logicalSize_.blksPerStripe &&
            end_blk_offset < logicalSize_.blksPerStripe &&
            lsa.byteOffset < ArrayConfig::BLOCK_SIZE_BYTE)
        {
            if (start_stripe_id < logicalSize_.totalStripes &&
                end_stripe_id < logicalSize_.totalStripes)
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
    uint64_t start_stripe_id = entry.addr.blkAddr.stripeId + start_blk_offset / logicalSize_.blksPerStripe;
    uint64_t end_blk_offset = entry.addr.blkAddr.offset + (entry.addr.byteOffset + entry.addr.byteSize) / block_size;
    uint64_t end_stripe_id = entry.addr.blkAddr.stripeId + end_blk_offset / logicalSize_.blksPerStripe;

    if (entry.addr.blkAddr.stripeId < logicalSize_.totalStripes &&
        entry.addr.blkAddr.offset < logicalSize_.blksPerStripe)
    {
        if (end_blk_offset < logicalSize_.blksPerStripe &&
            entry.addr.byteOffset < ArrayConfig::BLOCK_SIZE_BYTE)
        {
            if (end_stripe_id < logicalSize_.totalStripes)
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

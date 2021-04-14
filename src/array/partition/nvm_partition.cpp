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
    PartitionType type,
    PartitionPhysicalSize physicalSize,
    vector<ArrayDevice*> devs)
: Partition(array, type, physicalSize, devs, nullptr)
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

} // namespace pos

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

#include "journal_ssd_partition.h"
#include "src/helper/enumerable/query.h"
#include "src/bio/ubio.h"
#include "src/device/base/ublock_device.h"
#include "src/include/pos_event_id.h"
#include "src/include/array_config.h"
#include "src/logger/logger.h"
#include "src/helper/calc/calc.h"

namespace pos
{
JournalSsdPartition::JournalSsdPartition(vector<ArrayDevice *> devs)
: Partition(devs, PartitionType::JOURNAL_SSD)
{
}

JournalSsdPartition::~JournalSsdPartition(void)
{
}

int
JournalSsdPartition::Create(uint64_t startLba)
{
    POS_TRACE_INFO(EID(ARRAY_DEBUG_MSG), "JournalSsdPartition::Create");
    int ret = _SetPhysicalAddress(startLba);
    if (ret != 0)
    {
        return ret;
    }
    Partition::_UpdateLastLba();
    _SetLogicalAddress();
    return ret;
}

void
JournalSsdPartition::RegisterService(IPartitionServices* svc)
{
    POS_TRACE_DEBUG(EID(ARRAY_DEBUG_MSG), "JournalSsdPartition::RegisterService");
    svc->AddTranslator(type, this);
}

int
JournalSsdPartition::_SetPhysicalAddress(uint64_t startLba)
{
    ArrayDevice* baseline = Enumerable::First(devs,
        [](auto p) { return p->GetState() == ArrayDeviceState::NORMAL; });
    if (baseline == nullptr)
    {
        int eventId = EID(ARRAY_PARTITION_CREATION_ERROR);
        POS_TRACE_ERROR(eventId, "Failed to create partition \"JOURNAL_SSD\"");
        return eventId;
    }
    physicalSize.startLba = startLba;
    physicalSize.blksPerChunk = ArrayConfig::BLOCKS_PER_CHUNK;
    physicalSize.chunksPerStripe = devs.size();
    physicalSize.stripesPerSegment = ArrayConfig::STRIPES_PER_SEGMENT;
    physicalSize.totalSegments = ArrayConfig::JOURNAL_SSD_SEGMENT_SIZE;

    POS_TRACE_DEBUG(EID(ARRAY_DEBUG_MSG), "JournalSsdPartition::_SetPhysicalAddress, StartLba:{}, SegCnt:{}",
        startLba, physicalSize.totalSegments);

    return 0;
}

void
JournalSsdPartition::_SetLogicalAddress(void)
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

int
JournalSsdPartition::Translate(PhysicalBlkAddr& dst, const LogicalBlkAddr& src)
{
    if (false == _IsValidAddress(src))
    {
        int error = EID(ARRAY_INVALID_ADDRESS_ERROR);
        POS_TRACE_ERROR(error, "Invalid Address Error");
        return error;
    }

    uint32_t chunkIndex = src.offset / physicalSize.blksPerChunk;
    dst.arrayDev = devs.at(chunkIndex);
    dst.lba = physicalSize.startLba +
        (src.stripeId * logicalSize.blksPerStripe + src.offset) *
        ArrayConfig::SECTORS_PER_BLOCK;

    return 0;
}

int
JournalSsdPartition::Convert(list<PhysicalWriteEntry>& dst,
    const LogicalWriteEntry& src)
{
    if (false == _IsValidEntry(src))
    {
        int error = EID(ARRAY_INVALID_ADDRESS_ERROR);
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
JournalSsdPartition::ByteTranslate(PhysicalByteAddr& dst, const LogicalByteAddr& src)
{
    return -1;
}

int
JournalSsdPartition::ByteConvert(list<PhysicalByteWriteEntry> &dst,
    const LogicalByteWriteEntry &src)
{
    return -1;
}

bool
JournalSsdPartition::IsByteAccessSupported(void)
{
    return false;
}

} // namespace pos

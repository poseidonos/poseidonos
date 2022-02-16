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

#pragma once

#include <cstdint>
#include <list>

#include "src/array/ft/buffer_entry.h"
#include "src/include/smart_ptr_type.h"

namespace pos
{
static const uint32_t BLOCK_ADDR_BIT_LEN = 64;
static const uint32_t STRIPE_ID_BIT_LEN = 30;
static const uint32_t STRIPE_LOC_BIT_LEN = 1;
static const uint32_t BLOCK_OFFSET_BIT_LEN = 33;
static const uint32_t SEGMENT_ID_BIT_LEN = 32;
static const uint32_t BYTE_OFFSET_BIT_LEN = 32;
static const uint32_t BYTE_SIZE_BIT_LEN = 32;

using SegmentId = uint32_t;
using StripeId = uint32_t;
using BlkOffset = uint64_t;
using BlkAddr = uint64_t;
using ByteOffset = uint32_t;
using ByteSize = uint32_t;

using ChunkIndex = uint32_t;

class IArrayDevice;

struct VirtualBlkAddr
{
    StripeId stripeId : STRIPE_ID_BIT_LEN;
    BlkOffset offset : BLOCK_OFFSET_BIT_LEN;

    inline bool
    operator==(VirtualBlkAddr input) const
    {
        return (input.stripeId == stripeId && input.offset == offset);
    }
};

struct VirtualBlks
{
    VirtualBlkAddr startVsa;
    uint32_t numBlks;

    inline bool
    operator==(VirtualBlks input) const
    {
        return (input.startVsa == startVsa && input.numBlks == numBlks);
    }
};

typedef struct
{
    StripeId stripeId : STRIPE_ID_BIT_LEN;
    BlkOffset offset : BLOCK_OFFSET_BIT_LEN;
} LogicalBlkAddr;

struct LogicalByteAddr
{
    LogicalBlkAddr blkAddr;
    ByteOffset byteOffset : BYTE_OFFSET_BIT_LEN;
    ByteSize byteSize : BYTE_SIZE_BIT_LEN;
};

struct PhysicalBlkAddr
{
    uint64_t lba;
    IArrayDevice* arrayDev;
};

struct PhysicalByteAddr
{
    uint64_t byteAddress;
    IArrayDevice* arrayDev;
};

struct LogicalEntry
{
    LogicalBlkAddr addr;
    uint32_t blkCnt;
};

struct LogicalWriteEntry
{
    LogicalBlkAddr addr;
    uint32_t blkCnt;
    std::list<BufferEntry>* buffers;
};

struct LogicalByteWriteEntry
{
    LogicalByteAddr addr;
    uint32_t byteCnt;
    std::list<BufferEntry>* buffers;
};

struct PhysicalEntry
{
    PhysicalBlkAddr addr;
    uint32_t blkCnt;
};

struct PhysicalWriteEntry
{
    PhysicalBlkAddr addr;
    uint32_t blkCnt;
    std::list<BufferEntry> buffers;
};

struct PhysicalByteWriteEntry
{
    PhysicalByteAddr addr;
    uint32_t byteCnt;
    std::list<BufferEntry> buffers;
};

struct FtBlkAddr
{
    StripeId stripeId;
    BlkOffset offset;
};

struct FtEntry
{
    FtBlkAddr addr;
    uint32_t blkCnt;
};

struct FtWriteEntry
{
    FtBlkAddr addr;
    uint32_t blkCnt;
    std::list<BufferEntry> buffers;
};

using PhysicalEntries = std::list<PhysicalWriteEntry>;

// TODO change to PartitionType;
enum StripeLoc
{
    IN_USER_AREA = 0,
    IN_WRITE_BUFFER_AREA = 1,
};

struct StripeAddr
{
    StripeLoc stripeLoc : STRIPE_LOC_BIT_LEN;
    StripeId stripeId : STRIPE_ID_BIT_LEN;

    inline bool
    operator==(StripeAddr input) const
    {
        return ((input.stripeLoc == stripeLoc) && (input.stripeId == stripeId));
    }
};

static const SegmentId UNMAP_SEGMENT = ((1UL << SEGMENT_ID_BIT_LEN) - 1);
static const StripeId UNMAP_STRIPE = ((1UL << STRIPE_ID_BIT_LEN) - 1);
static const BlkOffset UNMAP_OFFSET = ((1UL << BLOCK_OFFSET_BIT_LEN) - 1);
static const VirtualBlkAddr UNMAP_VSA = {.stripeId = UNMAP_STRIPE, .offset = UNMAP_OFFSET};
static const SegmentId NEED_TO_RETRY = UNMAP_SEGMENT - 1;

constexpr bool
IsUnMapVsa(VirtualBlkAddr vsa)
{
    return ((vsa.stripeId == UNMAP_STRIPE) && (vsa.offset == UNMAP_OFFSET));
}

constexpr bool
IsUnMapStripe(StripeId stripeId)
{
    return (stripeId == UNMAP_STRIPE);
}

constexpr bool
IsSameVsa(VirtualBlkAddr vsa1, VirtualBlkAddr vsa2)
{
    return ((vsa1.stripeId == vsa2.stripeId) && (vsa1.offset == vsa2.offset));
}

} // namespace pos

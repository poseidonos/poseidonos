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

#include "raid5.h"

#include <list>

#include "../config/array_config.h"
#include "../partition/partition_size_info.h"
#include "raid5_rebuild.h"
#include "src/include/memory.h"
#include "src/logger/logger.h"

namespace ibofos
{
Raid5::Raid5(const PartitionPhysicalSize* physicalSize,
    const uint64_t parityCount)
: freeParityPool(parityCount, CHUNK_SIZE)
{
    ftSize_ = {
        .minWriteBlkCnt = 0,
        .backupBlkCnt = physicalSize->blksPerChunk,
        .blksPerChunk = physicalSize->blksPerChunk,
        .blksPerStripe =
            physicalSize->chunksPerStripe * physicalSize->blksPerChunk,
        .chunksPerStripe = physicalSize->chunksPerStripe};
    ftSize_.minWriteBlkCnt = ftSize_.blksPerStripe - ftSize_.backupBlkCnt;
    _BindRebuildFunc();
}

int
Raid5::Translate(FtBlkAddr& dst, const LogicalBlkAddr& src)
{
    dst = {.stripeId = src.stripeId,
        .offset = src.offset};

    uint32_t chunkIndex = src.offset / ftSize_.blksPerChunk;
    uint32_t parityIndex = _GetParityOffset(src.stripeId);
    if (chunkIndex >= parityIndex)
    {
        dst.offset += ftSize_.blksPerChunk;
    }
    return 0;
}

LogicalBlkAddr
Raid5::_Translate(const FtBlkAddr& fsa)
{
    LogicalBlkAddr lsa = {.stripeId = fsa.stripeId,
        .offset = fsa.offset};
    uint32_t chunkIndex = fsa.offset / ftSize_.blksPerChunk;
    uint32_t parityIndex = _GetParityOffset(lsa.stripeId);
    if (chunkIndex == parityIndex)
    {
        assert(0);
        // TODO Error; This address is not logical address;
    }
    else if (chunkIndex > parityIndex)
    {
        lsa.offset -= ftSize_.blksPerChunk;
    }

    return lsa;
}

list<FtBlkAddr>
Raid5::GetRebuildGroup(FtBlkAddr fba)
{
    uint32_t blksPerChunk = ftSize_.blksPerChunk;
    uint32_t offsetInChunk = fba.offset % blksPerChunk;
    uint32_t chunkIndex = fba.offset / blksPerChunk;

    list<FtBlkAddr> recoveryGroup;
    for (uint32_t i = 0; i < ftSize_.chunksPerStripe; i++)
    {
        if (i != chunkIndex)
        {
            FtBlkAddr fsa = {.stripeId = fba.stripeId,
                .offset = offsetInChunk + i * blksPerChunk};
            recoveryGroup.push_back(fsa);
        }
    }

    return recoveryGroup;
}

int
Raid5::Convert(list<FtWriteEntry>& dst, const LogicalWriteEntry& src)
{
    FtWriteEntry ftEntry;
    ftEntry.addr = {.stripeId = src.addr.stripeId,
        .offset = 0};
    ftEntry.buffers = *(src.buffers);

    // 버퍼의 크기는 chunk 크기와 같아야 함
    BufferEntry parity = _AllocBuffer();
    _ComputeParity(parity, *(src.buffers));
    uint32_t parityOffset = _GetParityOffset(ftEntry.addr.stripeId);
    auto it = ftEntry.buffers.begin();
    advance(it, parityOffset);
    ftEntry.buffers.insert(it, parity);
    ftEntry.blkCnt = src.blkCnt + ftSize_.backupBlkCnt;

    dst.clear();
    dst.push_front(ftEntry);

    return 0;
}

BufferEntry
Raid5::_AllocBuffer()
{
    void* mem = freeParityPool.GetBuffer();
    uint32_t blkCnt = ArrayConfig::BLOCKS_PER_CHUNK;

    // TODO error handling for the case of insufficient free parity buffer
    assert(nullptr != mem);

    BufferEntry buffer(mem, blkCnt, true);
    buffer.SetFreeBufferPool(&freeParityPool);
    return buffer;
}

void
Raid5::_ComputeParity(BufferEntry& dst, const list<BufferEntry>& src)
{
    uint32_t memSize = ftSize_.blksPerChunk * ArrayConfig::BLOCK_SIZE_BYTE;
    void* src1 = nullptr;
    void* src2 = nullptr;

    for (const BufferEntry& buffer : src)
    {
        if (nullptr == src1)
        {
            src1 = buffer.GetBufferEntry();
            continue;
        }

        if (nullptr == src2)
        {
            src2 = buffer.GetBufferEntry();
            _XorBlocks(dst.GetBufferEntry(), src1, src2, memSize);
            continue;
        }

        _XorBlocks(dst.GetBufferEntry(), buffer.GetBufferEntry(), memSize);
    }
}

void
Raid5::_XorBlocks(void* dst, const void* src, uint32_t memSize)
{
    uint64_t* dstElement = (uint64_t*)dst;
    uint64_t* srcElement = (uint64_t*)src;

    for (uint32_t i = 0; i < memSize / sizeof(uint64_t); i++)
    {
        dstElement[i] ^= srcElement[i];
    }
}

void
Raid5::_XorBlocks(void* dst, void* src1, void* src2, uint32_t memSize)
{
    uint64_t* dstElement = (uint64_t*)dst;
    uint64_t* srcElement1 = (uint64_t*)src1;
    uint64_t* srcElement2 = (uint64_t*)src2;

    for (uint32_t i = 0; i < memSize / sizeof(uint64_t); i++)
    {
        dstElement[i] = (srcElement1[i] ^ srcElement2[i]);
    }
}

uint32_t
Raid5::_GetParityOffset(StripeId lsid)
{
    return lsid % ftSize_.chunksPerStripe;
}

void
Raid5::_BindRebuildFunc(void)
{
    using namespace std::placeholders;
    rebuildFunc_ = bind(&Raid5::_RebuildData, this, _1, _2, _3);
}

RebuildBehavior*
Raid5::GetRebuildBehavior()
{
    unique_ptr<RebuildContext> ctx(new RebuildContext());
    return new Raid5Rebuild(move(ctx));
}

void
Raid5::_RebuildData(void* dst, void* src, uint32_t dstSize)
{
    using BlockData = char[dstSize];
    memset(dst, 0, dstSize);
    for (uint32_t i = 0; i < ftSize_.chunksPerStripe - 1; i++)
    {
        _XorBlocks(dst, reinterpret_cast<BlockData*>(src) + i, dstSize);
    }
}

} // namespace ibofos

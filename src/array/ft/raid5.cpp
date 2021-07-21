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

#include "src/include/array_config.h"
#include "src/array_models/dto/partition_physical_size.h"
#include "src/logger/logger.h"
#include "src/resource_manager/buffer_pool.h"

namespace pos
{
Raid5::Raid5(const PartitionPhysicalSize* physicalSize,
    const uint64_t maxParityBufferCountPerNuma,
    AffinityManager* affinityManager,
    MemoryManager* memoryManager)
: MAX_PARITY_BUFFER_COUNT_PER_NUMA(maxParityBufferCountPerNuma),
  affinityManager(affinityManager),
  memoryManager(memoryManager)
{
    raidType = RaidTypeEnum::RAID5;
    ftSize_ = {
        .minWriteBlkCnt = 0,
        .backupBlkCnt = physicalSize->blksPerChunk,
        .blksPerChunk = physicalSize->blksPerChunk,
        .blksPerStripe =
            physicalSize->chunksPerStripe * physicalSize->blksPerChunk,
        .chunksPerStripe = physicalSize->chunksPerStripe};
    ftSize_.minWriteBlkCnt = ftSize_.blksPerStripe - ftSize_.backupBlkCnt;
    _BindRecoverFunc();
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
    uint32_t numa = affinityManager->GetNumaIdFromCurrentThread();
    BufferPool* bufferPool = parityPools.at(numa);
    void* mem = bufferPool->TryGetBuffer();
    uint32_t blkCnt = ArrayConfig::BLOCKS_PER_CHUNK;

    // TODO error handling for the case of insufficient free parity buffer
    assert(nullptr != mem);

    BufferEntry buffer(mem, blkCnt, true);
    buffer.SetBufferPool(bufferPool);
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
            src1 = buffer.GetBufferPtr();
            continue;
        }

        if (nullptr == src2)
        {
            src2 = buffer.GetBufferPtr();
            _XorBlocks(dst.GetBufferPtr(), src1, src2, memSize);
            continue;
        }

        _XorBlocks(dst.GetBufferPtr(), buffer.GetBufferPtr(), memSize);
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
Raid5::_BindRecoverFunc(void)
{
    using namespace std::placeholders;
    recoverFunc_ = bind(&Raid5::_RebuildData, this, _1, _2, _3);
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

bool
Raid5::AllocParityPools()
{
    const string NUMA_PREFIX = "_NUMA_";
    const uint64_t ARRAY_CHUNK_SIZE = ArrayConfig::BLOCK_SIZE_BYTE
        * ArrayConfig::BLOCKS_PER_CHUNK;

    uint32_t totalNumaCount = affinityManager->GetNumaCount();
    parityPools.clear();

    for (uint32_t numa = 0; numa < totalNumaCount; numa++)
    {
        BufferInfo info = {
            .owner = typeid(this).name() + NUMA_PREFIX + to_string(numa),
            .size = ARRAY_CHUNK_SIZE,
            .count = MAX_PARITY_BUFFER_COUNT_PER_NUMA
        };
        BufferPool* pool = memoryManager->CreateBufferPool(info, numa);
        if (pool == nullptr)
        {
            parityPools.clear();
            return false;
        }
        parityPools.push_back(pool);
    }
    return true;
}

void
Raid5::ClearParityPools()
{
    for(unsigned int i = 0; i < parityPools.size(); i++)
    {
        if(parityPools[i] != nullptr)
        {
            memoryManager->DeleteBufferPool(parityPools[i]);
            parityPools[i] = nullptr;
        }
    }
    parityPools.clear();
}

Raid5::~Raid5()
{
    ClearParityPools();
}

int
Raid5::GetParityPoolSize()
{
    return parityPools.size();
}


} // namespace pos


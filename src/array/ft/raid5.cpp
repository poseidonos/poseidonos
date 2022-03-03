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

#include "raid5.h"
#include "src/include/array_config.h"
#include "src/include/pos_event_id.h"
#include "src/array_models/dto/partition_physical_size.h"
#include "src/logger/logger.h"
#include "src/resource_manager/buffer_pool.h"
#include "src/helper/enumerable/query.h"

#include <string>

namespace pos
{
Raid5::Raid5(const PartitionPhysicalSize* pSize)
: Method(RaidTypeEnum::RAID5)
{
    ftSize_ = {
        .minWriteBlkCnt = 0,
        .backupBlkCnt = pSize->blksPerChunk,
        .blksPerChunk = pSize->blksPerChunk,
        .blksPerStripe = pSize->chunksPerStripe * pSize->blksPerChunk,
        .chunksPerStripe = pSize->chunksPerStripe};
    ftSize_.minWriteBlkCnt = ftSize_.blksPerStripe - ftSize_.backupBlkCnt;
    _BindRecoverFunc();
}

list<FtEntry>
Raid5::Translate(const LogicalEntry& le)
{
    uint32_t parityIndex = GetParityOffset(le.addr.stripeId).front();
    uint32_t paritySize = ftSize_.blksPerChunk;
    BlkOffset parityOffset = (uint64_t)parityIndex * (uint64_t)ftSize_.blksPerChunk;
    BlkOffset startOffset = le.addr.offset;
    BlkOffset lastOffset = startOffset + le.blkCnt - 1;
    uint32_t firstIndex = startOffset / ftSize_.blksPerChunk;
    uint32_t lastIndex = lastOffset / ftSize_.blksPerChunk;

    list<FtEntry> feList;
    FtEntry fe;
    fe.addr.stripeId = le.addr.stripeId;
    fe.addr.offset = le.addr.offset;
    fe.blkCnt = le.blkCnt;

    if (parityIndex <= firstIndex)
    {
        fe.addr.offset += paritySize;
        feList.push_back(fe);
    }
    else if (firstIndex < parityIndex && parityIndex <= lastIndex)
    {
        fe.blkCnt = parityOffset - startOffset;
        feList.push_back(fe);
        FtEntry feSecond;
        feSecond.addr.stripeId = le.addr.stripeId;
        feSecond.addr.offset = parityOffset + paritySize;
        feSecond.blkCnt = le.blkCnt - fe.blkCnt;
        feList.push_back(feSecond);
    }
    else
    {
        feList.push_back(fe);
    }

    return feList;
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

RaidState
Raid5::GetRaidState(vector<ArrayDeviceState> devs)
{
    auto&& abnormalDevs = Enumerable::Where(devs,
        [](auto d) { return d != ArrayDeviceState::NORMAL; });

    POS_TRACE_INFO(EID(RAID_DEBUG_MSG), "GetRaidState from raid5, abnormal cnt:{} ", abnormalDevs.size());
    if (abnormalDevs.size() == 0)
    {
        return RaidState::NORMAL;
    }
    else if (abnormalDevs.size() == 1)
    {
        return RaidState::DEGRADED;
    }
    return RaidState::FAILURE;
}

int
Raid5::MakeParity(list<FtWriteEntry>& ftl, const LogicalWriteEntry& src)
{
    uint32_t parityIndex = GetParityOffset(src.addr.stripeId).front();
    FtWriteEntry fwe;
    fwe.addr.stripeId = src.addr.stripeId;
    fwe.addr.offset = (uint64_t)parityIndex * (uint64_t)ftSize_.blksPerChunk;
    fwe.blkCnt = ftSize_.blksPerChunk;
    BufferEntry parity = _AllocChunk();
    _ComputeParityChunk(parity, *(src.buffers));
    fwe.buffers.push_back(parity);
    ftl.clear();
    ftl.push_back(fwe);

    return 0;
}

bool
Raid5::CheckNumofDevsToConfigure(uint32_t numofDevs)
{
    uint32_t minRequiredNumofDevsforRAID5 = 3;
    return numofDevs >= minRequiredNumofDevsforRAID5;
}

BufferEntry
Raid5::_AllocChunk()
{
    uint32_t numa = affinityManager->GetNumaIdFromCurrentThread();
    BufferPool* bufferPool = parityPools.at(numa);
    void* mem = bufferPool->TryGetBuffer();

    // TODO error handling for the case of insufficient free parity buffer
    assert(nullptr != mem);

    BufferEntry buffer(mem, ftSize_.blksPerChunk, true);
    buffer.SetBufferPool(bufferPool);
    return buffer;
}

void
Raid5::_ComputeParityChunk(BufferEntry& dst, const list<BufferEntry>& src)
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

vector<uint32_t>
Raid5::GetParityOffset(StripeId lsid)
{
    return vector<uint32_t>{ lsid % ftSize_.chunksPerStripe };
}

void
Raid5::_BindRecoverFunc(void)
{
    using namespace std::placeholders;
    recoverFunc = bind(&Raid5::_RebuildData, this, _1, _2, _3);
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
Raid5::AllocParityPools(uint64_t maxParityBufferCntPerNuma,
        AffinityManager* affMgr, MemoryManager* memoryMgr)
{
    affinityManager = affMgr;
    memoryManager = memoryMgr;
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
            .count = maxParityBufferCntPerNuma
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
    for (unsigned int i = 0; i < parityPools.size(); i++)
    {
        if (parityPools[i] != nullptr)
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


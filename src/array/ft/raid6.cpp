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

#include "raid6.h"
#include "src/include/array_config.h"
#include "src/include/pos_event_id.h"
#include "src/array_models/dto/partition_physical_size.h"
#include "src/logger/logger.h"
#include "src/resource_manager/buffer_pool.h"
#include "src/helper/enumerable/query.h"

#include <string>
#include <isa-l.h>

namespace pos
{
Raid6::Raid6(const PartitionPhysicalSize* pSize, uint64_t bufferCntPerNuma)
: Method(RaidTypeEnum::RAID6),
  parityBufferCntPerNuma(bufferCntPerNuma)
{
    
    ftSize_ = {
        .minWriteBlkCnt = 0,
        .backupBlkCnt = pSize->blksPerChunk * 2,
        .blksPerChunk = pSize->blksPerChunk,
        .blksPerStripe = pSize->chunksPerStripe * pSize->blksPerChunk,
        .chunksPerStripe = pSize->chunksPerStripe};
    ftSize_.minWriteBlkCnt = ftSize_.blksPerStripe - ftSize_.backupBlkCnt;

    chunkCnt = ftSize_.chunksPerStripe;
    dataCnt = chunkCnt - parityCnt;
    chunkSize = ArrayConfig::BLOCK_SIZE_BYTE * ftSize_.blksPerChunk;
    encode_matrix = new unsigned char[chunkCnt * dataCnt];
    gf_gen_cauchy1_matrix(encode_matrix, chunkCnt, dataCnt);
    g_tbls = new unsigned char[dataCnt * parityCnt * 32];
    ec_init_tables(dataCnt, parityCnt, &encode_matrix[dataCnt * dataCnt], g_tbls);
    _BindRecoverFunc();
}

list<FtEntry>
Raid6::Translate(const LogicalEntry& le)
{
    vector<uint32_t> parityOffset = GetParityOffset(le.addr.stripeId);
    uint32_t offsetSizeforParity = ftSize_.blksPerChunk;
    assert(parityOffset.size() == 2);

    uint32_t pParityIndex = parityOffset.front();
    uint32_t qParityIndex = parityOffset.back();
    BlkOffset pParityOffset = (uint64_t)pParityIndex * (uint64_t)offsetSizeforParity;
    BlkOffset startOffset = le.addr.offset;
    BlkOffset lastOffset = startOffset + le.blkCnt - 1;
    uint32_t firstIndex = startOffset / ftSize_.blksPerChunk;
    uint32_t lastIndex = lastOffset / ftSize_.blksPerChunk;
 
    list<FtEntry> feList;
    FtEntry fe;
    fe.addr.stripeId = le.addr.stripeId;
    fe.addr.offset = le.addr.offset;
    fe.blkCnt = le.blkCnt;

    bool isPQSeparated = qParityIndex == 0;
    if (isPQSeparated)
    {
        uint32_t paritySize = ftSize_.blksPerChunk;
        fe.addr.offset += paritySize;
        feList.push_back(fe);
    }
    else
    {
        uint32_t paritySize = ftSize_.blksPerChunk * 2;
        if (pParityIndex <= firstIndex)
        {
            fe.addr.offset += paritySize;
            feList.push_back(fe);
        }
        else if (firstIndex < pParityIndex && pParityIndex <= lastIndex)
        {
            fe.blkCnt = pParityOffset - startOffset;
            feList.push_back(fe);
            FtEntry feSecond;
            feSecond.addr.stripeId = le.addr.stripeId;
            feSecond.addr.offset = pParityOffset + paritySize;
            feSecond.blkCnt = le.blkCnt - fe.blkCnt;
            feList.push_back(feSecond);
        }
        else
        {
            feList.push_back(fe);
        }
    }
    return feList;
}

vector<uint32_t>
Raid6::GetParityOffset(StripeId lsid)
{
    vector<uint32_t> raid6ParityIndex;

    uint32_t pParityOffset = lsid + dataCnt;
    uint32_t qParityOffset = pParityOffset + 1;

    uint32_t pParityIndex = pParityOffset % chunkCnt;
    uint32_t qParityIndex = qParityOffset % chunkCnt;

    raid6ParityIndex.push_back(pParityIndex);
    raid6ParityIndex.push_back(qParityIndex);

    return raid6ParityIndex;
}

int
Raid6::MakeParity(list<FtWriteEntry>& ftl, const LogicalWriteEntry& src)
{
    vector<uint32_t> parityOffset = GetParityOffset(src.addr.stripeId);
    assert(parityOffset.size() == parityCnt);

    uint32_t pParityIndex = parityOffset.front();
    uint32_t qParityIndex = parityOffset.back();

    FtWriteEntry fwe_pParity;
    FtWriteEntry fwe_qParity;

    fwe_pParity.addr.stripeId = src.addr.stripeId;
    fwe_pParity.addr.offset = (uint64_t)pParityIndex * (uint64_t)ftSize_.blksPerChunk;
    fwe_pParity.blkCnt = ftSize_.blksPerChunk;

    fwe_qParity.addr.stripeId = src.addr.stripeId;
    fwe_qParity.addr.offset = (uint64_t)qParityIndex * (uint64_t)ftSize_.blksPerChunk;
    fwe_qParity.blkCnt = ftSize_.blksPerChunk;
  
    BufferEntry pParity = _AllocChunk();
    BufferEntry qParity = _AllocChunk();

    list<BufferEntry> parities;

    parities.push_back(pParity);
    parities.push_back(qParity);

    _ComputePQParities(parities, *(src.buffers));
    assert(parities.size() == parityCnt);

    pParity = parities.front();
    qParity = parities.back();

    fwe_pParity.buffers.push_back(pParity);
    fwe_qParity.buffers.push_back(qParity);

    ftl.clear();
    ftl.push_back(fwe_pParity);
    ftl.push_back(fwe_qParity);

    return 0;
}

list<FtBlkAddr>
Raid6::GetRebuildGroup(FtBlkAddr fba)
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
Raid6::GetRaidState(vector<ArrayDeviceState> devs)
{
    auto&& abnormalDevs = Enumerable::Where(devs,
        [](auto d) { return d != ArrayDeviceState::NORMAL; });

    POS_TRACE_INFO(EID(RAID_DEBUG_MSG), "GetRaidState from raid6, abnormal cnt:{} ", abnormalDevs.size());
    if (abnormalDevs.size() == 0)
    {
        return RaidState::NORMAL;
    }
    else if (abnormalDevs.size() <= 2)
    {
        return RaidState::DEGRADED;
    }
    return RaidState::FAILURE;
}

bool
Raid6::CheckNumofDevsToConfigure(uint32_t numofDevs)
{
    uint32_t minRequiredNumofDevsforRAID6 = 4;
    return numofDevs >= minRequiredNumofDevsforRAID6;
}

BufferEntry
Raid6::_AllocChunk()
{
    if (parityPools.size() == 0 && parityBufferCntPerNuma > 0)
    {
        POS_TRACE_WARN(EID(RAID_DEBUG_MSG),
            "Attempt to reallocate ParityPool because it is not allocated during creation, req_buffersPerNuma:{}",
            parityBufferCntPerNuma);
        bool ret = AllocParityPools(parityBufferCntPerNuma);
        if (ret == false)
        {
            int eventId = EID(CREATE_ARRAY_INSUFFICIENT_MEMORY_UNABLE_TO_ALLOC_PARITY_POOL);
            POS_TRACE_ERROR(eventId, "required number of buffers:{}", parityBufferCntPerNuma);
        }
    }

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
Raid6::_ComputePQParities(list<BufferEntry>& dst, const list<BufferEntry>& src)
{
    uint32_t i;
    uint64_t* src_ptr = nullptr;
    uint64_t* dst_ptr = nullptr;
    unsigned char *sources[chunkCnt];

    for (i = 0; i < chunkCnt; i++)
    {
        sources[i] = new unsigned char[chunkSize];
	}

    for (const BufferEntry& src_buffer : src)
    {
        src_ptr = (uint64_t*)src_buffer.GetBufferPtr();
        for (i = 0; i < dataCnt; i++)
        {
            memcpy(sources[i], src_ptr + i, chunkSize);
        }
    }

    ec_encode_data(chunkSize, dataCnt, parityCnt,
        g_tbls, sources, &sources[dataCnt]);

    for (const BufferEntry& dst_buffer : dst)
    {
        dst_ptr = (uint64_t*)dst_buffer.GetBufferPtr();
        for (i = 0; i < parityCnt; i++)
        {
            memcpy(dst_ptr + i, sources[chunkCnt - parityCnt + i], chunkSize);
        }
    }

    for ( i = 0; i < chunkCnt; i++)
    {
        delete[] sources[i];
    }
}

void
Raid6::_BindRecoverFunc(void)
{
    using namespace std::placeholders;
    recoverFunc = bind(&Raid6::_RebuildData, this, _1, _2, _3);
}

// TODO: jh34.hong
void
Raid6::_RebuildData(void* dst, void* src, uint32_t dstSize)
{
}

bool
Raid6::AllocParityPools(uint64_t maxParityBufferCntPerNuma,
    AffinityManager* affMgr, MemoryManager* memoryMgr)
{
    affinityManager = affMgr;
    memoryManager = memoryMgr;
    const string NUMA_PREFIX = "_NUMA_";
    const uint64_t ARRAY_CHUNK_SIZE = ArrayConfig::BLOCK_SIZE_BYTE * ArrayConfig::BLOCKS_PER_CHUNK;

    uint32_t totalNumaCount = affinityManager->GetNumaCount();
    
    for (uint32_t numa = 0; numa < totalNumaCount; numa++)
    {
        BufferInfo info = {
            .owner = typeid(this).name() + NUMA_PREFIX + to_string(numa),
            .size = ARRAY_CHUNK_SIZE,
            .count = maxParityBufferCntPerNuma};
        BufferPool* pool = memoryManager->CreateBufferPool(info, numa);
        if (pool == nullptr)
        {
            ClearParityPools();
            return false;
        }
        parityPools.push_back(pool);
        POS_TRACE_DEBUG(EID(RAID_DEBUG_MSG), "BufferPool for RAID6 is created, {}", pool->GetOwner());
    }
    return true;
}

void
Raid6::ClearParityPools()
{
    for (unsigned int i = 0; i < parityPools.size(); i++)
    {
        if (parityPools[i] != nullptr)
        {
            POS_TRACE_DEBUG(EID(RAID_DEBUG_MSG), "ParityPool {} is cleared",
                parityPools[i]->GetOwner());
            memoryManager->DeleteBufferPool(parityPools[i]);
            parityPools[i] = nullptr;
        }
    }
    parityPools.clear();
}

Raid6::~Raid6()
{
    ClearParityPools();
}

int
Raid6::GetParityPoolSize()
{
    return parityPools.size();
}

} // namespace pos
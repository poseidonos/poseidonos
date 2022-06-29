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

    raid6BufferCnt = ftSize_.chunksPerStripe;
    parityBufferCnt = 2;
    srcBufferCnt = raid6BufferCnt - parityBufferCnt;
    raid6BufferSize = ArrayConfig::BLOCK_SIZE_BYTE * ftSize_.blksPerChunk;

    rsCodeSrc = new uint8_t*[raid6BufferCnt];
    for (uint32_t i = 0; i < raid6BufferCnt; i++)
    {
        rsCodeSrc[i] = new uint8_t[raid6BufferSize];
    }
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
    BlkOffset qParityOffset = (uint64_t)qParityIndex * (uint64_t)offsetSizeforParity;

    BlkOffset startOffset = le.addr.offset;
    BlkOffset lastOffset = startOffset + le.blkCnt - 1;
    uint32_t firstIndex = startOffset / ftSize_.blksPerChunk;
    uint32_t lastIndex = lastOffset / ftSize_.blksPerChunk;
    uint32_t maxBlksCnt = ftSize_.blksPerStripe;
 
    list<FtEntry> feList;
    FtEntry fe;
    fe.addr.stripeId = le.addr.stripeId;
    fe.addr.offset = le.addr.offset;
    fe.blkCnt = le.blkCnt;

    if (lastIndex < pParityIndex && pParityIndex < qParityIndex)
    {
        feList.push_back(fe);
    }
    else if (firstIndex < pParityIndex && pParityIndex <= lastIndex)
    {
        fe.blkCnt = pParityOffset - startOffset;
        feList.push_back(fe);

        FtEntry feSecond;
        feSecond.addr.stripeId = le.addr.stripeId;
        feSecond.addr.offset = qParityOffset + offsetSizeforParity;
        feSecond.blkCnt = le.blkCnt - fe.blkCnt;
        feList.push_back(feSecond);
    }
    else
    {
        uint32_t baseOffsetforSecondFeCase = le.addr.offset + qParityOffset + offsetSizeforParity;
        fe.addr.offset = baseOffsetforSecondFeCase % maxBlksCnt;

        if(maxBlksCnt < fe.addr.offset + le.blkCnt)
        {
            fe.blkCnt = maxBlksCnt - fe.addr.offset; 
            feList.push_back(fe);
            
            FtEntry feSecond;
            feSecond.addr.stripeId = le.addr.stripeId;
            feSecond.addr.offset = (baseOffsetforSecondFeCase + fe.blkCnt) % maxBlksCnt;
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

    uint32_t pParityOffset = lsid + srcBufferCnt;
    uint32_t qParityOffset = pParityOffset + 1;

    uint32_t pParityIndex = pParityOffset % raid6BufferCnt;
    uint32_t qParityIndex = qParityOffset % raid6BufferCnt;

    raid6ParityIndex.push_back(pParityIndex);
    raid6ParityIndex.push_back(qParityIndex);

    return raid6ParityIndex;
}

int
Raid6::MakeParity(list<FtWriteEntry>& ftl, const LogicalWriteEntry& src)
{
    vector<uint32_t> parityOffset = GetParityOffset(src.addr.stripeId);
    assert(parityOffset.size() == parityBufferCnt);

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

    vector<uint8_t> encodeMatrixforRaid6(raid6BufferCnt * srcBufferCnt, 0);
    vector<uint8_t> encodeTableforRaid6(srcBufferCnt * parityBufferCnt * 32 ,0);

    gf_gen_cauchy1_matrix(encodeMatrixforRaid6.data(), raid6BufferCnt, srcBufferCnt);
    ec_init_tables(srcBufferCnt, parityBufferCnt, &encodeMatrixforRaid6[srcBufferCnt * srcBufferCnt], encodeTableforRaid6.data());

    for (const BufferEntry& src_buffer : src)
    {
        src_ptr = (uint64_t*)src_buffer.GetBufferPtr();
        for (i = 0; i < srcBufferCnt; i++)
        {
            memcpy(rsCodeSrc[i], src_ptr + i, raid6BufferSize);
        }
    }

    ec_encode_data(raid6BufferSize, srcBufferCnt, parityBufferCnt, encodeTableforRaid6.data(), (uint8_t**)rsCodeSrc, (uint8_t**)&rsCodeSrc[srcBufferCnt]);

    for (const BufferEntry& dst_buffer : dst)
    {
        dst_ptr = (uint64_t*)dst_buffer.GetBufferPtr();
        for (i = 0; i < parityBufferCnt; i++)
        {
            memcpy(dst_ptr + i, rsCodeSrc[raid6BufferCnt - (i + 1)], raid6BufferSize);
        }
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
    for (uint32_t i = 0; i < raid6BufferCnt; i++)
    {
        delete[] rsCodeSrc[i];
    }
    delete[] rsCodeSrc;
}

int
Raid6::GetParityPoolSize()
{
    return parityPools.size();
}

} // namespace pos
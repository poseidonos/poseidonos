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
#include <algorithm>
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
    _MakeEncodingGFTable();
}

list<FtEntry>
Raid6::Translate(const LogicalEntry& le)
{
    FtEntry fe;
    fe.addr.stripeId = le.addr.stripeId;
    fe.addr.offset = le.addr.offset;
    fe.blkCnt = le.blkCnt;

    return list<FtEntry>{fe};
}

vector<uint32_t>
Raid6::GetParityOffset(StripeId lsid)
{
    vector<uint32_t> raid6ParityIndex;

    raid6ParityIndex.push_back(chunkCnt - 2);
    raid6ParityIndex.push_back(chunkCnt - 1);

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
Raid6::GetRebuildGroup(FtBlkAddr fba, const vector<uint32_t>& abnormals)
{
    uint32_t blksPerChunk = ftSize_.blksPerChunk;
    uint32_t offsetInChunk = fba.offset % blksPerChunk;
    uint32_t chunkIndex = fba.offset / blksPerChunk;

    list<FtBlkAddr> recoveryGroup;
    for (uint32_t i = 0; i < ftSize_.chunksPerStripe; i++)
    {
        if (i != chunkIndex && find(abnormals.begin(), abnormals.end(), i) == abnormals.end())
        {
            FtBlkAddr fsa = {.stripeId = fba.stripeId,
                .offset = offsetInChunk + i * blksPerChunk};
            recoveryGroup.push_back(fsa);
        }
    }
    return recoveryGroup;
}

vector<pair<vector<uint32_t>, vector<uint32_t>>>
Raid6::GetRebuildGroupPairs(vector<uint32_t>& targetIndexs)
{
    assert(targetIndexs.size() <= 2);
    vector<pair<vector<uint32_t>, vector<uint32_t>>> rgPair;
    vector<uint32_t> srcIdx;
    for (uint32_t i = 0; i < chunkCnt; i++)
    {
        if (find(targetIndexs.begin(), targetIndexs.end(), i) == targetIndexs.end())
        {
            srcIdx.push_back(i);
        }
    }
    rgPair.emplace_back(make_pair(srcIdx, targetIndexs));
    return rgPair;
}

RaidState
Raid6::GetRaidState(const vector<ArrayDeviceState>& devs)
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
Raid6::_MakeEncodingGFTable()
{
    encodeMatrix = new unsigned char[chunkCnt * dataCnt];
    galoisTable = new unsigned char[dataCnt * parityCnt * galoisTableSize];
    gf_gen_cauchy1_matrix(encodeMatrix, chunkCnt, dataCnt);
    ec_init_tables(dataCnt, parityCnt, &encodeMatrix[dataCnt * dataCnt], galoisTable);
}

void
Raid6::_ComputePQParities(list<BufferEntry>& dst, const list<BufferEntry>& src)
{
    uint64_t* src_ptr = nullptr;
    uint64_t* dst_ptr = nullptr;

    unsigned char* sources[chunkCnt];

    for (uint32_t i = 0; i < chunkCnt; i++)
    {
        sources[i] = new unsigned char[chunkSize];
    }

    int32_t src_iter = 0;
    for (const BufferEntry& src_buffer : src)
    {
        src_ptr = (uint64_t*)src_buffer.GetBufferPtr();
        memcpy(sources[src_iter++], src_ptr, chunkSize);
    }

    // Reed-Solomon encoding using Intel ISA-L API
    ec_encode_data(chunkSize, dataCnt, parityCnt, galoisTable, sources, &sources[dataCnt]);

    int32_t dst_iter = 0;
    for (const BufferEntry& dst_buffer : dst)
    {
        dst_ptr = (uint64_t*)dst_buffer.GetBufferPtr();
        memcpy(dst_ptr, sources[dataCnt + dst_iter++], chunkSize);
    }

    for (uint32_t i = 0; i < chunkCnt; i++)
    {
        delete sources[i];
    }
}

uint32_t
Raid6::_MakeKeyforGFMap(vector<uint32_t>& excluded)
{
    uint32_t key;
    // ascending sort
    sort(excluded.begin(), excluded.end());

    if (excluded.size() == 1)
    {
        key = excluded[0];
    }
    else
    {
        key = ArrayConfig::MAX_CHUNK_CNT * (excluded[0] + 1) + excluded[1];
    }

    return key;
}

void
Raid6::_MakeDecodingGFTable(uint32_t rebuildCnt, vector<uint32_t> excluded, unsigned char* rebuildGaloisTable)
{
    uint32_t destCnt = excluded.size();
    unsigned char* tempMatrix = new unsigned char[dataCnt * dataCnt];
    unsigned char* invertMatrix = new unsigned char[dataCnt * dataCnt];
    unsigned char* decodeMatrix = new unsigned char[dataCnt * dataCnt];

    unsigned char err_index[chunkCnt];
    memset(err_index, 0, sizeof(err_index));

    // Order the fragments in erasure for easier sorting
    for (uint32_t i = 0; i < destCnt; i++)
    {
        err_index[excluded[i]] = 1;
    }

    // Construct matrix that encoded remaining frags by removing erased rows
    for (uint32_t i = 0, r = 0; i < rebuildCnt; i++, r++)
    {
        //r is the index of the survived buffers
        while (err_index[r])
        {
            r++;
        }
        for (uint32_t j = 0; j < dataCnt; j++)
        {
            tempMatrix[dataCnt * i + j] = encodeMatrix[dataCnt * r + j];
        }
    }

    // Invert matrix to get recovery matrix
    gf_invert_matrix(tempMatrix, invertMatrix, dataCnt);

    // Get decode matrix with only wanted recovery rows
    for (uint32_t i = 0; i < destCnt; i++)
    {
        // We lost one of the buffers containing the data
        if (excluded[i] < dataCnt)
        {
            for (uint32_t j = 0; j < dataCnt; j++)
            {
                decodeMatrix[dataCnt * i + j] = invertMatrix[dataCnt * excluded[i] + j];
            }
        }
        // We lost one of the parity buffers containing the error correction codes
        else
        {
            // For parity buffers, need to multiply encode matrix * invert
            for (uint32_t pidx = 0; pidx < dataCnt; pidx++)
            {
                unsigned char s = 0;
                for (uint32_t j = 0; j < dataCnt; j++)
                {
                    s ^= gf_mul(invertMatrix[j * dataCnt + pidx], encodeMatrix[dataCnt * excluded[i] + j]);
                }
                decodeMatrix[dataCnt * i + pidx] = s;
            }
        }
    }

    ec_init_tables(dataCnt, destCnt, decodeMatrix, rebuildGaloisTable);

    delete[] decodeMatrix;
    delete[] invertMatrix;
    delete[] tempMatrix;
}

void
Raid6::_RebuildData(void* dst, void* src, uint32_t dstSize, vector<uint32_t> targets, vector<uint32_t> abnormals)
{
    vector<uint32_t> merged;
    merge(targets.begin(), targets.end(),
        abnormals.begin(), abnormals.end(), std::back_inserter(merged));

    // Make device index vector for rebuild
    vector<uint32_t> excluded = Enumerable::Distinct(merged,
        [](auto p) { return p; });
    assert(excluded.size() != 0);

    uint32_t destCnt = excluded.size();
    assert(destCnt <= parityCnt);

    dstSize = dstSize / targets.size();
    uint32_t rebuildCnt = chunkCnt - destCnt;

    unsigned char* rebuildInput[dataCnt];
    unsigned char* rebuildOutp[destCnt];

    unsigned char* src_ptr = (unsigned char*)src;
    for (uint32_t i = 0; i < dataCnt; i++)
    {
        rebuildInput[i] = src_ptr + (i * dstSize);
    }

    for (uint32_t i = 0; i < destCnt; i++)
    {
        rebuildOutp[i] = new unsigned char[dstSize];
    }

    // Make key for Galois field table caching during rebuild
    uint32_t key = _MakeKeyforGFMap(excluded);
    unsigned char* rebuildGaloisTable = nullptr;
    auto iter = galoisTableMap.find(key);
    if (iter != galoisTableMap.end())
    {
        // Use Galois field table in Map for caching
        rebuildGaloisTable = iter->second;
    }
    else
    {
        unique_lock<mutex> lock(rebuildMutex);
        auto iter = galoisTableMap.find(key);
        if (iter == galoisTableMap.end())
        {
            // Make Galois field table with given device indices and insert to Map
            rebuildGaloisTable = new unsigned char[dataCnt * parityCnt * galoisTableSize];
            _MakeDecodingGFTable(rebuildCnt, excluded, rebuildGaloisTable);
            POS_TRACE_WARN(EID(RAID_DEBUG_MSG), "a new galois table is inserted, key:{}", key);
            galoisTableMap.insert(make_pair(key, rebuildGaloisTable));
        }
        else
        {
            // Use Galois field table in Map if the key exist within multi-thread environments
            rebuildGaloisTable = iter->second;
        }
    }

    ec_encode_data(dstSize, dataCnt, destCnt, rebuildGaloisTable, rebuildInput, rebuildOutp);
    for (uint32_t i = 0 ; i < targets.size(); i++)
    {
        uint32_t targetVal = targets.at(i);
        for (uint32_t j = 0 ; j < excluded.size(); j++)
        {
            if (targetVal == excluded.at(j))
            {
                memcpy((unsigned char*)dst + i * dstSize, rebuildOutp[j], dstSize);
            }
        }
    }

    for (uint32_t i = 0; i < destCnt; i++)
    {
        delete rebuildOutp[i];
    }
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
    delete[] encodeMatrix;
    delete[] galoisTable;

    for (auto iter = galoisTableMap.begin(); iter != galoisTableMap.end(); iter++)
    {
        delete[](iter->second);
    }
}

int
Raid6::GetParityPoolSize()
{
    return parityPools.size();
}

RecoverFunc
Raid6::GetRecoverFunc(vector<uint32_t> targets, vector<uint32_t> abnormals)
{
    RecoverFunc recoverFunc = bind(&Raid6::_RebuildData, this,
        placeholders::_1, placeholders::_2, placeholders::_3, targets, abnormals);
    return recoverFunc;
}

} // namespace pos
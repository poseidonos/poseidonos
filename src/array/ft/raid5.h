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

#ifndef RAID5_H_
#define RAID5_H_

#include <list>
#include <vector>

#include "method.h"
#include "src/resource_manager/memory_manager.h"
#include "src/cpu_affinity/affinity_manager.h"

namespace pos
{
class PartitionPhysicalSize;
class RebuildBehavior;
class BufferPool;

class Raid5 : public Method
{
public:
    explicit Raid5(const PartitionPhysicalSize* pSize);
    virtual ~Raid5();
    virtual bool AllocParityPools(uint64_t maxParityBufferCntPerNuma,
        AffinityManager* affMgr = AffinityManagerSingleton::Instance(),
        MemoryManager* memoryMgr = MemoryManagerSingleton::Instance());
    virtual void ClearParityPools();
    virtual list<FtEntry> Translate(const LogicalEntry& le) override;
    virtual int MakeParity(list<FtWriteEntry>& ftl, const LogicalWriteEntry& src) override;
    virtual list<FtBlkAddr> GetRebuildGroup(FtBlkAddr fba) override;
    virtual RaidState GetRaidState(vector<ArrayDeviceState> devs) override;
    vector<uint32_t> GetParityOffset(StripeId lsid) override;
    bool CheckNumofDevsToConfigure(uint32_t numofDevs) override;

    // This function is for unit testing only
    virtual int GetParityPoolSize();
private:
    void _BindRecoverFunc(void);
    void _RebuildData(void* dst, void* src, uint32_t size);
    BufferEntry _AllocChunk();
    void _ComputeParityChunk(BufferEntry& dst, const list<BufferEntry>& src);
    void _XorBlocks(void* dst, const void* src, uint32_t memSize);
    void _XorBlocks(void* dst, void* src1, void* src2, uint32_t memSize);
    vector<BufferPool*> parityPools;
    AffinityManager* affinityManager = nullptr;
    MemoryManager* memoryManager = nullptr;
};

} // namespace pos
#endif // RAID5_H_

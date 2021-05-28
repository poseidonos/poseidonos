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

#ifndef RAID5_H_
#define RAID5_H_

#include <list>

#include "method.h"
#include "src/spdk_wrapper/free_buffer_pool.h"

namespace pos
{
class PartitionPhysicalSize;
class RebuildBehavior;

class Raid5 : public Method
{
    friend class ParityLocationWbtCommand;

public:
    Raid5(const PartitionPhysicalSize* physicalSize, const uint64_t parityCount,
      AffinityManager* affinityManager = AffinityManagerSingleton::Instance());
    virtual ~Raid5()
    {
    }
    virtual int Translate(FtBlkAddr&, const LogicalBlkAddr&) override;
    virtual int Convert(list<FtWriteEntry>&, const LogicalWriteEntry&) override;
    virtual list<FtBlkAddr> GetRebuildGroup(FtBlkAddr fba) override;

private:
    virtual void _BindRecoverFunc(void) override;
    void _RebuildData(void* dst, void* src, uint32_t size);
    BufferEntry _AllocBuffer();
    void _ComputeParity(BufferEntry& dst, const list<BufferEntry>& srcs);
    void _XorBlocks(void* dst, const void* src, uint32_t memSize);
    void _XorBlocks(void* dst, void* src1, void* src2, uint32_t memSize);
    uint32_t _GetParityOffset(StripeId lsid);

    uint32_t paritySizeBlock_ = 0;
    FreeBufferPool freeParityPool;
};

} // namespace pos
#endif // RAID5_H_

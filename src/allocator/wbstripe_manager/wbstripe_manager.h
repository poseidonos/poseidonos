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

#include <map>
#include <set>
#include <string>
#include <vector>

#include "src/allocator/block_manager/block_manager.h"
#include "src/allocator/context_manager/context_manager.h"
#include "src/allocator/i_wbstripe_allocator.h"
#include "src/mapper/i_stripemap.h"
#include "src/resource_manager/memory_manager.h"

namespace pos
{
class IVolumeInfoManager;
class IReverseMap;
class BufferPool;
class AllocatorCtx;
class TelemetryPublisher;
class StripeLoadStatus;
class EventScheduler;

class WBStripeManager : public IWBStripeAllocator
{
public:
    WBStripeManager(void) = default;
    WBStripeManager(TelemetryPublisher* tp_, int numVolumes_, IReverseMap* iReverseMap, IVolumeInfoManager* VolManager,
        IStripeMap* iStripeMap, AllocatorCtx* allocCtx, AllocatorAddressInfo* info, ContextManager* ctxMgr,
        BlockManager* blkMgr, StripeLoadStatus* stripeLoadStatus, std::string arrayName, int arrayId,
        MemoryManager* memoryManager = MemoryManagerSingleton::Instance(), EventScheduler* eventScheduler = nullptr);
    WBStripeManager(TelemetryPublisher* tp_, AllocatorAddressInfo* info, ContextManager* ctxMgr, BlockManager* blkMgr,
        std::string arrayName, int arrayId);
    virtual ~WBStripeManager(void);
    virtual void Init(void);
    virtual void Dispose(void);

    virtual Stripe* GetStripe(StripeId wbLsid) override;
    virtual void FreeWBStripeId(StripeId lsid) override;

    virtual bool ReferLsidCnt(StripeAddr& lsa) override;
    virtual void DereferLsidCnt(StripeAddr& lsa, uint32_t blockCount) override;

    virtual int ReconstructActiveStripe(uint32_t volumeId, StripeId wbLsid, VirtualBlkAddr tailVsa, std::map<uint64_t, BlkAddr> revMapInfos) override;
    virtual void FinishStripe(StripeId wbLsid, VirtualBlkAddr tail) override;
    virtual int LoadPendingStripesToWriteBuffer(void) override;

    virtual int FlushAllPendingStripes(void) override;
    virtual int FlushAllPendingStripesInVolume(int volumeId) override;
    virtual int FlushAllPendingStripesInVolume(int volumeId, FlushIoSmartPtr flushIo) override;

    virtual StripeId GetUserStripeId(StripeId vsid) override;

    virtual int FlushAllWbStripes(void);
    virtual void PushStripeToStripeArray(Stripe* stripe); // for UT

protected:
    Stripe* _GetStripe(StripeAddr& lsidEntry);

    Stripe* _FinishActiveStripe(ASTailArrayIdx index);
    bool _FillBlocksToStripe(Stripe* stripe, StripeId wbLsid, BlkOffset startOffset, uint32_t numBlks);
    VirtualBlks _AllocateRemainingBlocks(ASTailArrayIdx index);
    VirtualBlks _GetRemainingBlocks(VirtualBlkAddr tail);
    Stripe* _FinishRemainingBlocks(StripeId wbLsid, BlkOffset startOffset, uint32_t numBlks);
    virtual int _RequestStripeFlush(Stripe* stripe);
    int _ReconstructAS(StripeId vsid, StripeId wbLsid, uint64_t blockCount, ASTailArrayIdx idx, Stripe*& stripe);
    int _ReconstructReverseMap(uint32_t volumeId, Stripe* stripe, uint64_t blockCount, std::map<uint64_t, BlkAddr> revMapInfos);
    void _WaitForStripeFlushComplete(Stripe* stripe);
    void _LoadStripe(StripeAddr from, StripeAddr to);

    std::vector<Stripe*> wbStripeArray;
    BufferPool* stripeBufferPool;

    // DOCs
    IStripeMap* iStripeMap;
    AllocatorAddressInfo* addrInfo;
    ContextManager* contextManager;
    AllocatorCtx* allocCtx;
    BlockManager* blockManager;
    TelemetryPublisher* tp;
    std::string arrayName;
    int arrayId;
    IVolumeInfoManager* volumeManager;
    IReverseMap* iReverseMap;
    uint32_t numVolumes;
    MemoryManager* memoryManager;
    StripeLoadStatus* stripeLoadStatus;
    EventScheduler* eventScheduler;
};

} // namespace pos

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

#include "src/allocator/wbstripe_manager/wbstripe_manager.h"

#include <vector>

#include "src/allocator/context_manager/allocator_ctx/allocator_ctx.h"
#include "src/allocator/context_manager/rebuild_ctx/rebuild_ctx.h"
#include "src/allocator/context_manager/wbstripe_ctx/wbstripe_ctx.h"
#include "src/allocator/stripe/stripe.h"
#include "src/include/branch_prediction.h"
#include "src/io/backend_io/flush_submission.h"
#include "src/logger/logger.h"
#include "src/mapper_service/mapper_service.h"
#include "src/qos/qos_manager.h"
#include "src/spdk_wrapper/free_buffer_pool.h"

namespace pos
{
WBStripeManager::WBStripeManager(StripeVec* stripeVec_, int numVolumes_, IReverseMap* iReverseMap_, IVolumeManager* volManager, IStripeMap* istripeMap_, WbStripeCtx* wbCtx, AllocatorAddressInfo* info, ContextManager* ctxMgr, BlockManager* blkMgr, std::string arrayName, int arrayId)
: stripeBufferPool(nullptr),
  pendingFullStripes(nullptr),
  iStripeMap(istripeMap_),
  addrInfo(info),
  contextManager(ctxMgr),
  blockManager(blkMgr),
  arrayName(arrayName),
  arrayId(arrayId)
{
    wbStripeCtx = wbCtx;
    volumeManager = volManager;
    numVolumes = numVolumes_;
    iReverseMap = iReverseMap_;
    pendingFullStripes = stripeVec_;
    if (stripeVec_ != nullptr)
    {
        // only for UT
        for (auto stripe : *stripeVec_)
        {
            stripesToFlush4FlushCmd[0].push_back(stripe);
        }
    }
}

WBStripeManager::WBStripeManager(AllocatorAddressInfo* info, ContextManager* ctxMgr, BlockManager* blkMgr, std::string arrayName, int arrayId)
: WBStripeManager(nullptr, MAX_VOLUME_COUNT, nullptr, nullptr, nullptr, nullptr, info, ctxMgr, blkMgr, arrayName, arrayId)
{
    wbStripeCtx = ctxMgr->GetWbStripeCtx();
}

WBStripeManager::~WBStripeManager(void)
{
    if ((wbStripeArray.size() != 0) && (stripeBufferPool != nullptr))
    {
        Dispose();
    }
}

void
WBStripeManager::Init(void)
{
    if (iStripeMap == nullptr) // for UT
    {
        iStripeMap = MapperServiceSingleton::Instance()->GetIStripeMap(arrayId);
    }
    if (volumeManager == nullptr) // for UT
    {
        volumeManager = VolumeServiceSingleton::Instance()->GetVolumeManager(arrayId);
    }
    if (iReverseMap == nullptr)
    {
        iReverseMap = MapperServiceSingleton::Instance()->GetIReverseMap(arrayId);
    }
    uint32_t totalNvmStripes = addrInfo->GetnumWbStripes();
    uint32_t chunksPerStripe = addrInfo->GetchunksPerStripe();

    stripeBufferPool = new FreeBufferPool(totalNvmStripes * chunksPerStripe, CHUNK_SIZE);

    for (uint32_t stripeCnt = 0; stripeCnt < totalNvmStripes; ++stripeCnt)
    {
        Stripe* stripe = new Stripe(true, addrInfo);

        for (uint32_t chunkCnt = 0; chunkCnt < chunksPerStripe; ++chunkCnt)
        {
            void* buffer = stripeBufferPool->GetBuffer();
            stripe->AddDataBuffer(buffer);
        }
        wbStripeArray.push_back(stripe);
    }
}

void
WBStripeManager::Dispose(void)
{
    for (auto& stripeToClear : wbStripeArray)
    {
        if (nullptr != stripeToClear)
        {
            delete stripeToClear;
            stripeToClear = nullptr;
        }
    }
    wbStripeArray.clear();

    if (nullptr != stripeBufferPool)
    {
        delete stripeBufferPool;
        stripeBufferPool = nullptr;
    }
}

Stripe*
WBStripeManager::GetStripe(StripeAddr& lsa)
{
    if (iStripeMap->IsInUserDataArea(lsa))
    {
        return nullptr;
    }
    return wbStripeArray[lsa.stripeId];
}

StripeId
WBStripeManager::AllocateUserDataStripeId(StripeId vsid)
{
    return vsid;
}

void
WBStripeManager::FreeWBStripeId(StripeId lsid)
{
    std::lock_guard<std::mutex> lock(contextManager->GetCtxLock());
    assert(!IsUnMapStripe(lsid));
    wbStripeCtx->ReleaseWbStripe(lsid);
    QosManagerSingleton::Instance()->DecreaseUsedStripeCnt(arrayName);
}

void
WBStripeManager::FlushActiveStripes(uint32_t volumeId)
{
    if (volumeManager->GetVolumeStatus(volumeId) == Mounted)
    {
        PickActiveStripe(volumeId, stripesToFlush4FlushCmd[volumeId], vsidToCheckFlushDone4FlushCmd[volumeId]);
    }
    // This variable is not used.
    vsidToCheckFlushDone4FlushCmd[volumeId].clear();
}

void
WBStripeManager::GetWbStripes(FlushIoSmartPtr flushIo)
{
    uint32_t volumeId = flushIo->GetVolumeId();
    for (auto it = wbStripeArray.begin(); it != wbStripeArray.end(); ++it)
    {
        Stripe* arrStripe = *it;
        if (volumeId != arrStripe->GetAsTailArrayIdx())
        {
            continue;
        }
        arrStripe->UpdateFlushIo(flushIo);
    }
}

bool
WBStripeManager::ReferLsidCnt(StripeAddr& lsa)
{
    Stripe* stripe = GetStripe(lsa);
    if (nullptr == stripe)
    {
        return false;
    }

    stripe->Refer();
    return true;
}

void
WBStripeManager::DereferLsidCnt(StripeAddr& lsa, uint32_t blockCount)
{
    Stripe* stripe = GetStripe(lsa);
    if (nullptr == stripe)
    {
        return;
    }
    stripe->Derefer(blockCount);
}

void
WBStripeManager::FlushAllActiveStripes(void)
{
    std::vector<Stripe*> stripesToFlush;
    std::vector<StripeId> vsidToCheckFlushDone;

    CheckAllActiveStripes(stripesToFlush, vsidToCheckFlushDone);
    FinalizeWriteIO(stripesToFlush, vsidToCheckFlushDone);

    for (auto it = wbStripeArray.begin(); it != wbStripeArray.end(); ++it)
    {
        while ((*it)->GetBlksRemaining() > 0)
        {
            usleep(1);
        }
        while ((*it)->IsFinished() == false)
        {
            usleep(1);
        }
    }
}

int
WBStripeManager::ReconstructActiveStripe(uint32_t volumeId, StripeId wbLsid, VirtualBlkAddr tailVsa, std::map<uint64_t, BlkAddr> revMapInfos)
{
    Stripe* stripe;
    int ret = _ReconstructAS(tailVsa.stripeId, wbLsid, tailVsa.offset, volumeId, stripe);
    if (ret < 0)
    {
        return ret;
    }

    ret = _ReconstructReverseMap(volumeId, stripe, tailVsa.offset, revMapInfos);
    return ret;
}

void
WBStripeManager::SetActiveStripeTail(uint32_t volumeId, VirtualBlkAddr tail, StripeId wbLsid)
{
    wbStripeCtx->SetActiveStripeTail(volumeId, tail);
}

int
WBStripeManager::FlushPendingActiveStripes(void)
{
    int ret = 0;

    if (nullptr != pendingFullStripes)
    {
        for (Stripe* stripe : *pendingFullStripes)
        {
            ret = _RequestStripeFlush(stripe);
            if (ret < 0)
            {
                return ret;
            }

            POS_TRACE_DEBUG(EID(ALLOCATOR_TRIGGER_FLUSH), "Request stripe flush, vsid {} lsid {} remaining {}",
                stripe->GetVsid(), stripe->GetWbLsid(), stripe->GetBlksRemaining());
        }

        delete pendingFullStripes;
        pendingFullStripes = nullptr;
    }

    return ret;
}

int
WBStripeManager::PrepareRebuild(void)
{
    std::vector<Stripe*> stripesToFlush;
    std::vector<StripeId> vsidToCheckFlushDone;

    POS_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "Start @PrepareRebuild()");
    blockManager->TurnOffBlkAllocation();

    // Check rebuildTargetSegments data structure
    int ret = contextManager->MakeRebuildTarget();
    if (ret <= NO_REBUILD_TARGET_USER_SEGMENT)
    {
        blockManager->TurnOnBlkAllocation();
        return ret;
    }
    POS_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "MakeRebuildTarget Done @PrepareRebuild()");

    // Let nextSsdLsid point non-rebuild target segment
    ret = contextManager->SetNextSsdLsid();
    if (ret < 0)
    {
        blockManager->TurnOnBlkAllocation();
        return ret;
    }
    POS_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "SetNextSsdLsid Done @PrepareRebuild()");

    // Online stripes beyond target segments should be flushed
    ret = _FlushOnlineStripes(vsidToCheckFlushDone);
    if (ret < 0)
    {
        blockManager->TurnOnBlkAllocation();
        return ret;
    }

    ret = CheckAllActiveStripes(stripesToFlush, vsidToCheckFlushDone);
    FinalizeWriteIO(stripesToFlush, vsidToCheckFlushDone);
    POS_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "Stripes Flush Done @PrepareRebuild()");

    blockManager->TurnOnBlkAllocation();
    POS_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "End @PrepareRebuild()");

    return 0;
}

Stripe*
WBStripeManager::GetStripe(StripeId wbLsid)
{
    return wbStripeArray[wbLsid];
}

//------------------------------------------------------------------------------
int
WBStripeManager::_FlushOnlineStripes(std::vector<StripeId>& vsidToCheckFlushDone)
{
    // Flush Online Stripes Beyond Target Segment
    RebuildCtx* rbCtx = contextManager->GetRebuildCtx();
    for (auto it = rbCtx->GetRebuildTargetSegmentsBegin(); it != rbCtx->GetRebuildTargetSegmentsEnd(); ++it)
    {
        StripeId startVsid = *it * addrInfo->GetstripesPerSegment();
        StripeId endVsid = startVsid + addrInfo->GetstripesPerSegment();
        for (StripeId vsid = startVsid; vsid < endVsid; ++vsid)
        {
            StripeAddr lsa = iStripeMap->GetLSA(vsid);
            if (lsa.stripeId != UNMAP_STRIPE && iStripeMap->IsInWriteBufferArea(lsa))
            {
                Stripe* stripe = GetStripe(lsa);
                if (stripe != nullptr && stripe->IsFinished() == false && stripe->GetBlksRemaining() == 0)
                {
                    vsidToCheckFlushDone.push_back(stripe->GetVsid());
                }
            }
        }
    }

    return 0;
}

int
WBStripeManager::CheckAllActiveStripes(std::vector<Stripe*>& stripesToFlush, std::vector<StripeId>& vsidToCheckFlushDone)
{
    for (uint32_t volumeId = 0; volumeId < numVolumes; ++volumeId)
    {
        PickActiveStripe(volumeId, stripesToFlush, vsidToCheckFlushDone);
    }
    return 0;
}

void
WBStripeManager::PickActiveStripe(uint32_t volumeId, std::vector<Stripe*>& stripesToFlush, std::vector<StripeId>& vsidToCheckFlushDone)
{
    Stripe* activeStripe = nullptr;
    ASTailArrayIdx index = volumeId;

    activeStripe = _FinishActiveStripe(index);
    if (activeStripe != nullptr)
    {
        POS_TRACE_INFO(EID(PICKUP_ACTIVE_STRIPE),
            "Picked Active Stripe: index:{}  wbLsid:{}  vsid:{}  remaining:{}", index,
            activeStripe->GetWbLsid(), activeStripe->GetVsid(),
            activeStripe->GetBlksRemaining());
        stripesToFlush.push_back(activeStripe);
        vsidToCheckFlushDone.push_back(activeStripe->GetVsid());
    }
}

Stripe*
WBStripeManager::FinishReconstructedStripe(StripeId wbLsid, VirtualBlkAddr tail)
{
    VirtualBlks remainingVsaRange = _AllocateRemainingBlocks(tail);
    return _FinishRemainingBlocks(remainingVsaRange);
}

void
WBStripeManager::PushStripeToStripeArray(Stripe* stripe)
{
    // for UT
    wbStripeArray.push_back(stripe);
}

int
WBStripeManager::_ReconstructAS(StripeId vsid, StripeId wbLsid, uint64_t blockCount, ASTailArrayIdx tailarrayidx, Stripe*& stripe)
{
    if (0 == blockCount)
    {
        POS_TRACE_ERROR(EID(WRONG_BLOCK_COUNT), "Wrong blockCount:{}", blockCount);
        return -EID(WRONG_BLOCK_COUNT);
    }

    stripe = GetStripe(wbLsid);
    stripe->Assign(vsid, wbLsid, tailarrayidx);

    uint32_t remainingBlks = stripe->DecreseBlksRemaining(blockCount);
    if (remainingBlks == 0)
    {
        if (nullptr == pendingFullStripes)
        {
            pendingFullStripes = new StripeVec;
        }

        pendingFullStripes->push_back(stripe);

        POS_TRACE_DEBUG(EID(ALLOCATOR_REPLAYED_STRIPE_IS_FULL),
            "Stripe (vsid {}, wbLsid {}) is waiting to be flushed", vsid, wbLsid);
    }

    POS_TRACE_DEBUG(EID(ALLOCATOR_RECONSTRUCT_STRIPE),
        "Stripe (vsid {}, wbLsid {}, blockCount {}, remainingBlks {}) is reconstructed",
        vsid, wbLsid, blockCount, remainingBlks);

    return 0;
}

int
WBStripeManager::_ReconstructReverseMap(uint32_t volumeId, Stripe* stripe, uint64_t blockCount, std::map<uint64_t, BlkAddr> revMapInfos)
{
    int ret = 0;
    // TODO (jk.man.kim): Don't forget to insert array name in the future.
    StripeId wbLsid = stripe->GetWbLsid();
    ret = stripe->LinkReverseMap(iReverseMap->GetReverseMapPack(wbLsid));
    if (unlikely(ret < 0))
    {
        return ret;
    }

    ret = stripe->ReconstructReverseMap(volumeId, blockCount, revMapInfos);
    if (ret < 0)
    {
        POS_TRACE_INFO(EID(REVMAP_RECONSTRUCT_NOT_FOUND_RBA), "There was no vsa map entry for some blocks");
    }

    return ret;
}

Stripe*
WBStripeManager::_FinishActiveStripe(ASTailArrayIdx index)
{
    VirtualBlks remainingVsaRange = _AllocateRemainingBlocks(index);
    return _FinishRemainingBlocks(remainingVsaRange);
}

VirtualBlks
WBStripeManager::_AllocateRemainingBlocks(ASTailArrayIdx index)
{
    std::unique_lock<std::mutex> lock(wbStripeCtx->GetActiveStripeTailLock(index));
    VirtualBlkAddr tail = wbStripeCtx->GetActiveStripeTail(index);
    VirtualBlks remainingBlocks = _AllocateRemainingBlocks(tail);
    wbStripeCtx->SetActiveStripeTail(index, UNMAP_VSA);

    return remainingBlocks;
}

VirtualBlks
WBStripeManager::_AllocateRemainingBlocks(VirtualBlkAddr tail)
{
    VirtualBlks remainingBlks;

    // Nothing to do, 'ActiveStripeTailArray Index' is unused or already done
    if (UNMAP_OFFSET == tail.offset)
    {
        remainingBlks.startVsa = UNMAP_VSA;
        remainingBlks.numBlks = 0;
        return remainingBlks;
    }
    else if (tail.offset > addrInfo->GetblksPerStripe())
    {
        POS_TRACE_ERROR(EID(PICKUP_ACTIVE_STRIPE), "offsetInTail:{} > blksPerStirpe:{}", tail.offset, addrInfo->GetblksPerStripe());
        while (addrInfo->IsUT() != true)
        {
            usleep(1); // assert(false);
        }
    }

    remainingBlks.numBlks = addrInfo->GetblksPerStripe() - tail.offset;
    // if remaining blocks is empty, stripe wouldn't be in StripePool
    if (remainingBlks.numBlks == 0)
    {
        remainingBlks.startVsa = UNMAP_VSA;
    }
    else
    {
        remainingBlks.startVsa = tail;
    }

    return remainingBlks;
}

Stripe*
WBStripeManager::_FinishRemainingBlocks(VirtualBlks remainingVsaRange)
{
    Stripe* activeStripe = nullptr;
    StripeId vsid = remainingVsaRange.startVsa.stripeId;

    if (vsid != UNMAP_STRIPE)
    {
        StripeAddr lsa = iStripeMap->GetLSA(vsid);
        activeStripe = wbStripeArray[lsa.stripeId];

        uint32_t startBlock = remainingVsaRange.startVsa.offset;
        uint32_t lastBlock = startBlock + remainingVsaRange.numBlks - 1;
        for (uint32_t block = startBlock; block <= lastBlock; ++block)
        {
            activeStripe->UpdateReverseMap(block, INVALID_RBA, UINT32_MAX);
        }
        uint32_t remain = activeStripe->DecreseBlksRemaining(remainingVsaRange.numBlks);
        if (remain == 0)
        {
            POS_TRACE_DEBUG(EID(ALLOCATOR_TRIGGER_FLUSH), "Flush stripe (vsid {})", vsid);
            int ret = _RequestStripeFlush(activeStripe);
            if (ret != 0)
            {
                POS_TRACE_DEBUG(EID(ALLOCATOR_TRIGGER_FLUSH), "request stripe flush failed");
            }
        }
    }
    return activeStripe;
}

void
WBStripeManager::FinalizeWriteIO(std::vector<Stripe*>& stripesToFlush, std::vector<StripeId>& vsidToCheckFlushDone)
{
    // Wait for write I/O residues on active stripes then flush
    for (auto& stripe : stripesToFlush)
    {
        while (stripe->GetBlksRemaining() > 0)
        {
            usleep(1);
        }
    }
    // Check if flushing has been completed
    for (auto vsid : vsidToCheckFlushDone)
    {
        Stripe* stripe = nullptr;
        do
        {
            StripeAddr lsa = iStripeMap->GetLSA(vsid);
            stripe = GetStripe(lsa);
        } while (stripe != nullptr);
    }
}

int
WBStripeManager::_RequestStripeFlush(Stripe* stripe)
{
    EventSmartPtr event(new FlushSubmission(stripe, arrayId));
    return stripe->Flush(event);
}

} // namespace pos

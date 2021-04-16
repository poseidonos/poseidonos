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

#include "src/allocator/context_manager/active_stripe_index_info.h"
#include "src/allocator/wb_stripe_manager/wbstripe_manager.h"
#include "src/io/backend_io/flush_read_submission.h"
#include "src/mapper_service/mapper_service.h"
#include "src/spdk_wrapper/free_buffer_pool.h"
#include "src/include/branch_prediction.h"
#include "src/gc/gc_flush_submission.h"
#include "src/qos/qos_manager.h"

#include <vector>

namespace pos
{

WBStripeManager::WBStripeManager(AllocatorAddressInfo* info, ContextManager* ctxMgr, BlockManager* blkMgr, std::string arrayName)
: stripeBufferPool(nullptr),
  pendingFullStripes(nullptr),
  iStripeMap(nullptr),
  addrInfo(info),
  contextManager(ctxMgr),
  blockManager(blkMgr),
  arrayName(arrayName)
{
}

WBStripeManager::~WBStripeManager(void)
{
    for (auto& stripeToClear : wbStripeArray)
    {
        delete stripeToClear;
    }
    delete stripeBufferPool;
}

void
WBStripeManager::Init(void)
{
    iStripeMap = MapperServiceSingleton::Instance()->GetIStripeMap(arrayName);

    uint32_t totalNvmStripes = addrInfo->GetnumWbStripes();
    uint32_t chunksPerStripe = addrInfo->GetchunksPerStripe();

    stripeBufferPool = new FreeBufferPool(totalNvmStripes * chunksPerStripe, CHUNK_SIZE);

    for (uint32_t stripeCnt = 0; stripeCnt < totalNvmStripes; ++stripeCnt)
    {
        Stripe* stripe = new Stripe(arrayName);

        for (uint32_t chunkCnt = 0; chunkCnt < chunksPerStripe; ++chunkCnt)
        {
            void* buffer = stripeBufferPool->GetBuffer();
            stripe->AddDataBuffer(buffer);
        }
        wbStripeArray.push_back(stripe);
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
    contextManager->GetWbLsidBitmap()->ClearBit(lsid);
    QosManagerSingleton::Instance()->DecreaseUsedStripeCnt();
}

void
WBStripeManager::GetAllActiveStripes(uint32_t volumeId)
{
    IVolumeManager* volMgr = VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);
    if (volMgr == nullptr)
    {
        POS_TRACE_WARN((int)POS_EVENT_ID::VOL_NOT_EXIST, "The requested volume does not exist");
    }
    else if (volMgr->GetVolumeStatus(volumeId) == Mounted)
    {
        PickActiveStripe(volumeId, stripesToFlush4FlushCmd[volumeId], vsidToCheckFlushDone4FlushCmd[volumeId]);
    }
    // This variable is not used.
    vsidToCheckFlushDone4FlushCmd[volumeId].clear();
}

// Wait for pending writes on the closed stripes
bool
WBStripeManager::WaitPendingWritesOnStripes(uint32_t volumeId)
{
    std::vector<Stripe*> &stripesToFlush = stripesToFlush4FlushCmd[volumeId];
    auto stripe = stripesToFlush.begin();
    while (stripe != stripesToFlush.end())
    {
        if ((*stripe)->GetBlksRemaining() == 0)
        {
            stripe = stripesToFlush.erase(stripe);
        }
        else
        {
            // Writes on closed stripe still in progress
            stripe = stripesToFlush.erase(stripe);
        }
    }

    return stripesToFlush.size() == 0;
}

bool
WBStripeManager::WaitStripesFlushCompletion(uint32_t volumeId)
{
    // Need to check for stripes belonging to requested volumes stipes only including GC stripe of that volume
    uint32_t volumeIdGC = volumeId + MAX_VOLUME_COUNT;

    for (auto it = wbStripeArray.begin(); it != wbStripeArray.end(); ++it)
    {
        if (volumeId != (*it)->GetAsTailArrayIdx() || volumeIdGC != (*it)->GetAsTailArrayIdx())
        {
            continue;
        }
        if ((*it)->GetBlksRemaining() > 0)
        {
            if ((*it)->GetBlksRemaining() + contextManager->GetActiveStripeTail(volumeId).offset == addrInfo->GetblksPerStripe())
            {
                continue;
            }
        }

        StripeAddr lsa = iStripeMap->GetLSA((*it)->GetVsid());
        Stripe* stripe = GetStripe(lsa);

        if (stripe != nullptr)
        {
            return false;
        }
    }

    return true;
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
WBStripeManager::ReconstructActiveStripe(uint32_t volumeId, StripeId wbLsid, VirtualBlkAddr tailVsa, ASTailArrayIdx tailarrayidx)
{
    Stripe* stripe;
    int ret = _ReconstructAS(tailVsa.stripeId, wbLsid, tailVsa.offset, tailarrayidx, stripe);
    if (ret < 0)
    {
        return ret;
    }

    ret = _ReconstructReverseMap(volumeId, stripe, tailVsa.offset);
    return ret;
}

int
WBStripeManager::RestoreActiveStripeTail(int tailarrayidx, VirtualBlkAddr tail, StripeId wbLsid)
{
    contextManager->SetActiveStripeTail(tailarrayidx, tail);
    uint32_t volumeId = ActiveStripeTailArrIdxInfo::GetVolumeId(tailarrayidx);
    return ReconstructActiveStripe(volumeId, wbLsid, tail, tailarrayidx);
}

int
WBStripeManager::FlushPendingActiveStripes(void)
{
    int ret = 0;

    if (nullptr != pendingFullStripes)
    {
        for (Stripe* stripe : *pendingFullStripes)
        {
            ret = _RequestStripeFlush(*stripe);
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
    contextManager->TurnOffBlkAllocation();

    // Check rebuildTargetSegments data structure
    int ret = _MakeRebuildTarget();
    if (ret <= NO_REBUILD_TARGET_USER_SEGMENT)
    {
        contextManager->TurnOnBlkAllocation();
        return ret;
    }
    POS_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "MakeRebuildTarget Done @PrepareRebuild()");

    // Let nextSsdLsid point non-rebuild target segment
    ret = contextManager->SetNextSsdLsid();
    if (ret < 0)
    {
        contextManager->TurnOnBlkAllocation();
        return ret;
    }
    POS_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "SetNextSsdLsid Done @PrepareRebuild()");

    // Online stripes beyond target segments should be flushed
    ret = _FlushOnlineStripes(vsidToCheckFlushDone);
    if (ret < 0)
    {
        contextManager->TurnOnBlkAllocation();
        return ret;
    }
    ret = CheckAllActiveStripes(stripesToFlush, vsidToCheckFlushDone);
    FinalizeWriteIO(stripesToFlush, vsidToCheckFlushDone);
    POS_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "Stripes Flush Done @PrepareRebuild()");

    contextManager->TurnOnBlkAllocation();
    POS_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "End @PrepareRebuild()");

    return 0;
}

int
WBStripeManager::StopRebuilding(void)
{
    std::unique_lock<std::mutex> lock(contextManager->GetCtxLock());
    POS_TRACE_INFO(EID(ALLOCATOR_START), "@StopRebuilding");

    RebuildCtx* rbCtx = contextManager->GetRebuldCtx();
    if (rbCtx->GetTargetSegmentCnt() == 0)
    {
        POS_TRACE_INFO(EID(ALLOCATOR_REBUILD_TARGET_SET_EMPTY), "Rebuild was already done or not happen");
        return -EID(ALLOCATOR_REBUILD_TARGET_SET_EMPTY);
    }

    // Clear rebuildTargetSegments
    rbCtx->ClearRebuildTargetSegments();
    rbCtx->FlushRebuildCtx();
    rbCtx->SetUnderRebuildSegmentId(UINT32_MAX);
    return 0;
}

Stripe*
WBStripeManager::GetStripe(StripeId wbLsid)
{
    return wbStripeArray[wbLsid];
}

//------------------------------------------------------------------------------

int
WBStripeManager::_MakeRebuildTarget(void)
{
    POS_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "@MakeRebuildTarget()");
    RebuildCtx* rbCtx = contextManager->GetRebuldCtx();
    SegmentCtx* segCtx = contextManager->GetSegmentCtx();

    if (rbCtx->IsRebuidTargetSegmentsEmpty() == false)
    {
        POS_TRACE_WARN(EID(ALLOCATOR_REBUILD_TARGET_SET_NOT_EMPTY), "rebuildTargetSegments is NOT empty!");
        for (auto it = rbCtx->RebuildTargetSegmentsBegin(); it != rbCtx->RebuildTargetSegmentsEnd(); ++it)
        {
            POS_TRACE_WARN(EID(ALLOCATOR_REBUILD_TARGET_SET_NOT_EMPTY), "residue was segmentId:{}", *it);
        }
        rbCtx->ClearRebuildTargetSegments();
    }

    // Pick non-free segments and make rebuildTargetSegments
    SegmentId segmentId = 0;
    while (true)
    {
        segmentId = segCtx->GetSegmentBitmap()->FindFirstSetBit(segmentId);
        if (segCtx->GetSegmentBitmap()->IsValidBit(segmentId) == false)
        {
            break;
        }

        auto pr = rbCtx->EmplaceRebuildTargetSegment(segmentId);
        POS_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "segmentId:{} is inserted as target to rebuild", segmentId);
        if (pr.second == false)
        {
            POS_TRACE_ERROR(EID(ALLOCATOR_MAKE_REBUILD_TARGET_FAILURE), "segmentId:{} is already in set", segmentId);
            return -EID(ALLOCATOR_MAKE_REBUILD_TARGET_FAILURE);
        }
        ++segmentId;
    }

    rbCtx->FlushRebuildCtx(); // Store target segments info to MFS
    return rbCtx->GetTargetSegmentCnt();
}

int
WBStripeManager::_FlushOnlineStripes(std::vector<StripeId>& vsidToCheckFlushDone)
{
    // Flush Online Stripes Beyond Target Segment
    RebuildCtx* rbCtx = contextManager->GetRebuldCtx();

    for (auto it = rbCtx->RebuildTargetSegmentsBegin(); it != rbCtx->RebuildTargetSegmentsEnd(); ++it)
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
    for (uint32_t volumeId = 0; volumeId < MAX_VOLUME_COUNT; ++volumeId)
    {
        PickActiveStripe(volumeId, stripesToFlush, vsidToCheckFlushDone);
    }

    return 0;
}

void
WBStripeManager::PickActiveStripe(uint32_t volumeId, std::vector<Stripe*>& stripesToFlush, std::vector<StripeId>& vsidToCheckFlushDone)
{
    Stripe* activeStripe = nullptr;

    for (ASTailArrayIdx index = volumeId; index < ACTIVE_STRIPE_TAIL_ARRAYLEN; index += MAX_VOLUME_COUNT)
    {
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
}

Stripe*
WBStripeManager::FinishReconstructedStripe(StripeId wbLsid, VirtualBlkAddr tail)
{
    VirtualBlks remainingVsaRange = _AllocateRemainingBlocks(tail);
    return _FinishRemainingBlocks(remainingVsaRange);
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
WBStripeManager::_ReconstructReverseMap(uint32_t volumeId, Stripe* stripe, uint64_t blockCount)
{
    int ret = 0;
    // TODO (jk.man.kim): Don't forget to insert array name in the future.
    IReverseMap* iReverseMap = MapperServiceSingleton::Instance()->GetIReverseMap(arrayName);

    ret = iReverseMap->LinkReverseMap(stripe, stripe->GetWbLsid(), stripe->GetVsid());
    if (unlikely(ret < 0))
    {
        return ret;
    }

    ret = stripe->ReconstructReverseMap(volumeId, blockCount);
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
    std::unique_lock<std::mutex> lock(contextManager->GetActiveStripeTailLock(index));
    VirtualBlkAddr tail = contextManager->GetActiveStripeTail(index);

    VirtualBlks remainingBlocks = _AllocateRemainingBlocks(tail);
    contextManager->SetActiveStripeTail(index, UNMAP_VSA);

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
        assert(false);
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
            int ret = 0;
            uint32_t asTailArrayIndex = activeStripe->GetAsTailArrayIdx();
            if (asTailArrayIndex < MAX_VOLUME_COUNT)
            {
                EventSmartPtr event(new FlushReadSubmission(activeStripe, arrayName));
                ret = activeStripe->Flush(event);
            }
            else
            {
                EventSmartPtr event(new GcFlushSubmission(activeStripe, arrayName));
                ret = activeStripe->Flush(event);
            }

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
WBStripeManager::_RequestStripeFlush(Stripe& stripe)
{
    EventSmartPtr event(new FlushReadSubmission(&stripe, arrayName));
    return stripe.Flush(event);
}

} // namespace pos

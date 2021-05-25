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

#include "src/allocator/block_manager/block_manager.h"

#include <string>

#include "src/allocator/context_manager/active_stripe_index_info.h"
#include "src/allocator/context_manager/allocator_ctx/allocator_ctx.h"
#include "src/allocator/context_manager/segment_ctx/segment_ctx.h"
#include "src/allocator/context_manager/wbstripe_ctx/wbstripe_ctx.h"
#include "src/include/branch_prediction.h"
#include "src/logger/logger.h"
#include "src/mapper_service/mapper_service.h"
#include "src/qos/qos_manager.h"

namespace pos
{
BlockManager::BlockManager(SegmentCtx* segCtx_, AllocatorCtx* allocCtx_, WbStripeCtx* wbCtx_, AllocatorAddressInfo* info, ContextManager* ctxMgr, std::string arrayName)
: userBlkAllocProhibited(false),
  addrInfo(info),
  contextManager(ctxMgr),
  iWBStripeInternal(nullptr),
  arrayName(arrayName)
{
    segCtx = segCtx_;
    allocCtx = allocCtx_;
    wbStripeCtx = wbCtx_;
}

BlockManager::BlockManager(AllocatorAddressInfo* info, ContextManager* ctxMgr, std::string arrayName)
: BlockManager(nullptr, nullptr, nullptr, info, contextManager, arrayName)
{
    segCtx = contextManager->GetSegmentCtx();
    allocCtx = contextManager->GetAllocatorCtx();
    wbStripeCtx = contextManager->GetWbStripeCtx();
}

void
BlockManager::Init(IWBStripeInternal* iwbstripeInternal)
{
    iWBStripeInternal = iwbstripeInternal;

    for (int volume = 0; volume < MAX_VOLUME_COUNT; volume++)
    {
        blkAllocProhibited[volume] = false;
    }
}

VirtualBlks
BlockManager::AllocateWriteBufferBlks(uint32_t volumeId, uint32_t numBlks, bool forGC)
{
    VirtualBlks allocatedBlks;

    if ((blkAllocProhibited[volumeId] == true) || ((forGC == false) && (userBlkAllocProhibited == true)))
    {
        allocatedBlks.startVsa = UNMAP_VSA;
        allocatedBlks.numBlks = 0;
        return allocatedBlks;
    }

    ActiveStripeTailArrIdxInfo info = {volumeId, forGC};
    allocatedBlks = _AllocateBlks(info.GetActiveStripeTailArrIdx(), numBlks);
    return allocatedBlks;
}

void
BlockManager::InvalidateBlks(VirtualBlks blks)
{
    SegmentId segId = blks.startVsa.stripeId / addrInfo->GetstripesPerSegment();
    uint32_t validCount = segCtx->DecreaseValidBlockCount(segId, blks.numBlks);
    if (validCount == 0)
    {
        contextManager->FreeUserDataSegment(segId);
    }
}

void
BlockManager::ValidateBlks(VirtualBlks blks)
{
    SegmentId segId = blks.startVsa.stripeId / addrInfo->GetstripesPerSegment();
    segCtx->IncreaseValidBlockCount(segId, blks.numBlks);
}

void
BlockManager::ProhibitUserBlkAlloc(void)
{
    userBlkAllocProhibited = true;
}

void
BlockManager::PermitUserBlkAlloc(void)
{
    userBlkAllocProhibited = false;
}

bool
BlockManager::BlockAllocating(uint32_t volumeId)
{
    return (blkAllocProhibited[volumeId].exchange(true) == false);
}

void
BlockManager::UnblockAllocating(uint32_t volumeId)
{
    blkAllocProhibited[volumeId] = false;
}

void
BlockManager::TurnOffBlkAllocation(void)
{
    for (auto i = 0; i < MAX_VOLUME_COUNT; i++)
    {
        // Wait for flag to be reset
        while (blkAllocProhibited[i].exchange(true) == true)
        {
        }
    }
}

void
BlockManager::TurnOnBlkAllocation(void)
{
    for (auto i = 0; i < MAX_VOLUME_COUNT; i++)
    {
        blkAllocProhibited[i] = false;
    }
}
//----------------------------------------------------------------------------//
VirtualBlks
BlockManager::_AllocateBlks(ASTailArrayIdx asTailArrayIdx, int numBlks)
{
    assert(numBlks != 0);
    std::unique_lock<std::mutex> volLock(wbStripeCtx->GetActiveStripeTailLock(asTailArrayIdx));
    VirtualBlks allocatedBlks;
    VirtualBlkAddr curVsa = wbStripeCtx->GetActiveStripeTail(asTailArrayIdx);

    if (_IsStripeFull(curVsa) || IsUnMapStripe(curVsa.stripeId))
    {
        StripeId newVsid = UNMAP_STRIPE;
        int ret = _AllocateStripe(asTailArrayIdx, newVsid);
        if (likely(ret == 0))
        {
            allocatedBlks = _AllocateWriteBufferBlksFromNewStripe(asTailArrayIdx, newVsid, numBlks);
        }
        else
        {
            allocatedBlks.startVsa = UNMAP_VSA;
            allocatedBlks.numBlks = UINT32_MAX;
            return allocatedBlks;
        }
    }
    else if (_IsValidOffset(curVsa.offset + numBlks - 1) == false)
    {
        allocatedBlks.startVsa = curVsa;
        allocatedBlks.numBlks = addrInfo->GetblksPerStripe() - curVsa.offset;

        VirtualBlkAddr vsa = {.stripeId = curVsa.stripeId, .offset = addrInfo->GetblksPerStripe()};
        wbStripeCtx->SetActiveStripeTail(asTailArrayIdx, vsa);
    }
    else
    {
        allocatedBlks.startVsa = curVsa;
        allocatedBlks.numBlks = numBlks;

        VirtualBlkAddr vsa = {.stripeId = curVsa.stripeId, .offset = curVsa.offset + numBlks};
        wbStripeCtx->SetActiveStripeTail(asTailArrayIdx, vsa);
    }

    return allocatedBlks;
}

VirtualBlks
BlockManager::_AllocateWriteBufferBlksFromNewStripe(ASTailArrayIdx asTailArrayIdx, StripeId vsid, int numBlks)
{
    VirtualBlkAddr curVsa = {.stripeId = vsid, .offset = 0};

    VirtualBlks allocatedBlks;
    allocatedBlks.startVsa = curVsa;

    if (_IsValidOffset(numBlks))
    {
        allocatedBlks.numBlks = numBlks;
    }
    else
    {
        allocatedBlks.numBlks = addrInfo->GetblksPerStripe();
    }
    curVsa.offset = allocatedBlks.numBlks;

    // Temporally no lock required, as AllocateBlks and this function cannot be executed in parallel
    // TODO(jk.man.kim): add or move lock to wbuf tail manager
    wbStripeCtx->SetActiveStripeTail(asTailArrayIdx, curVsa);

    return allocatedBlks;
}

int
BlockManager::_AllocateStripe(ASTailArrayIdx asTailArrayIdx, StripeId& vsid)
{
    // 1. WriteBuffer Logical StripeId Allocation
    StripeId wbLsid = wbStripeCtx->AllocWbStripe();
    if (wbLsid == UNMAP_STRIPE)
    {
        return -EID(ALLOCATOR_CANNOT_ALLOCATE_STRIPE);
    }

#if defined QOS_ENABLED_BE
    QosManagerSingleton::Instance()->IncreaseUsedStripeCnt();
#endif

    // 2. SSD Logical StripeId Allocation
    bool isUserStripeAlloc = _IsUserStripeAllocation(asTailArrayIdx);
    StripeId arrayLsid = _AllocateUserDataStripeIdInternal(isUserStripeAlloc);
    if (IsUnMapStripe(arrayLsid))
    {
        std::lock_guard<std::mutex> lock(contextManager->GetCtxLock());
        _RollBackStripeIdAllocation(wbLsid, UINT32_MAX);
        return -EID(ALLOCATOR_CANNOT_ALLOCATE_STRIPE);
    }
    // If arrayLsid is the front and first stripe of Segment
    if (_IsSegmentFull(arrayLsid))
    {
        SegmentId segId = arrayLsid / addrInfo->GetstripesPerSegment();
        allocCtx->SetSegmentState(segId, SegmentState::NVRAM, false);
    }
    StripeId newVsid = arrayLsid;

    // 3. Get Stripe object for wbLsid and link it with reverse map for vsid
    Stripe* stripe = iWBStripeInternal->GetStripe(wbLsid);
    stripe->Assign(newVsid, wbLsid, asTailArrayIdx);

    // TODO (jk.man.kim): Don't forget to insert array name in the future.
    IReverseMap* iReverseMap = MapperServiceSingleton::Instance()->GetIReverseMap(arrayName);
    if (unlikely(iReverseMap->LinkReverseMap(stripe, wbLsid, newVsid) < 0))
    {
        std::lock_guard<std::mutex> lock(contextManager->GetCtxLock());
        _RollBackStripeIdAllocation(wbLsid, arrayLsid);
        return -EID(ALLOCATOR_CANNOT_LINK_REVERSE_MAP);
    }

    // 4. Update the stripe map
    // TODO (jk.man.kim): Don't forget to insert array name in the future.
    IStripeMap* iStripeMap = MapperServiceSingleton::Instance()->GetIStripeMap(arrayName);
    iStripeMap->SetLSA(newVsid, wbLsid, IN_WRITE_BUFFER_AREA);

    vsid = newVsid;
    return 0;
}

StripeId
BlockManager::_AllocateUserDataStripeIdInternal(bool isUserStripeAlloc)
{
    std::lock_guard<std::mutex> lock(contextManager->GetCtxLock());
    RebuildCtx* rbCtx = contextManager->GetRebuildCtx();

    StripeId ssdLsid = allocCtx->UpdatePrevLsid();

    if (_IsSegmentFull(ssdLsid))
    {
        if (contextManager->GetCurrentGcMode() == MODE_URGENT_GC)
        {
            userBlkAllocProhibited = true;
            if (isUserStripeAlloc)
            {
                return UNMAP_STRIPE;
            }
        }

        SegmentId segmentId = contextManager->AllocateFreeSegment(true);
        if (segmentId == UNMAP_SEGMENT)
        {
            // Under Rebuiling...
            if (rbCtx->IsRebuidTargetSegmentsEmpty() == false)
            {
                POS_TRACE_INFO(EID(ALLOCATOR_REBUILDING_SEGMENT), "Couldn't Allocate a SegmentId, seems Under Rebuiling");
                return UNMAP_STRIPE;
            }
            else
            {
                assert(false);
            }
        }
        ssdLsid = segmentId * addrInfo->GetstripesPerSegment();
    }

    allocCtx->SetCurrentSsdLsid(ssdLsid);

    return ssdLsid;
}

void
BlockManager::_RollBackStripeIdAllocation(StripeId wbLsid, StripeId arrayLsid)
{
    if (wbLsid != UINT32_MAX)
    {
        wbStripeCtx->ReleaseWbStripe(wbLsid);
    }

    if (arrayLsid != UINT32_MAX)
    {
        if (_IsSegmentFull(arrayLsid))
        {
            SegmentId SegmentIdToClear = arrayLsid / addrInfo->GetstripesPerSegment();
            contextManager->FreeUserDataSegment(SegmentIdToClear);
        }
        allocCtx->RollbackCurrentSsdLsid();
    }
}

} // namespace pos

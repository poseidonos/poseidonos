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

#include "src/allocator/block_manager/block_manager.h"

#include <string>

#include "src/allocator/context_manager/block_allocation_status.h"
#include "src/allocator/context_manager/allocator_ctx/allocator_ctx.h"
#include "src/allocator/stripe/stripe.h"
#include "src/include/branch_prediction.h"
#include "src/logger/logger.h"
#include "src/mapper_service/mapper_service.h"
#include "src/qos/qos_manager.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"
#include "src/allocator/context_manager/segment_ctx/segment_ctx.h"

namespace pos
{
BlockManager::BlockManager(TelemetryPublisher* tp_, IStripeMap* stripeMap_, IReverseMap* iReverseMap_, AllocatorCtx* allocCtx_, BlockAllocationStatus* allocStatus, AllocatorAddressInfo* info, ContextManager* ctxMgr, int arrayId, bool allocateBlocksFromSSDStripe)
: addrInfo(info),
  contextManager(ctxMgr),
  iWBStripeAllocator(nullptr),
  allocStatus(allocStatus),
  arrayId(arrayId),
  allocateBlocksFromSSDStripe(allocateBlocksFromSSDStripe),
  tp(tp_)
{
    allocCtx = allocCtx_;
    iReverseMap = iReverseMap_;
    iStripeMap = stripeMap_;
}

BlockManager::BlockManager(TelemetryPublisher* tp_, AllocatorAddressInfo* info, ContextManager* ctxMgr, int arrayId, bool allocateBlocksFromSSDStripe)
: BlockManager(tp_, nullptr, nullptr, nullptr, nullptr, info, ctxMgr, arrayId, allocateBlocksFromSSDStripe)
{
    allocCtx = contextManager->GetAllocatorCtx();
    allocStatus = contextManager->GetAllocationStatus();
}

void
BlockManager::Init(IWBStripeAllocator* iWBStripeAllocator_)
{
    iWBStripeAllocator = iWBStripeAllocator_;
    if (iReverseMap == nullptr)
    {
        iReverseMap = MapperServiceSingleton::Instance()->GetIReverseMap(arrayId);
    }
    if (iStripeMap == nullptr)
    {
        iStripeMap = MapperServiceSingleton::Instance()->GetIStripeMap(arrayId);
    }
}

std::pair<VirtualBlks, StripeId>
BlockManager::AllocateWriteBufferBlks(uint32_t volumeId, uint32_t numBlks)
{
    VirtualBlks allocatedBlks;

    if (allocStatus->IsUserBlockAllocationProhibited(volumeId) == true)
    {
        allocatedBlks.startVsa = UNMAP_VSA;
        allocatedBlks.numBlks = 0;
        return {allocatedBlks, UNMAP_STRIPE};
    }

    return _AllocateBlks(volumeId, numBlks);
}

Stripe*
BlockManager::AllocateGcDestStripe(uint32_t volumeId)
{
    if (allocStatus->IsBlockAllocationProhibited(volumeId))
    {
        return nullptr;
    }

    StripeId arrayLsid = _AllocateSsdStripe();
    if (IsUnMapStripe(arrayLsid))
    {
        POS_TRACE_ERROR(EID(ALLOCATOR_CANNOT_ALLOCATE_STRIPE), "failed to allocate gc stripe!");
        return nullptr;
    }

    StripeId newVsid = arrayLsid;
    Stripe* stripe = new Stripe(iReverseMap, false, addrInfo->GetblksPerStripe());
    stripe->Assign(newVsid, UINT32_MAX, 0);
    return stripe;
}

void
BlockManager::ProhibitUserBlkAlloc(void)
{
    POSMetricValue v;
    v.gauge = 1;
    tp->PublishData(TEL30011_ALCT_PROHIBIT_USERBLK_ALLOCATION_ONOFF, v, MT_GAUGE);
    allocStatus->ProhibitUserBlockAllocation();
}

void
BlockManager::PermitUserBlkAlloc(void)
{
    POSMetricValue v;
    v.gauge = 0;
    tp->PublishData(TEL30011_ALCT_PROHIBIT_USERBLK_ALLOCATION_ONOFF, v, MT_GAUGE);
    allocStatus->PermitUserBlockAllocation();
}

bool
BlockManager::BlockAllocating(uint32_t volumeId)
{
    return allocStatus->TryProhibitBlockAllocation(volumeId);
}

void
BlockManager::UnblockAllocating(uint32_t volumeId)
{
    allocStatus->PermitBlockAllocation(volumeId);
}

void
BlockManager::TurnOffBlkAllocation(void)
{
    allocStatus->ProhibitBlockAllocation();
}

void
BlockManager::TurnOnBlkAllocation(void)
{
    allocStatus->PermitBlockAllocation();
}
//----------------------------------------------------------------------------//
std::pair<VirtualBlks, StripeId>
BlockManager::_AllocateBlks(ASTailArrayIdx asTailArrayIdx, int numBlks)
{
    assert(numBlks != 0);
    std::unique_lock<std::mutex> volLock(allocCtx->GetActiveStripeTailLock(asTailArrayIdx));
    StripeId allocatedWbLsid = UNMAP_STRIPE;

    VirtualBlkAddr curVsa = allocCtx->GetActiveStripeTail(asTailArrayIdx);
    if (_IsStripeFull(curVsa) || IsUnMapStripe(curVsa.stripeId))
    {
        auto allocatedStripes = _AllocateStripesAndUpdateActiveStripeTail(asTailArrayIdx);
        if (allocatedStripes.first == UNMAP_STRIPE || allocatedStripes.second == UNMAP_STRIPE)
        {
            return {{UNMAP_VSA, UINT32_MAX}, UNMAP_STRIPE};
        }

        allocatedWbLsid = allocatedStripes.first;
    }
    else
    {
        allocatedWbLsid = allocCtx->GetActiveWbStripeId(asTailArrayIdx);
    }

    VirtualBlks allocatedBlks = _AllocateBlocksFromActiveStripe(asTailArrayIdx, numBlks);
    return {allocatedBlks, allocatedWbLsid};
}

std::pair<StripeId, StripeId>
BlockManager::_AllocateStripesAndUpdateActiveStripeTail(ASTailArrayIdx asTailArrayIdx)
{
    StripeId wbLsid = _AllocateWbStripe();
    if (unlikely(wbLsid == UNMAP_STRIPE))
    {
        return {UNMAP_STRIPE, UNMAP_STRIPE};
    }

    StripeId userLsid = _AllocateSsdStripeForUser(asTailArrayIdx);
    if (unlikely(userLsid == UNMAP_STRIPE))
    {
        _RollBackStripeIdAllocation(wbLsid);
        return {UNMAP_STRIPE, UNMAP_STRIPE};
    }

    QosManagerSingleton::Instance()->IncreaseUsedStripeCnt(arrayId);

    StripeId newVsid = userLsid;
    _AssignStripe(newVsid, wbLsid, asTailArrayIdx);

    if (allocateBlocksFromSSDStripe == true)
    {
        // New blocks will be allocted from the ssd stripe in write-through mode
        // WB stripe is allocated above, but will not be updated to the stripe map (temporally)
        iStripeMap->SetLSA(newVsid, userLsid, IN_USER_AREA);
    }
    else
    {
        iStripeMap->SetLSA(newVsid, wbLsid, IN_WRITE_BUFFER_AREA);
    }

    // Temporally no lock required, as AllocateBlks and this function cannot be executed in parallel
    // TODO(jk.man.kim): add or move lock to wbuf tail manager
    VirtualBlkAddr curVsa = {
        .stripeId = newVsid,
        .offset = 0};
    allocCtx->SetNewActiveStripeTail(asTailArrayIdx, curVsa, wbLsid);

    return {wbLsid, userLsid};
}

StripeId
BlockManager::_AllocateWbStripe(void)
{
    return allocCtx->AllocFreeWbStripe();
}

void
BlockManager::_AssignStripe(StripeId vsid, StripeId wbLsid, ASTailArrayIdx asTailArrayIdx)
{
    Stripe* stripe = iWBStripeAllocator->GetStripe(wbLsid);
    stripe->Assign(vsid, wbLsid, asTailArrayIdx);
}

StripeId
BlockManager::_AllocateSsdStripeForUser(int volumeId)
{
    std::lock_guard<std::mutex> lock(allocCtx->GetCtxLock());
    StripeId ssdLsid = allocCtx->GetCurrentSsdLsid() + 1;

    if (true == _IsSegmentFull(ssdLsid))
    {
        if (allocStatus->IsUserBlockAllocationProhibited(volumeId) == true)
        {
            ssdLsid = UNMAP_STRIPE;
        }
        else
        {
            ssdLsid = _AllocateSegmentAndStripe();
        }
    }
    allocCtx->SetCurrentSsdLsid(ssdLsid);
    return ssdLsid;
}

StripeId
BlockManager::_AllocateSsdStripe(void)
{
    std::lock_guard<std::mutex> lock(allocCtx->GetCtxLock());
    StripeId ssdLsid = allocCtx->GetCurrentSsdLsid() + 1;

    if (true == _IsSegmentFull(ssdLsid))
    {
        ssdLsid = _AllocateSegmentAndStripe();
    }
    allocCtx->SetCurrentSsdLsid(ssdLsid);
    return ssdLsid;
}

StripeId
BlockManager::_AllocateSegmentAndStripe(void)
{
    SegmentId segmentId = contextManager->AllocateFreeSegment();
    if (segmentId == UNMAP_SEGMENT)
    {
        return UNMAP_STRIPE;
    }

    StripeId newStripe = segmentId * addrInfo->GetstripesPerSegment();

    return newStripe;
}

void
BlockManager::_RollBackStripeIdAllocation(StripeId wbLsid)
{
    if (wbLsid != UINT32_MAX)
    {
        allocCtx->ReleaseWbStripe(wbLsid);
    }
}

VirtualBlks
BlockManager::_AllocateBlocksFromActiveStripe(ASTailArrayIdx asTailArrayIdx, int numBlks)
{
    VirtualBlkAddr curVsa = allocCtx->GetActiveStripeTail(asTailArrayIdx);
    VirtualBlkAddr updatedTail = curVsa;
    VirtualBlks allocatedBlks;
    allocatedBlks.startVsa = curVsa;

    if (_IsValidOffset(curVsa.offset + numBlks - 1) == false)
    {
        allocatedBlks.numBlks = addrInfo->GetblksPerStripe() - curVsa.offset;
        updatedTail.offset = addrInfo->GetblksPerStripe();
    }
    else
    {
        allocatedBlks.numBlks = numBlks;
        updatedTail.offset = curVsa.offset + numBlks;
    }
    allocCtx->SetActiveStripeTail(asTailArrayIdx, updatedTail);

    return allocatedBlks;
}

} // namespace pos

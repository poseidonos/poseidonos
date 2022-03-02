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

namespace pos
{
BlockManager::BlockManager(TelemetryPublisher* tp_, IStripeMap* stripeMap_, IReverseMap* iReverseMap_, AllocatorCtx* allocCtx_, BlockAllocationStatus* allocStatus, AllocatorAddressInfo* info, ContextManager* ctxMgr, int arrayId)
: addrInfo(info),
  contextManager(ctxMgr),
  iWBStripeInternal(nullptr),
  allocStatus(allocStatus),
  arrayId(arrayId),
  tp(tp_)
{
    allocCtx = allocCtx_;
    iReverseMap = iReverseMap_;
    iStripeMap = stripeMap_;
}

BlockManager::BlockManager(TelemetryPublisher* tp_, AllocatorAddressInfo* info, ContextManager* ctxMgr, int arrayId)
: BlockManager(tp_, nullptr, nullptr, nullptr, nullptr, info, ctxMgr, arrayId)
{
    allocCtx = contextManager->GetAllocatorCtx();
    allocStatus = contextManager->GetAllocationStatus();
}

void
BlockManager::Init(IWBStripeInternal* iwbstripeInternal)
{
    iWBStripeInternal = iwbstripeInternal;
    if (iReverseMap == nullptr)
    {
        iReverseMap = MapperServiceSingleton::Instance()->GetIReverseMap(arrayId);
    }
    if (iStripeMap == nullptr)
    {
        iStripeMap = MapperServiceSingleton::Instance()->GetIStripeMap(arrayId);
    }
}

VirtualBlks
BlockManager::AllocateWriteBufferBlks(uint32_t volumeId, uint32_t numBlks)
{
    VirtualBlks allocatedBlks;

    if (allocStatus->IsUserBlockAllocationProhibited(volumeId) == true)
    {
        allocatedBlks.startVsa = UNMAP_VSA;
        allocatedBlks.numBlks = 0;
        return allocatedBlks;
    }

    allocatedBlks = _AllocateBlks(volumeId, numBlks);
    return allocatedBlks;
}

Stripe*
BlockManager::AllocateGcDestStripe(uint32_t volumeId)
{
    if (allocStatus->IsBlockAllocationProhibited(volumeId))
    {
        return nullptr;
    }

    QosManagerSingleton::Instance()->IncreaseUsedStripeCnt(arrayId);

    // 2. SSD Logical StripeId/vsid Allocation
    StripeId arrayLsid = _AllocateUserDataStripeIdInternal(false /*isUser*/);
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
VirtualBlks
BlockManager::_AllocateBlks(ASTailArrayIdx asTailArrayIdx, int numBlks)
{
    assert(numBlks != 0);
    std::unique_lock<std::mutex> volLock(allocCtx->GetActiveStripeTailLock(asTailArrayIdx));
    VirtualBlks allocatedBlks;
    VirtualBlkAddr curVsa = allocCtx->GetActiveStripeTail(asTailArrayIdx);

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
        allocCtx->SetActiveStripeTail(asTailArrayIdx, vsa);
    }
    else
    {
        allocatedBlks.startVsa = curVsa;
        allocatedBlks.numBlks = numBlks;

        VirtualBlkAddr vsa = {.stripeId = curVsa.stripeId, .offset = curVsa.offset + numBlks};
        allocCtx->SetActiveStripeTail(asTailArrayIdx, vsa);
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
    allocCtx->SetActiveStripeTail(asTailArrayIdx, curVsa);

    return allocatedBlks;
}

int
BlockManager::_AllocateStripe(ASTailArrayIdx asTailArrayIdx, StripeId& vsid)
{
    // 1. WriteBuffer Logical StripeId Allocation
    StripeId wbLsid = allocCtx->AllocFreeWbStripe();
    if (wbLsid == UNMAP_STRIPE)
    {
        return -EID(ALLOCATOR_CANNOT_ALLOCATE_STRIPE);
    }

    QosManagerSingleton::Instance()->IncreaseUsedStripeCnt(arrayId);

    // 2. SSD Logical StripeId Allocation
    bool isUserStripeAlloc = _IsUserStripeAllocation(asTailArrayIdx);
    StripeId arrayLsid = _AllocateUserDataStripeIdInternal(isUserStripeAlloc);
    if (IsUnMapStripe(arrayLsid))
    {
        std::lock_guard<std::mutex> lock(contextManager->GetCtxLock());
        _RollBackStripeIdAllocation(wbLsid);
        return -EID(ALLOCATOR_CANNOT_ALLOCATE_STRIPE);
    }

    StripeId newVsid = arrayLsid;

    // 3. Get Stripe object for wbLsid and link it with reverse map for vsid
    Stripe* stripe = iWBStripeInternal->GetStripe(wbLsid);
    stripe->Assign(newVsid, wbLsid, asTailArrayIdx);

    // 4. Update the stripe map
    iStripeMap->SetLSA(newVsid, wbLsid, IN_WRITE_BUFFER_AREA);

    vsid = newVsid;
    return 0;
}

StripeId
BlockManager::_AllocateUserDataStripeIdInternal(bool isUserStripeAlloc)
{
    std::lock_guard<std::mutex> lock(contextManager->GetCtxLock());
    StripeId ssdLsid = allocCtx->UpdatePrevLsid();

    if (_IsSegmentFull(ssdLsid))
    {
        if (contextManager->GetCurrentGcMode() == MODE_URGENT_GC)
        {
            allocStatus->ProhibitUserBlockAllocation();
            if (isUserStripeAlloc)
            {
                return UNMAP_STRIPE;
            }
        }

        SegmentId segmentId = contextManager->AllocateFreeSegment();
        if (segmentId == UNMAP_SEGMENT)
        {
            while (addrInfo->IsUT() != true)
            {
                usleep(1); // assert(false);
            }
            return UNMAP_STRIPE;
        }
        ssdLsid = segmentId * addrInfo->GetstripesPerSegment();
    }

    allocCtx->SetCurrentSsdLsid(ssdLsid);

    return ssdLsid;
}

void
BlockManager::_RollBackStripeIdAllocation(StripeId wbLsid)
{
    if (wbLsid != UINT32_MAX)
    {
        allocCtx->ReleaseWbStripe(wbLsid);
    }
}

} // namespace pos

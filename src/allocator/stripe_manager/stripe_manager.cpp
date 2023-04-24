/*
 *   BSD LICENSE
 *   Copyright (c) 2022 Samsung Electronics Corporation
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

#include "src/allocator/stripe_manager/stripe_manager.h"

#include "src/allocator/address/allocator_address_info.h"
#include "src/allocator/context_manager/context_manager.h"
#include "src/allocator/i_wbstripe_allocator.h"
#include "src/allocator/stripe_manager/stripe.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/mapper_service/mapper_service.h"
#include "src/qos/qos_manager.h"

namespace pos
{
StripeManager::StripeManager(ContextManager* ctxMgr, AllocatorAddressInfo* addrInfo_, int arrayId)
: StripeManager(ctxMgr, nullptr, nullptr, addrInfo_, arrayId)
{
}

StripeManager::StripeManager(ContextManager* ctxMgr, IReverseMap* iReverseMap_, IStripeMap* stripeMap_, AllocatorAddressInfo* addrInfo_, int arrayId_)
: wbStripeManager(nullptr),
  contextManager(ctxMgr),
  allocCtx(ctxMgr->GetAllocatorCtx()),
  allocStatus(ctxMgr->GetAllocationStatus()),
  addrInfo(addrInfo_),
  arrayId(arrayId_),
  reverseMap(iReverseMap_),
  stripeMap(stripeMap_)
{
}

void
StripeManager::Init(IWBStripeAllocator* wbStripeMgr_)
{
    wbStripeManager = wbStripeMgr_;

    if (reverseMap == nullptr)
    {
        reverseMap = MapperServiceSingleton::Instance()->GetIReverseMap(arrayId);
    }
    if (stripeMap == nullptr)
    {
        stripeMap = MapperServiceSingleton::Instance()->GetIStripeMap(arrayId);
    }
}

bool
StripeManager::_IsLastStripesWithinSegment(StripeId stripeId)
{
    return stripeId % addrInfo->GetstripesPerSegment() == 0;
}

StripeSmartPtr
StripeManager::AllocateGcDestStripe(uint32_t volumeId)
{
    if (allocStatus->IsBlockAllocationProhibited(volumeId))
    {
        return nullptr;
    }

    StripeId arrayLsid = _AllocateSsdStripe();
    if (IsUnMapStripe(arrayLsid))
    {
       // POS_TRACE_ERROR(EID(ALLOCATOR_CANNOT_ALLOCATE_STRIPE), "failed to allocate gc stripe!");
        return nullptr;
    }

    StripeId newVsid = arrayLsid;
    StripeSmartPtr stripe = _AllocateStripe(newVsid, UNMAP_STRIPE, volumeId);

    return stripe;
}

StripeId
StripeManager::_AllocateSsdStripe(void)
{
    std::lock_guard<std::mutex> lock(allocCtx->GetCtxLock());
    StripeId ssdLsid = allocCtx->GetCurrentSsdLsid() + 1;

    if (true == _IsLastStripesWithinSegment(ssdLsid))
    {
        ssdLsid = _AllocateSegmentAndStripe();
    }
    allocCtx->SetCurrentSsdLsid(ssdLsid);
    return ssdLsid;
}

StripeId
StripeManager::_AllocateSegmentAndStripe(void)
{
    SegmentId segmentId = contextManager->AllocateFreeSegment();
    if (segmentId == UNMAP_SEGMENT)
    {
        return UNMAP_STRIPE;
    }

    StripeId newStripe = segmentId * addrInfo->GetstripesPerSegment();

    return newStripe;
}

StripeSmartPtr
StripeManager::_AllocateStripe(StripeId vsid, StripeId wbLsid, uint32_t volumeId)
{
    StripeSmartPtr stripe = StripeSmartPtr(new Stripe(reverseMap, addrInfo->GetblksPerStripe()));
    StripeId userLsid = VsidToUserLsid(vsid);
    stripe->Assign(vsid, wbLsid, userLsid, volumeId);

    return stripe;
}

StripeId
StripeManager::_AllocateUserSsdStripe(int volumeId)
{
    std::lock_guard<std::mutex> lock(allocCtx->GetCtxLock());
    StripeId ssdLsid = allocCtx->GetCurrentSsdLsid() + 1;

    if (true == _IsLastStripesWithinSegment(ssdLsid))
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

std::pair<StripeId, StripeId>
StripeManager::AllocateStripesForUser(uint32_t volumeId)
{
    StripeId wbLsid = _AllocateWbStripe();
    if (unlikely(wbLsid == UNMAP_STRIPE))
    {
        return {UNMAP_STRIPE, UNMAP_STRIPE};
    }

    StripeId userLsid = _AllocateUserSsdStripe(volumeId);
    if (unlikely(userLsid == UNMAP_STRIPE))
    {
        _RollBackWbStripeIdAllocation(wbLsid);
        return {UNMAP_STRIPE, UNMAP_STRIPE};
    }

    QosManagerSingleton::Instance()->IncreaseUsedStripeCnt(arrayId);

    StripeId newVsid = userLsid;
    StripeSmartPtr stripe = _AllocateStripe(newVsid, wbLsid, volumeId);
    wbStripeManager->AssignStripe(stripe);

    stripeMap->SetLSA(newVsid, wbLsid, IN_WRITE_BUFFER_AREA);

    // Temporally no lock required, as AllocateBlks and this function cannot be executed in parallel
    // TODO(jk.man.kim): add or move lock to wbuf tail manager
    VirtualBlkAddr curVsa = {
        .stripeId = newVsid,
        .offset = 0};
    allocCtx->SetActiveStripeTail(volumeId, curVsa);

    return {wbLsid, userLsid};
}

StripeId
StripeManager::_AllocateWbStripe(void)
{
    return allocCtx->AllocFreeWbStripe();
}

void
StripeManager::_RollBackWbStripeIdAllocation(StripeId wbLsid)
{
    if (wbLsid != UINT32_MAX)
    {
        allocCtx->ReleaseWbStripe(wbLsid);
    }
}

} // namespace pos

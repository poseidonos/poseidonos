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

#include "src/allocator/context_manager/allocator_ctx/allocator_ctx.h"
#include "src/allocator/context_manager/block_allocation_status.h"
#include "src/allocator/stripe_manager/stripe_manager.h"
#include "src/logger/logger.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"

namespace pos
{
BlockManager::BlockManager(TelemetryPublisher* tp_, AllocatorCtx* allocCtx_, BlockAllocationStatus* allocStatus, AllocatorAddressInfo* info, ContextManager* ctxMgr, int arrayId)
: addrInfo(info),
  contextManager(ctxMgr),
  stripeManager(nullptr),
  allocStatus(allocStatus),
  arrayId(arrayId),
  tp(tp_)
{
    allocCtx = allocCtx_;
}

BlockManager::BlockManager(TelemetryPublisher* tp_, AllocatorAddressInfo* info, ContextManager* ctxMgr, int arrayId)
: BlockManager(tp_, nullptr, nullptr, info, ctxMgr, arrayId)
{
    allocCtx = contextManager->GetAllocatorCtx();
    allocStatus = contextManager->GetAllocationStatus();
}

void
BlockManager::Init(StripeManager* stripeManager_)
{
    stripeManager = stripeManager_;
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

StripeSmartPtr
BlockManager::AllocateGcDestStripe(uint32_t volumeId)
{
    return stripeManager->AllocateGcDestStripe(volumeId);
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
    StripeId allocatedUserStripe = UNMAP_STRIPE;

    VirtualBlkAddr curVsa = allocCtx->GetActiveStripeTail(asTailArrayIdx);
    if (_IsStripeFull(curVsa) || IsUnMapStripe(curVsa.stripeId))
    {
        auto allocatedStripes = stripeManager->AllocateStripesForUser(asTailArrayIdx);
        if (allocatedStripes.first == UNMAP_STRIPE || allocatedStripes.second == UNMAP_STRIPE)
        {
            return {{UNMAP_VSA, UINT32_MAX}, UNMAP_STRIPE};
        }

        allocatedUserStripe = allocatedStripes.second;
    }
    else
    {
        allocatedUserStripe = VsidToUserLsid(curVsa.stripeId);
    }

    VirtualBlks allocatedBlks = _AllocateBlocksFromActiveStripe(asTailArrayIdx, numBlks);
    return {allocatedBlks, allocatedUserStripe};
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

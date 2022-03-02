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

#include "gc_map_update.h"

#include "src/allocator/i_block_allocator.h"
#include "src/allocator/i_context_manager.h"
#include "src/allocator/stripe/stripe.h"
#include "src/logger/logger.h"
#include "src/mapper/i_stripemap.h"
#include "src/spdk_wrapper/event_framework_api.h"

namespace pos
{
GcMapUpdate::GcMapUpdate(IVSAMap* vsaMap, IStripeMap* stripeMap,
    ISegmentCtx* segmentCtx, IContextManager* contextManager,
    IArrayInfo* arrayInfo, Stripe* stripe, GcStripeMapUpdateList mapUpdateInfoList,
    std::map<SegmentId, uint32_t> invalidSegCnt)
: Callback(EventFrameworkApiSingleton::Instance()->IsReactorNow()),
  vsaMap(vsaMap),
  stripeMap(stripeMap),
  segmentCtx(segmentCtx),
  contextManager(contextManager),
  arrayInfo(arrayInfo),
  stripe(stripe),
  mapUpdateInfoList(mapUpdateInfoList),
  invalidSegCnt(invalidSegCnt)
{
    stripesPerSegment = arrayInfo->GetSizeInfo(PartitionType::USER_DATA)->stripesPerSegment;
}

GcMapUpdate::~GcMapUpdate(void)
{
}

bool
GcMapUpdate::_DoSpecificJob(void)
{
    StripeId stripeId = stripe->GetVsid();
    BlkAddr rba;
    uint32_t volId = mapUpdateInfoList.volumeId;
    VirtualBlkAddr writeVsa;

    StripeId currentLsid = stripe->GetUserLsid();
    stripeMap->SetLSA(stripe->GetVsid(), stripe->GetUserLsid(), IN_USER_AREA);
    contextManager->UpdateOccupiedStripeCount(currentLsid);

    uint32_t validCount = mapUpdateInfoList.blockMapUpdateList.size();
    for (auto it : mapUpdateInfoList.blockMapUpdateList)
    {
        rba = it.rba;
        writeVsa = it.vsa;
        VirtualBlks writeVsaRange = {writeVsa, 1};
        vsaMap->SetVSAsInternal(volId, rba, writeVsaRange);
    }
    _InvalidateBlock();
    _ValidateBlock(stripeId, validCount);

    POS_TRACE_DEBUG((int)POS_EVENT_ID::GC_MAP_UPDATE_COMPLETION,
        "gc map update, arrayName:{}, stripeUserLsid:{}",
        arrayInfo->GetName(), currentLsid);

    return true;
}

void
GcMapUpdate::_InvalidateBlock(void)
{
    for (auto it : invalidSegCnt)
    {
        SegmentId segId = it.first;
        uint32_t invalidCnt = it.second;

        VirtualBlkAddr invalidVsa;
        invalidVsa.stripeId = segId * stripesPerSegment;
        invalidVsa.offset = 0;
        VirtualBlks invalidVsaRange = {invalidVsa, invalidCnt};

        segmentCtx->InvalidateBlks(invalidVsaRange);
    }
}

void
GcMapUpdate::_ValidateBlock(StripeId stripeId, uint32_t cnt)
{
    VirtualBlkAddr writeVsa = {stripeId, 0};
    VirtualBlks writeVsaRange = {writeVsa, cnt};

    segmentCtx->ValidateBlks(writeVsaRange);
}

} // namespace pos

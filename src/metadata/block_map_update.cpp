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

#include "src/metadata/block_map_update.h"

#include "src/bio/volume_io.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/io/general_io/vsa_range_maker.h"
#include "src/logger/logger.h"
#include "src/spdk_wrapper/event_framework_api.h"

namespace pos
{
BlockMapUpdate::BlockMapUpdate(VolumeIoSmartPtr volumeIo, IVSAMap* vsaMap,
    ISegmentCtx* segmentCtx, IWBStripeAllocator* wbStripeAllocator)
: BlockMapUpdate(volumeIo, vsaMap, segmentCtx, wbStripeAllocator,
      new VsaRangeMaker(volumeIo->GetVolumeId(),
          ChangeSectorToBlock(volumeIo->GetSectorRba()),
          DivideUp(volumeIo->GetSize(), BLOCK_SIZE), volumeIo->GetArrayId()))
{
}

BlockMapUpdate::BlockMapUpdate(VolumeIoSmartPtr volumeIo, IVSAMap* vsaMap,
    ISegmentCtx* segmentCtx, IWBStripeAllocator* wbStripeAllocator,
    VsaRangeMaker* vsaRangeMaker)
: Callback(EventFrameworkApiSingleton::Instance()->IsReactorNow()),
  volumeIo(volumeIo),
  vsaMap(vsaMap),
  segmentCtx(segmentCtx),
  wbStripeAllocator(wbStripeAllocator),
  oldVsaRangeMaker(vsaRangeMaker)
{
}

BlockMapUpdate::~BlockMapUpdate(void)
{
    if (oldVsaRangeMaker != nullptr)
    {
        delete oldVsaRangeMaker;
        oldVsaRangeMaker = nullptr;
    }
}

bool
BlockMapUpdate::_DoSpecificJob(void)
{
    uint32_t blockCount = DivideUp(volumeIo->GetSize(), BLOCK_SIZE);
    VirtualBlks targetVsaRange = {
        .startVsa = volumeIo->GetVsa(),
        .numBlks = blockCount};
    BlkAddr rba = ChangeSectorToBlock(volumeIo->GetSectorRba());
    vsaMap->SetVSAs(volumeIo->GetVolumeId(), rba, targetVsaRange);

    StripeId wbLsid = volumeIo->GetWbLsid();
    Stripe& stripe = _GetStripe(wbLsid);
    _UpdateReverseMap(stripe);

    uint32_t vsaRangeCount = oldVsaRangeMaker->GetCount();
    for (uint32_t vsaRangeIndex = 0; vsaRangeIndex < vsaRangeCount;
         vsaRangeIndex++)
    {
        VirtualBlks& vsaRange = oldVsaRangeMaker->GetVsaRange(vsaRangeIndex);
        segmentCtx->InvalidateBlks(vsaRange);

        POS_TRACE_DEBUG_IN_MEMORY(ModuleInDebugLogDump::META, POS_EVENT_ID::MAPPER_SUCCESS,
            "Invalidate rba {} vsid {}", ChangeSectorToBlock(volumeIo->GetSectorRba()), vsaRange.startVsa.stripeId);
    }

    segmentCtx->ValidateBlks(targetVsaRange);

    return true;
}

void
BlockMapUpdate::_UpdateReverseMap(Stripe& stripe)
{
    VirtualBlkAddr startVsa = volumeIo->GetVsa();
    uint32_t blocks = DivideUp(volumeIo->GetSize(), BLOCK_SIZE);
    uint64_t startRba = ChangeSectorToBlock(volumeIo->GetSectorRba());
    uint32_t startVsOffset = startVsa.offset;
    uint32_t volumeId = volumeIo->GetVolumeId();

    for (uint32_t blockIndex = 0; blockIndex < blocks; blockIndex++)
    {
        uint64_t vsOffset = startVsOffset + blockIndex;
        BlkAddr targetRba = startRba + blockIndex;
        stripe.UpdateReverseMapEntry(vsOffset, targetRba, volumeId);
    }
}

Stripe&
BlockMapUpdate::_GetStripe(StripeId& wbLsid)
{
    Stripe* foundStripe = wbStripeAllocator->GetStripe(wbLsid);

    if (unlikely(nullptr == foundStripe))
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::WRCMP_INVALID_STRIPE;
        POS_TRACE_ERROR(static_cast<int>(eventId),
            "Stripe is null at WriteCompleting state");
        throw eventId;
    }

    return *foundStripe;
}

} // namespace pos

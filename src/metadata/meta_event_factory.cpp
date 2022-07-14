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

#include "src/metadata/meta_event_factory.h"

#include "src/allocator/i_block_allocator.h"
#include "src/allocator/i_context_manager.h"
#include "src/allocator/i_wbstripe_allocator.h"
#include "src/mapper/i_stripemap.h"
#include "src/mapper/i_vsamap.h"
#include "src/metadata/block_map_update.h"
#include "src/metadata/gc_map_update.h"
#include "src/metadata/stripe_map_update.h"

namespace pos
{
MetaEventFactory::MetaEventFactory(IVSAMap* vsaMap, IStripeMap* stripeMap,
    ISegmentCtx* segmentCtx_, IWBStripeAllocator* wbStripeAllocator,
    IContextManager* contextManager, IArrayInfo* arrayInfo)
: vsaMap(vsaMap),
  stripeMap(stripeMap),
  segmentCtx(segmentCtx_),
  wbStripeAllocator(wbStripeAllocator),
  contextManager(contextManager),
  arrayInfo(arrayInfo)
{
}

CallbackSmartPtr
MetaEventFactory::CreateBlockMapUpdateEvent(VolumeIoSmartPtr volumeIo)
{
    CallbackSmartPtr callback(new BlockMapUpdate(volumeIo, vsaMap, segmentCtx, wbStripeAllocator));
    return callback;
}

CallbackSmartPtr
MetaEventFactory::CreateStripeMapUpdateEvent(Stripe* stripe)
{
    CallbackSmartPtr callback(new StripeMapUpdate(stripe, stripeMap, segmentCtx));
    return callback;
}

CallbackSmartPtr
MetaEventFactory::CreateGcMapUpdateEvent(StripeSmartPtr stripe, GcStripeMapUpdateList mapUpdateInfoList, std::map<SegmentId, uint32_t> invalidSegCnt)
{
    CallbackSmartPtr callback(new GcMapUpdate(vsaMap, stripeMap, segmentCtx, contextManager, arrayInfo, stripe, mapUpdateInfoList, invalidSegCnt));
    return callback;
}

} // namespace pos

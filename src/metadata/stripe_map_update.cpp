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

#include "src/metadata/stripe_map_update.h"

#include "src/allocator/stripe/stripe.h"
#include "src/include/address_type.h"
#include "src/spdk_wrapper/event_framework_api.h"

namespace pos
{
StripeMapUpdate::StripeMapUpdate(Stripe* stripe, IStripeMap* stripeMap,
    ISegmentCtx* segmentCtx_)
: MetaUpdateCallback(EventFrameworkApiSingleton::Instance()->IsReactorNow(), segmentCtx_),
  stripe(stripe),
  stripeMap(stripeMap)
{
}

StripeMapUpdate::~StripeMapUpdate(void)
{
}

bool
StripeMapUpdate::_DoSpecificJob(void)
{
    StripeId currentLsid = stripe->GetUserLsid();
    stripeMap->SetLSA(stripe->GetVsid(), currentLsid, IN_USER_AREA);
    UpdateOccupiedStripeCount(currentLsid);

    return true;
}
void
StripeMapUpdate::ValidateBlks(VirtualBlks blks)
{
    MetaUpdateCallback::ValidateBlks(blks);
}

bool
StripeMapUpdate::InvalidateBlks(VirtualBlks blks, bool isForced)
{
    return MetaUpdateCallback::InvalidateBlks(blks, isForced);
}

bool
StripeMapUpdate::UpdateOccupiedStripeCount(StripeId lsid)
{
    return MetaUpdateCallback::UpdateOccupiedStripeCount(lsid);
}
} // namespace pos

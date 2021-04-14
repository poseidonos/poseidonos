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

#include "stripe.h"

namespace pos
{
Stripe::Stripe(void)
: asTailArrayIdx(UINT32_MAX),
  vsid(UINT32_MAX),
  wbLsid(UINT32_MAX),
  userLsid(UINT32_MAX),
  revMapPack(nullptr),
  finished(true),
  remaining(0),
  referenceCount(0)
{
}

void
Stripe::UpdateReverseMap(uint32_t offset, BlkAddr rba, uint32_t volumeId)
{
}

void
Stripe::Assign(StripeId inputVsid, StripeId inputLsid, ASTailArrayIdx tailarrayidx)
{
    vsid = inputVsid;
    wbLsid = inputLsid;
    asTailArrayIdx = tailarrayidx;
}

bool
Stripe::IsOkToFree(void)
{
    bool isOkToFree = (0 == referenceCount);
    return isOkToFree;
}

void
Stripe::Derefer(uint32_t blockCount)
{
    referenceCount -= blockCount;
}

void
Stripe::Refer(void)
{
    referenceCount++;
}

StripeId
Stripe::GetWbLsid(void)
{
    return wbLsid;
}

void
Stripe::SetWbLsid(StripeId wbAreaLsid)
{
    wbLsid = wbAreaLsid;
}

StripeId
Stripe::GetUserLsid(void)
{
    return userLsid;
}

void
Stripe::SetUserLsid(StripeId userAreaLsid)
{
    userLsid = userAreaLsid;
}

StripeId
Stripe::GetVsid(void)
{
    return vsid;
}

void
Stripe::SetVsid(StripeId virtsid)
{
    vsid = virtsid;
}

uint32_t
Stripe::GetAsTailArrayIdx(void)
{
    return asTailArrayIdx;
}

int
Stripe::LinkReverseMap(ReverseMapPack* revMapPackToLink)
{
    return 0;
}

int
Stripe::Flush(EventSmartPtr callback)
{
    return 0;
}

int
Stripe::ReconstructReverseMap(uint32_t volumeId, uint64_t blockCount)
{
    return 0;
}

int
Stripe::UnLinkReverseMap(void)
{
    return 0;
}

bool
Stripe::IsFinished(void)
{
    return true;
}

void
Stripe::SetFinished(bool state)
{
}

uint32_t
Stripe::GetBlksRemaining(void)
{
    return 0;
}

uint32_t
Stripe::DecreseBlksRemaining(uint32_t amount)
{
    return 0;
}

void
Stripe::AddDataBuffer(void* buf)
{
}

DataBufferIter
Stripe::DataBufferBegin(void)
{
    return dataBuffer.begin();
}

DataBufferIter
Stripe::DataBufferEnd(void)
{
    return dataBuffer.end();
}

std::tuple<BlkAddr, uint32_t>
Stripe::GetReverseMapEntry(uint32_t offset)
{
    return std::make_tuple(0, 0);
}

void
Stripe::UpdateVictimVsa(uint32_t offset, VirtualBlkAddr vsa)
{
}

VirtualBlkAddr
Stripe::GetVictimVsa(uint32_t offset)
{
    VirtualBlkAddr tail = {
    .stripeId = UNMAP_STRIPE,
    .offset = 0};
    return tail;
}
} // namespace pos

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

#include "src/allocator/context_manager/wbstripe_ctx/wbstripe_ctx.h"

#include <string>
#include <vector>

#include "src/allocator/address/allocator_address_info.h"
#include "src/meta_file_intf/meta_file_include.h"
#include "src/metafs/metafs_file_intf.h"

namespace pos
{
WbStripeCtx::WbStripeCtx(void)
: allocWbLsidBitmap(nullptr),
  initialized(false),
  addrInfo(nullptr)
{
    for (int i = 0; i < ACTIVE_STRIPE_TAIL_ARRAYLEN; ++i)
    {
        activeStripeTail[i].stripeId = 0;
        activeStripeTail[i].offset = 0;
    }
}

WbStripeCtx::WbStripeCtx(BitMapMutex* allocWbLsidBitmap_, AllocatorAddressInfo* info_)
: initialized(false),
  addrInfo(info_)
{
    // for UT
    allocWbLsidBitmap = allocWbLsidBitmap_;
}

WbStripeCtx::WbStripeCtx(AllocatorAddressInfo* info)
: WbStripeCtx(nullptr, info)
{
}

WbStripeCtx::~WbStripeCtx(void)
{
    Dispose();
}

void
WbStripeCtx::Init(void)
{
    if (initialized == false)
    {
        allocWbLsidBitmap = new BitMapMutex(addrInfo->GetnumWbStripes());
        for (ASTailArrayIdx asTailArrayIdx = 0; asTailArrayIdx < ACTIVE_STRIPE_TAIL_ARRAYLEN; ++asTailArrayIdx)
        {
            activeStripeTail[asTailArrayIdx] = UNMAP_VSA;
        }

        initialized = true;
    }
}

void
WbStripeCtx::Dispose(void)
{
    if (initialized == true)
    {
        if (allocWbLsidBitmap != nullptr)
        {
            delete allocWbLsidBitmap;
            allocWbLsidBitmap = nullptr;
        }

        initialized = false;
    }
}

void
WbStripeCtx::AllocWbStripe(StripeId stripeId)
{
    allocWbLsidBitmap->SetBit(stripeId);
}

StripeId
WbStripeCtx::AllocWbStripe(void)
{
    StripeId stripe = allocWbLsidBitmap->SetNextZeroBit();
    if (allocWbLsidBitmap->IsValidBit(stripe) == false)
    {
        stripe = UNMAP_STRIPE;
    }
    return stripe;
}

void
WbStripeCtx::ReleaseWbStripe(StripeId stripeId)
{
    allocWbLsidBitmap->ClearBit(stripeId);
}

std::mutex&
WbStripeCtx::GetActiveStripeTailLock(ASTailArrayIdx asTailArrayIdx)
{
    return activeStripeTailLock[asTailArrayIdx];
}

void
WbStripeCtx::SetActiveStripeTail(ASTailArrayIdx asTailArrayIdx, VirtualBlkAddr vsa)
{
    activeStripeTail[asTailArrayIdx] = vsa;
}

VirtualBlkAddr
WbStripeCtx::GetActiveStripeTail(ASTailArrayIdx asTailArrayIdx)
{
    return activeStripeTail[asTailArrayIdx];
}

std::vector<VirtualBlkAddr>
WbStripeCtx::GetAllActiveStripeTail(void)
{
    std::vector<VirtualBlkAddr> asTails;
    for (ASTailArrayIdx asTailArrayIdx = 0; asTailArrayIdx < ACTIVE_STRIPE_TAIL_ARRAYLEN; ++asTailArrayIdx)
    {
        asTails.push_back(activeStripeTail[asTailArrayIdx]);
    }
    return asTails;
}

void
WbStripeCtx::SetAllocatedWbStripeCount(int count)
{
    allocWbLsidBitmap->SetNumBitsSet(count);
}

uint64_t
WbStripeCtx::GetAllocatedWbStripeCount(void)
{
    return allocWbLsidBitmap->GetNumBitsSet();
}

uint64_t
WbStripeCtx::GetNumTotalWbStripe(void)
{
    return allocWbLsidBitmap->GetNumBits();
}

std::mutex&
WbStripeCtx::GetAllocWbLsidBitmapLock(void)
{
    return allocWbLsidBitmap->GetLock();
}

void
WbStripeCtx::AfterLoad(char* buf)
{
    AllocatorCtxHeader* header = (AllocatorCtxHeader*)buf;
    allocWbLsidBitmap->SetNumBitsSet(header->numValidWbLsid);
}

void
WbStripeCtx::BeforeFlush(int section, char* buf)
{
    switch (section)
    {
        case AC_HEADER:
        {
            AllocatorCtxHeader* header = (AllocatorCtxHeader*)buf;
            header->numValidWbLsid = allocWbLsidBitmap->GetNumBitsSetWoLock();
            break;
        }
        case AC_ALLOCATE_WBLSID_BITMAP:
        {
            memcpy(buf, GetSectionAddr(section), GetSectionSize(section));
            break;
        }
        case AC_ACTIVE_STRIPE_TAIL:
        {
            for (int index = 0; index < ACTIVE_STRIPE_TAIL_ARRAYLEN; index++)
            {
                std::unique_lock<std::mutex> volLock(activeStripeTailLock[index]);
                int offset = sizeof(VirtualBlkAddr) * index;
                memcpy(buf + offset, &activeStripeTail[index], sizeof(VirtualBlkAddr));
            }
            break;
        }
    }
}

void
WbStripeCtx::FinalizeIo(AsyncMetaFileIoCtx* ctx)
{
}

char*
WbStripeCtx::GetSectionAddr(int section)
{
    char* ret = nullptr;
    switch (section)
    {
        case AC_ALLOCATE_WBLSID_BITMAP:
        {
            ret = (char*)allocWbLsidBitmap->GetMapAddr();
            break;
        }
        case AC_ACTIVE_STRIPE_TAIL:
        {
            ret = (char*)activeStripeTail;
            break;
        }
    }
    return ret;
}

int
WbStripeCtx::GetSectionSize(int section)
{
    int ret = 0;
    switch (section)
    {
        case AC_ALLOCATE_WBLSID_BITMAP:
        {
            ret = allocWbLsidBitmap->GetNumEntry() * BITMAP_ENTRY_SIZE;
            break;
        }
        case AC_ACTIVE_STRIPE_TAIL:
        {
            ret = sizeof(activeStripeTail);
            break;
        }
    }
    return ret;
}

uint64_t
WbStripeCtx::GetStoredVersion(void)
{
    return 0;
}

void
WbStripeCtx::ResetDirtyVersion(void)
{
    return;
}

//----------------------------------------------------------------------------//

} // namespace pos

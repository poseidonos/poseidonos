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

#include "src/allocator/context_manager/allocator_ctx/allocator_ctx.h"

#include <string>
#include <vector>

#include "src/allocator/address/allocator_address_info.h"
#include "src/allocator/context_manager/io_ctx/allocator_io_ctx.h"
#include "src/include/meta_const.h"
#include "src/include/pos_event_id.h"
#include "src/lib/bitmap.h"
#include "src/logger/logger.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"

namespace pos
{
AllocatorCtx::AllocatorCtx(TelemetryPublisher* tp_, AllocatorCtxHeader* header, BitMapMutex* allocWbLsidBitmap_, AllocatorAddressInfo* info_)
: ctxStoredVersion(0),
  ctxDirtyVersion(0),
  addrInfo(info_),
  tp(tp_),
  initialized(false)
{
    ctxHeader.sig = SIG_ALLOCATOR_CTX;
    ctxHeader.numValidWbLsid = 0;
    ctxHeader.ctxVersion = 0;
    currentSsdLsid = 0;

    if (header != nullptr)
    {
        ctxHeader.sig = header->sig;
        ctxHeader.numValidWbLsid = header->numValidWbLsid;
        ctxHeader.ctxVersion = header->ctxVersion;
    }

    allocWbLsidBitmap = allocWbLsidBitmap_;
}

AllocatorCtx::AllocatorCtx(TelemetryPublisher* tp_, AllocatorAddressInfo* info)
: AllocatorCtx(tp_, nullptr, nullptr, info)
{
}

AllocatorCtx::~AllocatorCtx(void)
{
    Dispose();
}

void
AllocatorCtx::Init(void)
{
    if (initialized == true)
    {
        return;
    }

    allocWbLsidBitmap = new BitMapMutex(addrInfo->GetnumWbStripes());
    for (ASTailArrayIdx asTailArrayIdx = 0; asTailArrayIdx < ACTIVE_STRIPE_TAIL_ARRAYLEN; ++asTailArrayIdx)
    {
        activeStripeTail[asTailArrayIdx] = UNMAP_VSA;
        activeWbStripeId[asTailArrayIdx] = UNMAP_STRIPE;
    }
    currentSsdLsid = STRIPES_PER_SEGMENT - 1;

    ctxHeader.ctxVersion = 0;
    ctxStoredVersion = 0;
    ctxDirtyVersion = 0;
    initialized = true;
}

void
AllocatorCtx::Dispose(void)
{
    if (initialized == false)
    {
        return;
    }

    if (allocWbLsidBitmap != nullptr)
    {
        delete allocWbLsidBitmap;
        allocWbLsidBitmap = nullptr;
    }

    initialized = false;
}

void
AllocatorCtx::SetNextSsdLsid(SegmentId segId)
{
    currentSsdLsid = segId * addrInfo->GetstripesPerSegment();
}

void
AllocatorCtx::SetCurrentSsdLsid(StripeId stripe)
{
    currentSsdLsid = stripe;
}

StripeId
AllocatorCtx::GetCurrentSsdLsid(void)
{
    return currentSsdLsid;
}

void
AllocatorCtx::AfterLoad(char* buf)
{
    POS_TRACE_DEBUG(EID(ALLOCATOR_FILE_ERROR), "AllocatorCtx file loaded:{}", ctxHeader.ctxVersion);
    ctxStoredVersion = ctxHeader.ctxVersion;
    ctxDirtyVersion = ctxHeader.ctxVersion + 1;

    AllocatorCtxHeader* header = (AllocatorCtxHeader*)buf;
    allocWbLsidBitmap->SetNumBitsSet(header->numValidWbLsid);
}

void
AllocatorCtx::BeforeFlush(char* buf)
{
    char* currentPtr = buf;

    // AC_HEADER
    ctxHeader.ctxVersion = ctxDirtyVersion++;
    AllocatorCtxHeader* header = (AllocatorCtxHeader*)currentPtr;
    header->numValidWbLsid = allocWbLsidBitmap->GetNumBitsSetWoLock();
    currentPtr += GetSectionSize(AC_HEADER);

    // AC_ALLOCATE_WBLSID_BITMAP
    {
        std::lock_guard<std::mutex> lock(allocWbLsidBitmap->GetLock());
        memcpy(currentPtr, GetSectionAddr(AC_ALLOCATE_WBLSID_BITMAP), GetSectionSize(AC_ALLOCATE_WBLSID_BITMAP));
    }
    currentPtr += GetSectionSize(AC_ALLOCATE_WBLSID_BITMAP);

    // AC_ACTIVE_STRIPE_TAIL
    for (int index = 0; index < ACTIVE_STRIPE_TAIL_ARRAYLEN; index++)
    {
        std::unique_lock<std::mutex> volLock(activeStripeTailLock[index]);
        int offset = sizeof(VirtualBlkAddr) * index;
        memcpy(currentPtr + offset, &activeStripeTail[index], sizeof(VirtualBlkAddr));
    }
    currentPtr += GetSectionSize(AC_ACTIVE_STRIPE_TAIL);
}

void
AllocatorCtx::FinalizeIo(AsyncMetaFileIoCtx* ctx)
{
    ctxStoredVersion = ((AllocatorCtxHeader*)ctx->buffer)->ctxVersion;
}

char*
AllocatorCtx::GetSectionAddr(int section)
{
    char* ret = nullptr;
    switch (section)
    {
        case AC_HEADER:
        {
            ret = (char*)&ctxHeader;
            break;
        }
        case AC_CURRENT_SSD_LSID:
        {
            ret = (char*)&currentSsdLsid;
            break;
        }
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
AllocatorCtx::GetSectionSize(int section)
{
    int ret = 0;
    switch (section)
    {
        case AC_HEADER:
        {
            ret = sizeof(AllocatorCtxHeader);
            break;
        }
        case AC_CURRENT_SSD_LSID:
        {
            ret = sizeof(currentSsdLsid);
            break;
        }
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

void
AllocatorCtx::AllocWbStripe(StripeId stripeId)
{
    allocWbLsidBitmap->SetBit(stripeId);
}

StripeId
AllocatorCtx::AllocFreeWbStripe(void)
{
    StripeId stripe = allocWbLsidBitmap->SetNextZeroBit();
    if (allocWbLsidBitmap->IsValidBit(stripe) == false)
    {
        stripe = UNMAP_STRIPE;
    }
    return stripe;
}

void
AllocatorCtx::ReleaseWbStripe(StripeId stripeId)
{
    allocWbLsidBitmap->ClearBit(stripeId);
}

void
AllocatorCtx::SetAllocatedWbStripeCount(int count)
{
    allocWbLsidBitmap->SetNumBitsSet(count);
}

uint64_t
AllocatorCtx::GetAllocatedWbStripeCount(void)
{
    return allocWbLsidBitmap->GetNumBitsSet();
}

uint64_t
AllocatorCtx::GetNumTotalWbStripe(void)
{
    return allocWbLsidBitmap->GetNumBits();
}

std::vector<VirtualBlkAddr>
AllocatorCtx::GetAllActiveStripeTail(void)
{
    std::vector<VirtualBlkAddr> asTails;
    for (ASTailArrayIdx asTailArrayIdx = 0; asTailArrayIdx < ACTIVE_STRIPE_TAIL_ARRAYLEN; ++asTailArrayIdx)
    {
        asTails.push_back(activeStripeTail[asTailArrayIdx]);
    }
    return asTails;
}

VirtualBlkAddr
AllocatorCtx::GetActiveStripeTail(ASTailArrayIdx asTailArrayIdx)
{
    return activeStripeTail[asTailArrayIdx];
}

StripeId
AllocatorCtx::GetActiveWbStripeId(ASTailArrayIdx asTailArrayIdx)
{
    return activeWbStripeId[asTailArrayIdx];
}

void
AllocatorCtx::SetActiveStripeTail(ASTailArrayIdx asTailArrayIdx, VirtualBlkAddr vsa)
{
    activeStripeTail[asTailArrayIdx] = vsa;
}

void
AllocatorCtx::SetNewActiveStripeTail(ASTailArrayIdx asTailArrayIdx, VirtualBlkAddr vsa, StripeId wbLsid)
{
    activeStripeTail[asTailArrayIdx] = vsa;
    activeWbStripeId[asTailArrayIdx] = wbLsid;
}

std::mutex&
AllocatorCtx::GetActiveStripeTailLock(ASTailArrayIdx asTailArrayIdx)
{
    return activeStripeTailLock[asTailArrayIdx];
}

uint64_t
AllocatorCtx::GetStoredVersion(void)
{
    return ctxStoredVersion;
}

void
AllocatorCtx::ResetDirtyVersion(void)
{
    ctxDirtyVersion = 0;
}

std::string
AllocatorCtx::GetFilename(void)
{
    return "AllocatorContexts";
}

uint32_t
AllocatorCtx::GetSignature(void)
{
    return SIG_ALLOCATOR_CTX;
}

int
AllocatorCtx::GetNumSections(void)
{
    return NUM_ALLOCATOR_CTX_SECTION;
}

}  // namespace pos

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
  initialized(false),
  addrInfo(info_),
  tp(tp_)
{
    currentSsdLsid.data = 0;

    if (header != nullptr)
    {
        ctxHeader.data = *header;
    }

    allocWbLsidBitmap.data = allocWbLsidBitmap_;
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

    if (allocWbLsidBitmap.data == nullptr)
    {
        allocWbLsidBitmap.data = new BitMapMutex(addrInfo->GetnumWbStripes());
        POS_TRACE_DEBUG(EID(ALLOCATOR_INFO),
            "Init bitmap, allocWbBitmapNumBits: {}, allocWbBitmapNumBits: {}", 
            allocWbLsidBitmap.data->GetNumBitsSet(), allocWbLsidBitmap.data->GetNumEntry());
    }

    for (ASTailArrayIdx asTailArrayIdx = 0; asTailArrayIdx < ACTIVE_STRIPE_TAIL_ARRAYLEN; ++asTailArrayIdx)
    {
        activeStripeTail.data[asTailArrayIdx] = UNMAP_VSA;
    }
    currentSsdLsid.data = STRIPES_PER_SEGMENT - 1;

    ctxHeader.data.ctxVersion = 0;
    ctxStoredVersion = 0;
    ctxDirtyVersion = 0;

    _UpdateSectionInfo();

    initialized = true;
}

void
AllocatorCtx::_UpdateSectionInfo(void)
{
    uint64_t currentOffset = 0;

    // AC_HEADER
    ctxHeader.InitAddressInfoWithItsData(currentOffset);
    currentOffset += ctxHeader.GetSectionSize();

    // AC_CURRENT_SSD_LSID
    currentSsdLsid.InitAddressInfoWithItsData(currentOffset);
    currentOffset += currentSsdLsid.GetSectionSize();

    // AC_ALLOCATE_WBLSID_BITMAP
    allocWbLsidBitmap.InitAddressInfo(
        (char*)(allocWbLsidBitmap.data->GetMapAddr()),
        currentOffset,
        allocWbLsidBitmap.data->GetNumEntry() * BITMAP_ENTRY_SIZE);
    currentOffset += allocWbLsidBitmap.GetSectionSize();

    // AC_ACTIVE_STRIPE_TAIL
    activeStripeTail.InitAddressInfoWithItsData(currentOffset);
    currentOffset += activeStripeTail.GetSectionSize();

    // AC_EXTENDED
    uint64_t sectionSize = this->ctxExtended.GetSectionSize();
    this->ctxExtended.InitAddressInfo(currentOffset, sectionSize);
    currentOffset += sectionSize;

    totalDataSize = currentOffset;
}

void
AllocatorCtx::Dispose(void)
{
    if (initialized == false)
    {
        return;
    }

    if (allocWbLsidBitmap.data != nullptr)
    {
        delete allocWbLsidBitmap.data;
        allocWbLsidBitmap.data = nullptr;
    }

    initialized = false;
}

void
AllocatorCtx::SetNextSsdLsid(SegmentId segId)
{
    currentSsdLsid.data = segId * addrInfo->GetstripesPerSegment();
}

void
AllocatorCtx::SetCurrentSsdLsid(StripeId stripe)
{
    currentSsdLsid.data = stripe;
}

StripeId
AllocatorCtx::GetCurrentSsdLsid(void)
{
    return currentSsdLsid.data;
}

void
AllocatorCtx::AfterLoad(char* buf)
{
    AllocatorCtxHeader* header = reinterpret_cast<AllocatorCtxHeader*>(buf);
    assert(header->sig == SIG_ALLOCATOR_CTX);

    // AC_HEADER
    ctxHeader.CopyFrom(buf);

    // AC_CURRENT_SSD_LSID
    currentSsdLsid.CopyFrom(buf);

    // AC_ALLOCATE_WBLSID_BITMAP
    allocWbLsidBitmap.CopyFrom(buf);

    // AC_ACTIVE_STRIPE_TAIL
    activeStripeTail.CopyFrom(buf);

    // AC_EXTENDED
    this->ctxExtended.CopyFrom(buf);

    POS_TRACE_DEBUG(EID(ALLOCATOR_FILE_ERROR), "AllocatorCtx file loaded:{}", ctxHeader.data.ctxVersion);
    ctxStoredVersion = ctxHeader.data.ctxVersion;
    ctxDirtyVersion = ctxHeader.data.ctxVersion + 1;

    allocWbLsidBitmap.data->SetNumBitsSet(header->numValidWbLsid);

}

void
AllocatorCtx::BeforeFlush(char* buf, ContextSectionBuffer externalBuf)
{
    std::lock_guard<std::mutex> lock(allocCtxLock);

    // AC_HEADER
    ctxHeader.data.ctxVersion = ctxDirtyVersion++;
    ctxHeader.data.numValidWbLsid = allocWbLsidBitmap.data->GetNumBitsSetWoLock();

    ctxHeader.CopyTo(buf);

    // AC_CURRENT_SSD_LSID
    currentSsdLsid.CopyTo(buf);

    // AC_ALLOCATE_WBLSID_BITMAP
    {
        std::lock_guard<std::mutex> lock(allocWbLsidBitmap.data->GetLock());
        allocWbLsidBitmap.CopyTo(buf);
    }

    // AC_ACTIVE_STRIPE_TAIL
    for (auto index = 0; index < ACTIVE_STRIPE_TAIL_ARRAYLEN; index++)
    {
        std::lock_guard<std::mutex> lock(activeStripeTailLock[index]);
        activeStripeTail.CopyToListElement(buf, index);
    }
    
    // AC_EXTENDED
    this->ctxExtended.CopyTo(buf);
}
void
AllocatorCtx::AfterFlush(char* buf)
{
    AllocatorCtxHeader* header = reinterpret_cast<AllocatorCtxHeader*>(buf);
    assert(header->sig == SIG_ALLOCATOR_CTX);

    ctxStoredVersion = header->ctxVersion;

    POS_TRACE_DEBUG(EID(ALLOCATOR_DEBUG),
        "AllocatorCtx stored, array_id: {}, context_version: {}",
        addrInfo->GetArrayId(), ctxStoredVersion);
}

ContextSectionAddr
AllocatorCtx::GetSectionInfo(int section)
{
    if (section == AC_HEADER)
    {
        return ctxHeader.GetSectionInfo();
    }
    else if (section == AC_CURRENT_SSD_LSID)
    {
        return currentSsdLsid.GetSectionInfo();
    }
    else if (section == AC_ALLOCATE_WBLSID_BITMAP)
    {
        return allocWbLsidBitmap.GetSectionInfo();
    }
    else if (section == AC_ACTIVE_STRIPE_TAIL)
    {
        return activeStripeTail.GetSectionInfo();
    }
    else if (section == AC_EXTENDED)
    {
        return this->ctxExtended.GetSectionInfo();
    }
    else
    {
        assert(false);
    }
}

void
AllocatorCtx::AllocWbStripe(StripeId stripeId)
{
    allocWbLsidBitmap.data->SetBit(stripeId);
}

StripeId
AllocatorCtx::AllocFreeWbStripe(void)
{
    StripeId stripe = allocWbLsidBitmap.data->SetNextZeroBit();
    if (allocWbLsidBitmap.data->IsValidBit(stripe) == false)
    {
        stripe = UNMAP_STRIPE;
    }
    return stripe;
}

void
AllocatorCtx::ReleaseWbStripe(StripeId stripeId)
{
    allocWbLsidBitmap.data->ClearBit(stripeId);
}

void
AllocatorCtx::SetAllocatedWbStripeCount(int count)
{
    allocWbLsidBitmap.data->SetNumBitsSet(count);
}

uint64_t
AllocatorCtx::GetAllocatedWbStripeCount(void)
{
    return allocWbLsidBitmap.data->GetNumBitsSet();
}

uint64_t
AllocatorCtx::GetNumTotalWbStripe(void)
{
    return allocWbLsidBitmap.data->GetNumBits();
}

void
AllocatorCtx::CopyContextSectionToBufferforWBT(int sectionId, char* dstBuf)
{
    if (sectionId == AC_CURRENT_SSD_LSID)
    {
        currentSsdLsid.CopyTo(dstBuf);
    }
    else if (sectionId == AC_ALLOCATE_WBLSID_BITMAP)
    {
        allocWbLsidBitmap.CopyTo(dstBuf);
    }
    else if (sectionId == AC_ACTIVE_STRIPE_TAIL)
    {
        activeStripeTail.CopyTo(dstBuf);
    }
}

void
AllocatorCtx::CopyContextSectionFromBufferforWBT(int sectionId, char* srcBuf)
{
    if (sectionId == AC_CURRENT_SSD_LSID)
    {
        currentSsdLsid.CopyFrom(srcBuf);
    }
    else if (sectionId == AC_ALLOCATE_WBLSID_BITMAP)
    {
        allocWbLsidBitmap.CopyFrom(srcBuf);
    }
    else if (sectionId == AC_ACTIVE_STRIPE_TAIL)
    {
        activeStripeTail.CopyFrom(srcBuf);
    }
}

std::vector<VirtualBlkAddr>
AllocatorCtx::GetAllActiveStripeTail(void)
{
    std::vector<VirtualBlkAddr> asTails;
    for (ASTailArrayIdx asTailArrayIdx = 0; asTailArrayIdx < ACTIVE_STRIPE_TAIL_ARRAYLEN; ++asTailArrayIdx)
    {
        asTails.push_back(activeStripeTail.data[asTailArrayIdx]);
    }
    return asTails;
}

VirtualBlkAddr
AllocatorCtx::GetActiveStripeTail(ASTailArrayIdx asTailArrayIdx)
{
    return activeStripeTail.data[asTailArrayIdx];
}

void
AllocatorCtx::SetActiveStripeTail(ASTailArrayIdx asTailArrayIdx, VirtualBlkAddr vsa)
{
    activeStripeTail.data[asTailArrayIdx] = vsa;
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

int
AllocatorCtx::GetNumSections(void)
{
    return NUM_ALLOCATOR_CTX_SECTION;
}

uint64_t
AllocatorCtx::GetTotalDataSize(void)
{
    return ctxHeader.GetSectionSize() + currentSsdLsid.GetSectionSize()
        + allocWbLsidBitmap.GetSectionSize() + activeStripeTail.GetSectionSize() + this->ctxExtended.GetSectionSize();
}
}  // namespace pos

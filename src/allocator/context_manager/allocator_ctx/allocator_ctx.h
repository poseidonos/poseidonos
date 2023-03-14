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

#pragma once

#include <string>
#include <vector>

#include "src/allocator/context_manager/context/active_stripe_tail_context_section.h"
#include "src/allocator/context_manager/context/context.h"
#include "src/allocator/context_manager/context/context_section.h"
#include "src/allocator/context_manager/i_allocator_file_io_client.h"
#include "src/lib/bitmap.h"

namespace pos
{
class AllocatorAddressInfo;
class TelemetryPublisher;

class AllocatorCtx : public IAllocatorFileIoClient
{
public:
    AllocatorCtx(void) = default;
    AllocatorCtx(TelemetryPublisher* tp_, AllocatorCtxHeader* header, BitMapMutex* allocWbLsidBitmap_, AllocatorAddressInfo* info);
    explicit AllocatorCtx(TelemetryPublisher* tp_, AllocatorAddressInfo* info);
    virtual ~AllocatorCtx(void);
    virtual void Init(void);
    virtual void Dispose(void);

    virtual void AfterLoad(char* buf) override;
    virtual void BeforeFlush(char* buf, ContextSectionBuffer externalBuf = INVALID_CONTEXT_SECTION_BUFFER) override;
    virtual void AfterFlush(char* buf) override;
    virtual ContextSectionAddr GetSectionInfo(int section) override;
    virtual uint64_t GetStoredVersion(void) override;
    virtual void ResetDirtyVersion(void) override;
    virtual int GetNumSections(void) override;
    virtual uint64_t GetTotalDataSize(void) override;

    virtual void SetCurrentSsdLsid(StripeId stripe);
    virtual StripeId GetCurrentSsdLsid(void);
    virtual void SetNextSsdLsid(SegmentId segId);

    virtual void AllocWbStripe(StripeId stripeId);
    virtual StripeId AllocFreeWbStripe(void);
    virtual void ReleaseWbStripe(StripeId stripeId);
    virtual void SetAllocatedWbStripeCount(int count);
    virtual uint64_t GetAllocatedWbStripeCount(void);
    virtual uint64_t GetNumTotalWbStripe(void);

    virtual void CopyContextSectionToBufferforWBT(int sectionId, char* dstBuf);
    virtual void CopyContextSectionFromBufferforWBT(int sectionId, char* srcBuf);

    virtual std::vector<VirtualBlkAddr> GetAllActiveStripeTail(void);
    virtual VirtualBlkAddr GetActiveStripeTail(ASTailArrayIdx asTailArrayIdx);
    virtual void SetActiveStripeTail(ASTailArrayIdx asTailArrayIdx, VirtualBlkAddr vsa);

    virtual std::mutex& GetCtxLock(void) { return allocCtxLock; }
    virtual std::mutex& GetActiveStripeTailLock(ASTailArrayIdx asTailArrayIdx);

private:
    void _UpdateSectionInfo(void);

    // Data to be stored
    ContextSection<AllocatorCtxHeader> ctxHeader;
    ContextSection<StripeId> currentSsdLsid;
    ContextSection<BitMapMutex*> allocWbLsidBitmap;
    ActiveStripeTailContextSection activeStripeTail;

    uint64_t totalDataSize;

    // In-memory data structures
    std::atomic<uint64_t> ctxStoredVersion;
    std::atomic<uint64_t> ctxDirtyVersion;

    std::mutex activeStripeTailLock[ACTIVE_STRIPE_TAIL_ARRAYLEN];

    std::mutex allocCtxLock;
    bool initialized;

    // Dependencies
    AllocatorAddressInfo* addrInfo;
    TelemetryPublisher* tp;
};

} // namespace pos

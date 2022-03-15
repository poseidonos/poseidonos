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

#include "src/allocator/context_manager/i_allocator_file_io_client.h"
#include "src/allocator/include/allocator_const.h"
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

    virtual void AfterLoad(char* buf);
    virtual void BeforeFlush(char* buf);
    virtual std::mutex& GetCtxLock(void) { return allocCtxLock; }
    virtual void FinalizeIo(AsyncMetaFileIoCtx* ctx);
    virtual char* GetSectionAddr(int section);
    virtual int GetSectionSize(int section);
    virtual uint64_t GetStoredVersion(void);
    virtual void ResetDirtyVersion(void);
    virtual std::string GetFilename(void);
    virtual uint32_t GetSignature(void);
    virtual int GetNumSections(void);

    virtual void SetCurrentSsdLsid(StripeId stripe);
    virtual StripeId GetCurrentSsdLsid(void);
    virtual void SetNextSsdLsid(SegmentId segId);

    virtual void AllocWbStripe(StripeId stripeId);
    virtual StripeId AllocFreeWbStripe(void);
    virtual void ReleaseWbStripe(StripeId stripeId);
    virtual void SetAllocatedWbStripeCount(int count);
    virtual uint64_t GetAllocatedWbStripeCount(void);
    virtual uint64_t GetNumTotalWbStripe(void);

    virtual std::vector<VirtualBlkAddr> GetAllActiveStripeTail(void);
    virtual VirtualBlkAddr GetActiveStripeTail(ASTailArrayIdx asTailArrayIdx);
    virtual StripeId GetActiveWbStripeId(ASTailArrayIdx asTailArrayIdx);
    virtual void SetActiveStripeTail(ASTailArrayIdx asTailArrayIdx, VirtualBlkAddr vsa);
    virtual void SetNewActiveStripeTail(ASTailArrayIdx asTailArrayIdx, VirtualBlkAddr vsa, StripeId wbLsid);

    virtual std::mutex& GetActiveStripeTailLock(ASTailArrayIdx asTailArrayIdx);

    static const uint32_t SIG_ALLOCATOR_CTX = 0xBFBFBFBF;

private:
    AllocatorCtxHeader ctxHeader;
    std::atomic<uint64_t> ctxStoredVersion;
    std::atomic<uint64_t> ctxDirtyVersion;

    VirtualBlkAddr activeStripeTail[ACTIVE_STRIPE_TAIL_ARRAYLEN];
    StripeId activeWbStripeId[ACTIVE_STRIPE_TAIL_ARRAYLEN];
    std::mutex activeStripeTailLock[ACTIVE_STRIPE_TAIL_ARRAYLEN];
    BitMapMutex* allocWbLsidBitmap = nullptr;

    StripeId currentSsdLsid;

    AllocatorAddressInfo* addrInfo;
    TelemetryPublisher* tp;

    std::mutex allocCtxLock;
    bool initialized;
};

} // namespace pos

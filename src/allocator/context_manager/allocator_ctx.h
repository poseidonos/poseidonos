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

#pragma once

#include <string>
#include <vector>

#include "src/allocator/context_manager/i_allocator_file_io_client.h"
#include "src/allocator/context_manager/segment/segment_states.h"
#include "src/allocator/include/allocator_const.h"
#include "src/lib/bitmap.h"

namespace pos
{
class AllocatorAddressInfo;
class SegmentLock;
class AllocatorCtx : public IAllocatorFileIoClient
{
public:
    AllocatorCtx(AllocatorAddressInfo* info, std::string arrayName);
    virtual ~AllocatorCtx(void);
    void Init(void);
    void Close(void);

    virtual void AfterLoad(char* buf);
    virtual void BeforeFlush(int section, char* buf);
    virtual void FinalizeIo(AsyncMetaFileIoCtx* ctx);
    virtual char* GetSectionAddr(int section);
    virtual int GetSectionSize(int section);
    virtual uint64_t GetStoredVersion(void);
    virtual void ResetDirtyVersion(void);

    StripeId UpdatePrevLsid(void);
    void SetCurrentSsdLsid(StripeId stripe);
    void RollbackCurrentSsdLsid(void);
    StripeId GetCurrentSsdLsid(void);
    StripeId GetPrevSsdLsid(void);
    void SetPrevSsdLsid(StripeId stripeId);
    void SetNextSsdLsid(SegmentId segId);

    void AllocateSegment(SegmentId segId);
    void ReleaseSegment(SegmentId segId);
    SegmentId AllocateFreeSegment(int startSegId);
    uint64_t GetNumOfFreeUserDataSegment(void);
    SegmentState GetSegmentState(SegmentId segmentId, bool needlock);
    void SetSegmentState(SegmentId segmentId, SegmentState state, bool needlock);

    void SetAllocatedSegmentCount(int count);
    int GetAllocatedSegmentCount(void);
    int GetTotalSegmentsCount(void);

    std::mutex& GetSegStateLock(SegmentId segId);
    std::mutex& GetAllocatorCtxLock(void) { return allocCtxLock; }

private:
    static const uint32_t SIG_ALLOCATOR_CTX = 0xACACACAC;

    // File
    AllocatorCtxHeader ctxHeader;
    std::atomic<uint64_t> ctxStoredVersion;
    std::atomic<uint64_t> ctxDirtyVersion;

    // Segment Allocation
    BitMapMutex* allocSegBitmap; // Unset:Free, Set:Not-Free
    StripeId prevSsdLsid;
    StripeId currentSsdLsid;
    SegmentStates* segmentStates;

    // DOCs
    AllocatorAddressInfo* addrInfo;
    std::string arrayName;

    // Lock
    std::mutex allocCtxLock;
    SegmentLock* segStateLocks;
};

} // namespace pos

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

#include "src/allocator/address/allocator_address_info.h"
#include "src/allocator/context_manager/i_allocator_file_io_client.h"
#include "src/allocator/context_manager/segment_ctx/segment_info.h"
#include "src/allocator/context_manager/segment_ctx/segment_states.h"
#include "src/allocator/context_manager/segment_ctx/segment_lock.h"
#include "src/allocator/include/allocator_const.h"
#include "src/include/address_type.h"
#include "src/lib/bitmap.h"

namespace pos
{
class SegmentCtx : public IAllocatorFileIoClient
{
public:
    SegmentCtx(void) = default;
    SegmentCtx(SegmentCtxHeader* header, SegmentInfo* segmentInfo_, AllocatorAddressInfo* addrInfo_);
    SegmentCtx(SegmentCtxHeader* header, SegmentInfo* segmentInfo_, 
        SegmentStates* segmentStates_, SegmentLock* segmentStateLocks_,
        AllocatorAddressInfo* addrInfo_);
    explicit SegmentCtx(AllocatorAddressInfo* info);
    virtual ~SegmentCtx(void);
    virtual void Init(void);
    virtual void Dispose(void);

    virtual void AfterLoad(char* buf);
    virtual void BeforeFlush(int section, char* buf);
    virtual void FinalizeIo(AsyncMetaFileIoCtx* ctx);
    virtual char* GetSectionAddr(int section);
    virtual int GetSectionSize(int section);
    virtual uint64_t GetStoredVersion(void);
    virtual void ResetDirtyVersion(void);

    virtual uint32_t IncreaseValidBlockCount(SegmentId segId, uint32_t cnt);
    virtual int32_t DecreaseValidBlockCount(SegmentId segId, uint32_t cnt);
    virtual uint32_t GetValidBlockCount(SegmentId segId);
    virtual void SetOccupiedStripeCount(SegmentId segId, int count);
    virtual int GetOccupiedStripeCount(SegmentId segId);
    virtual int IncreaseOccupiedStripeCount(SegmentId segId);

    virtual void SetSegmentState(SegmentId segId, SegmentState state, bool needlock);
    virtual SegmentState GetSegmentState(SegmentId segId, bool needlock);
    virtual std::mutex& GetSegStateLock(SegmentId segId);

    virtual SegmentInfo* GetSegmentInfo(void) { return segmentInfos;}
    virtual std::mutex& GetSegmentCtxLock(void) { return segCtxLock;}

    virtual void CopySegmentInfoToBufferforWBT(WBTAllocatorMetaType type, char* dstBuf);
    virtual void CopySegmentInfoFromBufferforWBT(WBTAllocatorMetaType type, char* dstBuf);

    static const uint32_t SIG_SEGMENT_CTX = 0xAFAFAFAF;

private:
    SegmentCtxHeader ctxHeader;
    std::atomic<uint64_t> ctxDirtyVersion;
    std::atomic<uint64_t> ctxStoredVersion;
    SegmentInfo* segmentInfos;
    SegmentStates* segmentStates;

    uint32_t numSegments;
    bool initialized;

    AllocatorAddressInfo* addrInfo;

    std::mutex segCtxLock;
    SegmentLock* segStateLocks;
};

} // namespace pos

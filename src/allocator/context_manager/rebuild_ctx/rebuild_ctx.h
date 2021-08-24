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

#include <set>
#include <string>
#include <utility>

#include "src/allocator/address/allocator_address_info.h"
#include "src/allocator/context_manager/allocator_ctx/allocator_ctx.h"
#include "src/allocator/context_manager/i_allocator_file_io_client.h"
#include "src/allocator/include/allocator_const.h"
#include "src/state/interface/i_state_control.h"

namespace pos
{
class RebuildCtx : public IAllocatorFileIoClient
{
public:
    RebuildCtx(void) = default;
    RebuildCtx(RebuildCtxHeader* header, AllocatorCtx* allocCtx, AllocatorAddressInfo* info); // for UT
    RebuildCtx(AllocatorCtx* allocCtx, AllocatorAddressInfo* info);
    virtual ~RebuildCtx(void);
    virtual void Init(void);
    virtual void Dispose(void);

    virtual void AfterLoad(char* buf);
    virtual void BeforeFlush(int section, char* buf);
    virtual void FinalizeIo(AsyncMetaFileIoCtx* ctx);
    virtual char* GetSectionAddr(int section);
    virtual int GetSectionSize(int section);
    virtual uint64_t GetStoredVersion(void);
    virtual void ResetDirtyVersion(void);

    virtual SegmentId GetRebuildTargetSegment(void);
    virtual int ReleaseRebuildSegment(SegmentId segmentId);
    virtual bool NeedRebuildAgain(void);
    virtual int FreeSegmentInRebuildTarget(SegmentId segId);
    virtual bool IsRebuidTargetSegmentsEmpty(void);
    virtual bool IsRebuildTargetSegment(SegmentId segId);
    virtual uint32_t GetRebuildTargetSegmentCount(void);
    virtual RTSegmentIter GetRebuildTargetSegmentsBegin(void);
    virtual RTSegmentIter GetRebuildTargetSegmentsEnd(void);
    virtual int MakeRebuildTarget(void);
    virtual int StopRebuilding(void);

    // For Testing
    virtual std::mutex& GetLock(void) { return rebuildLock; }
    virtual std::pair<RTSegmentIter, bool> EmplaceRebuildTargetSegment(SegmentId segmentId);
    virtual void SetTargetSegmentCnt(uint32_t val) { targetSegmentCount = val; }

    static const uint32_t SIG_REBUILD_CTX = 0xCFCFCFCF;

private:
    void _EraseRebuildTargetSegments(RTSegmentIter iter);

    AllocatorAddressInfo* addrInfo;
    std::atomic<uint64_t> ctxStoredVersion;
    std::atomic<uint64_t> ctxDirtyVersion;
    RebuildCtxHeader ctxHeader;
    bool needContinue;
    std::atomic<uint32_t> targetSegmentCount;   // for monitor
    std::set<SegmentId> targetSegmentList;
    SegmentId currentTarget;
    std::mutex rebuildLock;

    AllocatorCtx* allocatorCtx;
    bool initialized;
};

} // namespace pos

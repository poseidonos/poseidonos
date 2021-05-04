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

#include "src/allocator/address/allocator_address_info.h"
#include "src/allocator/context_manager/gc_ctx.h"
#include "src/allocator/context_manager/rebuild/rebuild_ctx.h"
#include "src/allocator/i_context_replayer.h"
#include "src/allocator/i_context_manager.h"
#include "src/allocator/include/allocator_const.h"

namespace pos
{
class IAllocatorFileIoClient;
class AllocatorFileIoManager;
class AllocatorCtx;
class SegmentCtx;
class WbStripeCtx;
class ContextReplayer;

const int NO_REBUILD_TARGET_USER_SEGMENT = 0;

class ContextManager : public IContextManager
{
public:
    ContextManager(AllocatorAddressInfo* info, std::string arrayName);
    virtual ~ContextManager(void);
    void Init(void);
    void Close(void);

    virtual int FlushContextsSync(void);
    virtual int FlushContextsAsync(EventSmartPtr callback);
    virtual void UpdateOccupiedStripeCount(StripeId lsid);
    virtual SegmentId AllocateFreeSegment(bool forUser);
    virtual SegmentId AllocateGCVictimSegment(void);
    virtual SegmentId AllocateRebuildTargetSegment(void);
    virtual int ReleaseRebuildSegment(SegmentId segmentId);
    virtual bool NeedRebuildAgain(void);
    virtual int GetNumFreeSegment(void);
    virtual CurrentGcMode GetCurrentGcMode(void);
    virtual int GetGcThreshold(CurrentGcMode mode);
    virtual uint64_t GetStoredContextVersion(int owner);

    void FreeUserDataSegment(SegmentId segId);
    int SetNextSsdLsid(void);
    char* GetContextSectionAddr(int owner, int section);
    int GetContextSectionSize(int owner, int section);

    RebuildCtx* GetRebuldCtx(void) { return rebuildCtx; }
    SegmentCtx* GetSegmentCtx(void) { return segmentCtx; }
    AllocatorCtx* GetAllocatorCtx(void) { return allocatorCtx; }
    WbStripeCtx* GetWbStripeCtx(void) { return wbStripeCtx; }
    ContextReplayer* GetContextReplayer(void) { return contextReplayer; }
    GcCtx* GetGcCtx(void) { return &gcCtx; }
    std::mutex& GetCtxLock(void) { return ctxLock; }

private:
    void _UpdateSectionInfo(void);
    void _LoadContexts(void);
    int _FlushAsync(int owner, EventSmartPtr callback);
    int _FlushSync(int owner);
    void _FlushCompletedThenCB(AsyncMetaFileIoCtx* ctx);
    void _PrepareBuffer(int owner, char* buf);
    void _ResetSegmentStates(void);
    void _FreeSegment(SegmentId segId);

    // file io
    IAllocatorFileIoClient* fileOwner[NUM_FILES];
    int numAsyncIoIssued;
    std::atomic<bool> flushInProgress;
    EventSmartPtr flushCallback;

    // DOCs
    AllocatorFileIoManager* fileIoManager;
    AllocatorAddressInfo* addrInfo;
    AllocatorCtx* allocatorCtx;
    SegmentCtx* segmentCtx;
    WbStripeCtx* wbStripeCtx;
    RebuildCtx* rebuildCtx;
    ContextReplayer* contextReplayer;
    GcCtx gcCtx;

    std::string arrayName;

    // Lock
    std::mutex ctxLock;
};

} // namespace pos

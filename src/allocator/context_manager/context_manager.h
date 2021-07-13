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
#include "src/allocator/context_manager/gc_ctx/gc_ctx.h"
#include "src/allocator/context_manager/rebuild_ctx/rebuild_ctx.h"
#include "src/allocator/i_context_manager.h"
#include "src/allocator/i_context_replayer.h"
#include "src/allocator/include/allocator_const.h"

namespace pos
{
class IAllocatorFileIoClient;
class AllocatorFileIoManager;
class AllocatorCtx;
class SegmentCtx;
class WbStripeCtx;
class ContextReplayer;
class TelemetryPublisher;

const int NO_REBUILD_TARGET_USER_SEGMENT = 0;

class ContextManager : public IContextManager
{
public:
    ContextManager(void) = default;
    ContextManager(TelemetryPublisher* tp, AllocatorCtx* allocCtx_, SegmentCtx* segCtx_, RebuildCtx* rebuildCtx_,
        WbStripeCtx* wbstripeCtx_, AllocatorFileIoManager* fileMananager_,
        ContextReplayer* ctxReplayer_, bool flushProgress, AllocatorAddressInfo* info_, std::string arrayName_);
    ContextManager(TelemetryPublisher* tp, AllocatorAddressInfo* info, std::string arrayName);
    virtual ~ContextManager(void);
    virtual void Init(void);
    virtual void Close(void);

    virtual int FlushContextsSync(void);
    virtual int FlushContextsAsync(EventSmartPtr callback);
    virtual void UpdateOccupiedStripeCount(StripeId lsid);
    virtual SegmentId AllocateFreeSegment(void);
    virtual SegmentId AllocateGCVictimSegment(void);
    virtual SegmentId AllocateRebuildTargetSegment(void);
    virtual int ReleaseRebuildSegment(SegmentId segmentId);
    virtual bool NeedRebuildAgain(void);
    virtual int MakeRebuildTarget(void);
    virtual int StopRebuilding(void);
    virtual int GetNumOfFreeSegment(bool needLock);
    virtual GcMode GetCurrentGcMode(void);
    virtual int GetGcThreshold(GcMode mode);
    virtual uint64_t GetStoredContextVersion(int owner);

    virtual void FreeUserDataSegment(SegmentId segId);
    virtual int SetNextSsdLsid(void);
    virtual char* GetContextSectionAddr(int owner, int section);
    virtual int GetContextSectionSize(int owner, int section);

    virtual RebuildCtx* GetRebuildCtx(void) { return rebuildCtx; }
    virtual SegmentCtx* GetSegmentCtx(void) { return segmentCtx; }
    virtual AllocatorCtx* GetAllocatorCtx(void) { return allocatorCtx; }
    virtual WbStripeCtx* GetWbStripeCtx(void) { return wbStripeCtx; }
    virtual ContextReplayer* GetContextReplayer(void) { return contextReplayer; }
    virtual GcCtx* GetGcCtx(void) { return &gcCtx; }
    virtual std::mutex& GetCtxLock(void) { return ctxLock; }

    void TestCallbackFunc(AsyncMetaFileIoCtx* ctx, int numIssuedIo);
    void SetCallbackFunc(EventSmartPtr callback);

private:
    void _UpdateSectionInfo(void);
    int _LoadContexts(void);
    int _FlushAsync(int owner, EventSmartPtr callback);
    int _FlushSync(int owner);
    void _FlushCompletedThenCB(AsyncMetaFileIoCtx* ctx);
    MetaIoCbPtr _SetCallbackFunc(int owner, EventSmartPtr callbackEvent);
    void _PrepareBuffer(int owner, char* buf);
    void _ResetSegmentStates(void);
    void _FreeSegment(SegmentId segId);

    std::string fileNames[NUM_FILES] = {"SegmentContext", "AllocatorContexts", "RebuildContext"};
    IAllocatorFileIoClient* fileOwner[NUM_FILES];
    std::atomic<int> numAsyncIoIssued;
    std::atomic<bool> flushInProgress;
    EventSmartPtr flushCallback;

    AllocatorFileIoManager* fileIoManager;
    AllocatorAddressInfo* addrInfo;
    AllocatorCtx* allocatorCtx;
    SegmentCtx* segmentCtx;
    WbStripeCtx* wbStripeCtx;
    RebuildCtx* rebuildCtx;
    ContextReplayer* contextReplayer;
    GcCtx gcCtx;
    GcMode curGcMode;
    GcMode prevGcMode;

    std::string arrayName;
    std::mutex ctxLock;

    TelemetryPublisher* telPublisher;
};

} // namespace pos

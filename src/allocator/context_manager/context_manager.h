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

#include "src/allocator/address/allocator_address_info.h"
#include "src/allocator/i_allocator_ctx.h"
#include "src/allocator/i_context_internal.h"
#include "src/allocator/i_wbstripe_ctx.h"
#include "src/allocator/context_manager/rebuild/rebuild_ctx.h"
#include "src/allocator/context_manager/segment/segment_ctx.h"

#include <vector>
#include <string>

namespace pos
{

const int NO_REBUILD_TARGET_USER_SEGMENT = 0;

struct CtxHeader
{
    uint32_t totalSize = 0; // Sum of all allocator meta size (Byte)
    uint64_t ctxVersion = 0;
    uint32_t numValidWbLsid = 0;
    uint32_t numValidSegment = 0;
};

class ContextManager : public IAllocatorCtx, public IWBStripeCtx, public IContextInternal
{
    class RawCtxInfo
    {
    public:
        uint32_t GetSize() { return size; }
        char* GetAddr() { return addr; }

        char* addr = nullptr;
        uint32_t size = 0;
        uint32_t offset = 0;
    };

public:
    ContextManager(AllocatorAddressInfo* info, std::string arrayName);
    virtual ~ContextManager(void);
    void Init(void);

    int FlushAllocatorCtxs(EventSmartPtr callback) override;
    int StoreAllocatorCtxs(void) override;
    uint64_t GetAllocatorCtxsStoredVersion(void) override;
    void ResetAllocatorCtxsDirtyVersion(void) override;

    void ReplayStripeAllocation(StripeId vsid, StripeId wbLsid) override;
    void ReplayStripeFlushed(StripeId wbLsid) override;
    std::vector<VirtualBlkAddr> GetAllActiveStripeTail(void) override;
    void ResetActiveStripeTail(int index) override;

    std::mutex& GetCtxLock(void) override { return ctxLock; }

    RebuildCtx* GetRebuldCtx(void) { return rebuildCtx; }
    SegmentCtx* GetSegmentCtx(void) { return segmentCtx; }
    VirtualBlkAddr GetActiveStripeTail(ASTailArrayIdx asTailArrayIdx);
    std::mutex& GetActiveStripeTailLock(ASTailArrayIdx asTailArrayIdx);
    void SetActiveStripeTail(ASTailArrayIdx asTailArrayIdx, VirtualBlkAddr vsa);
    int SetNextSsdLsid(void);
    SegmentId AllocateUserDataSegmentId(void);

    bool TurnOffVolumeBlkAllocation(uint32_t volumeId);
    void TurnOnVolumeBlkAllocation(uint32_t volumeId);
    void TurnOffBlkAllocation(void);
    void TurnOnBlkAllocation(void);

    bool IsblkAllocProhibited(uint32_t volumeId) { return blkAllocProhibited[volumeId] == true; }

    void ProhibitUserBlkAlloc(void) { userBlkAllocProhibited = true; }
    void PermitUserBlkAlloc(void) { userBlkAllocProhibited = false; }

    bool IsuserBlkAllocProhibited(void) { return userBlkAllocProhibited == true; }

    void Close(void);

    RawCtxInfo& GetRawCtxInfo(AllocatorCtxType type) { return ctxsList[type]; }
    BitMapMutex* GetWbLsidBitmap() { return wbLsidBitmap; }

private:
    void _UpdateCtxList(void);
    int _HeaderUpdate(void);

    void _LoadSync(void);
    void _CtxLoaded(char* buffer);
    int _HeaderLoaded(void);

    int _Flush(char* data, EventSmartPtr callback);
    void _PrepareCtxsStore(char* data);
    void _FlushCompletedThenCB(AsyncMetaFileIoCtx* ctx);

    bool _IsFirstStripeOfSegment(StripeId stripeId) { return (stripeId % addrInfo->GetstripesPerSegment() == 0); }
    char* _GetCopiedCtxBuffer(void);
    void _CopyWbufTail(char* data, int index);
    void _FillBuffer(char* buffer, int count);

    static const uint64_t INVALID_VERSION = UINT64_MAX;

    MetaFileIntf* ctxFile;

    std::atomic<bool> blkAllocProhibited[MAX_VOLUME_COUNT];
    std::atomic<bool> userBlkAllocProhibited;

    std::mutex ctxLock;
    std::atomic<uint64_t> ctxStoredVersion;
    std::atomic<uint64_t> ctxDirtyVersion;
    CtxHeader ctxHeader;                        // ctxLock
    RawCtxInfo ctxsList[NUM_ALLOCATOR_META];    // ctxLock

    // WriteBuffer
    //    Description about 'ASTailArrayIdx'
    //    Index  0  ... 255 are for Userdata IO of volume 0 ... 255
    //    Index 256 ... 511 are for GC IO of volume 0 ... 255
    VirtualBlkAddr activeStripeTail[ACTIVE_STRIPE_TAIL_ARRAYLEN];
    std::mutex activeStripeTailLock[ACTIVE_STRIPE_TAIL_ARRAYLEN];
    BitMapMutex* wbLsidBitmap;

    std::atomic<bool> flushInProgress;
    EventSmartPtr flushCallback;

    // DOCs
    AllocatorAddressInfo* addrInfo;
    RebuildCtx* rebuildCtx;
    SegmentCtx* segmentCtx;

    int numAsyncIoIssued;
    std::string arrayName;
};

} // namespace pos

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

#include <set>
#include <string>

#include "src/allocator/address/allocator_address_info.h"
#include "src/allocator/context_manager/i_allocator_file_io_client.h"
#include "src/allocator/context_manager/rebuild_ctx/rebuild_ctx.h"
#include "src/allocator/context_manager/gc_ctx/gc_ctx.h"
#include "src/allocator/context_manager/segment_ctx/segment_info.h"
#include "src/allocator/context_manager/segment_ctx/segment_list.h"
#include "src/allocator/i_segment_ctx.h"
#include "src/allocator/include/allocator_const.h"
#include "src/include/address_type.h"

namespace pos
{
class TelemetryPublisher;
class SegmentCtx : public IAllocatorFileIoClient, public ISegmentCtx
{
public:
    SegmentCtx(void) = default;
    SegmentCtx(TelemetryPublisher* tp_, SegmentCtxHeader* header, SegmentInfo* segmentInfo_,
        RebuildCtx* rebuildCtx_, AllocatorAddressInfo* addrInfo_, GcCtx* gcCtx_, int arrayId_);
    SegmentCtx(TelemetryPublisher* tp_, SegmentCtxHeader* header, SegmentInfo* segmentInfo_,
        SegmentList* rebuildSegmentList, RebuildCtx* rebuildCtx_, AllocatorAddressInfo* addrInfo_,
        GcCtx* gcCtx_, int arrayId_);
    explicit SegmentCtx(TelemetryPublisher* tp_, RebuildCtx* rebuildCtx_,
        AllocatorAddressInfo* info, GcCtx* gcCtx_, int arrayId_, SegmentInfo* segmentInfo_ = nullptr);
    virtual ~SegmentCtx(void);

    // Only for UT
    void SetSegmentList(SegmentState state, SegmentList* list);
    void SetRebuildList(SegmentList* list);

    virtual void Init(void);
    virtual void Dispose(void);

    virtual void AfterLoad(char* buf);
    virtual void BeforeFlush(char* buf);
    virtual std::mutex& GetCtxLock(void) { return segCtxLock; }
    virtual void FinalizeIo(AsyncMetaFileIoCtx* ctx);
    virtual char* GetSectionAddr(int section);
    virtual int GetSectionSize(int section);
    virtual uint64_t GetStoredVersion(void);
    virtual void ResetDirtyVersion(void);
    virtual std::string GetFilename(void);
    virtual uint32_t GetSignature(void);
    virtual int GetNumSections(void);

    virtual void MoveToFreeState(SegmentId segId);
    virtual uint32_t GetValidBlockCount(SegmentId segId);
    virtual int GetOccupiedStripeCount(SegmentId segId);
    virtual SegmentState GetSegmentState(SegmentId segId);
    virtual void ResetSegmentsStates(void);

    virtual void AllocateSegment(SegmentId segId);
    virtual SegmentId AllocateFreeSegment(void);

    virtual uint64_t GetNumOfFreeSegment(void);
    virtual uint64_t GetNumOfFreeSegmentWoLock(void);
    virtual int GetAllocatedSegmentCount(void);

    virtual SegmentId AllocateGCVictimSegment(void);

    virtual SegmentId GetRebuildTargetSegment(void);
    virtual int SetRebuildCompleted(SegmentId segId);
    virtual int MakeRebuildTarget(void);
    virtual std::set<SegmentId> GetNvramSegmentList(void);
    virtual std::set<SegmentId> GetVictimSegmentList(void);
    virtual int StopRebuilding(void);
    virtual uint32_t GetRebuildTargetSegmentCount(void);
    virtual std::set<SegmentId> GetRebuildSegmentList(void);
    virtual bool LoadRebuildList(void);

    virtual void CopySegmentInfoToBufferforWBT(WBTAllocatorMetaType type, char* dstBuf);
    virtual void CopySegmentInfoFromBufferforWBT(WBTAllocatorMetaType type, char* dstBuf);

    virtual void ValidateBlks(VirtualBlks blks) override;
    virtual bool InvalidateBlks(VirtualBlks blks, bool allowVictimSegRelease) override;
    virtual bool UpdateOccupiedStripeCount(StripeId lsid) override;

    virtual void ValidateBlocksWithGroupId(VirtualBlks blks, int logGroupId);
    virtual bool InvalidateBlocksWithGroupId(VirtualBlks blks, bool isForced, int logGroupId);
    virtual bool UpdateStripeCount(StripeId lsid, int logGroupId);

    virtual SegmentInfo* GetSegmentInfos(void);

    static const uint32_t SIG_SEGMENT_CTX = 0xAFAFAFAF;

private:
    void _SetOccupiedStripeCount(SegmentId segId, int count);
    void _GetUsedSegmentList(std::set<SegmentId>& segmentList);
    SegmentId _FindMostInvalidSSDSegment(void);
    void _SegmentFreed(SegmentId segId);

    void _RebuildSegmentList(void);
    void _BuildRebuildSegmentList(void);
    void _ResetSegmentIdInRebuilding(void);
    int _FlushRebuildSegmentList(void);
    bool _SetVictimSegment(SegmentId victimSegment);
    void _BuildRebuildSegmentListFromTheList(SegmentState state);
    void _UpdateTelemetryOnVictimSegmentAllocation(SegmentId victimSegment);

    void _IncreaseValidBlockCount(SegmentId segId, uint32_t cnt);
    bool _DecreaseValidBlockCount(SegmentId segId, uint32_t cnt, bool allowVictimSegRelease);
    bool _IncreaseOccupiedStripeCount(SegmentId segId);

    int _OnNumFreeSegmentChanged(void);

    SegmentCtxHeader ctxHeader;
    std::atomic<uint64_t> ctxDirtyVersion;
    std::atomic<uint64_t> ctxStoredVersion;

    SegmentInfo* segmentInfos;

    SegmentList* segmentList[SegmentState::NUM_STATES];
    SegmentList* rebuildList;
    SegmentId rebuildingSegment;

    bool initialized;

    AllocatorAddressInfo* addrInfo;

    std::mutex segCtxLock;

    RebuildCtx* rebuildCtx;
    GcCtx* gcCtx;
    TelemetryPublisher* tp;

    int arrayId;
    const int INVALID_SECTION_ID = -1;
};

} // namespace pos

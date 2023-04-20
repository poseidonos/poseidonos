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
#include "src/allocator/context_manager/context/context.h"
#include "src/allocator/context_manager/context/context_section.h"
#include "src/allocator/context_manager/gc_ctx/gc_ctx.h"
#include "src/allocator/context_manager/i_allocator_file_io_client.h"
#include "src/allocator/context_manager/rebuild_ctx/rebuild_ctx.h"
#include "src/allocator/context_manager/segment_ctx/segment_info.h"
#include "src/allocator/context_manager/segment_ctx/segment_list.h"
#include "src/allocator/context_manager/segment_ctx/segment_ctx_extended.h"
#include "src/allocator/i_segment_ctx.h"
#include "src/include/address_type.h"

namespace pos
{
class EventScheduler;
class ISegmentFreeSubscriber;
class TelemetryPublisher;
class SegmentCtx : public IAllocatorFileIoClient, public ISegmentCtx
{
public:
    SegmentCtx(void) = default;
    SegmentCtx(TelemetryPublisher* tp_, SegmentCtxHeader* header, SegmentInfoData* segmentInfoData_,
        RebuildCtx* rebuildCtx_, AllocatorAddressInfo* addrInfo_, GcCtx* gcCtx_);
    SegmentCtx(TelemetryPublisher* tp_, SegmentCtxHeader* header, SegmentInfoData* segmentInfoData_,
        SegmentList* rebuildSegmentList, RebuildCtx* rebuildCtx_, AllocatorAddressInfo* addrInfo_,
        GcCtx* gcCtx_);
    explicit SegmentCtx(TelemetryPublisher* tp_, RebuildCtx* rebuildCtx_,
        AllocatorAddressInfo* info, GcCtx* gcCtx_, SegmentInfoData* segmentInfoData_ = nullptr);
    virtual ~SegmentCtx(void);

    // Only for Test
    void SetEventScheduler(EventScheduler* eventScheduler_);
    void SetSegmentList(SegmentState state, SegmentList* list);
    void SetRebuildList(SegmentList* list);

    virtual void Init(EventScheduler* eventScheduler_);
    virtual void Dispose(void);

    virtual void AfterLoad(char* buf) override;
    virtual void BeforeFlush(char* buf, ContextSectionBuffer externalBuf = INVALID_CONTEXT_SECTION_BUFFER) override;
    virtual void AfterFlush(char* buf) override;
    virtual ContextSectionAddr GetSectionInfo(int section) override;
    virtual uint64_t GetStoredVersion(void) override;
    virtual void ResetDirtyVersion(void) override;
    virtual int GetNumSections(void) override;
    virtual uint64_t GetTotalDataSize(void) override;

    virtual bool MoveToFreeState(SegmentId segId);
    virtual int GetValidBlockCount(SegmentId segId);
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
    virtual uint32_t GetVictimSegmentCount(void);
    virtual int StopRebuilding(void);
    virtual uint32_t GetRebuildTargetSegmentCount(void);
    virtual std::set<SegmentId> GetRebuildSegmentList(void);
    virtual bool LoadRebuildList(void);

    virtual void CopySegmentInfoToBufferforWBT(WBTAllocatorMetaType type, char* dstBuf);
    virtual void CopySegmentInfoFromBufferforWBT(WBTAllocatorMetaType type, char* dstBuf);

    virtual void ValidateBlks(VirtualBlks blks) override;
    virtual bool InvalidateBlks(VirtualBlks blks, bool allowVictimSegRelease) override;
    virtual bool UpdateOccupiedStripeCount(StripeId lsid) override;
    virtual void AddSegmentFreeSubscriber(ISegmentFreeSubscriber* subscriber) override;

    virtual void ReplayBlockInvalidated(VirtualBlks blks, bool allowVictimSegRelease);
    virtual void ReplayStripeFlushed(StripeId userLsid);

    virtual SegmentInfoData* GetSegmentInfoDataArray(void);
    virtual void SegmentFreeUpdateCompleted(SegmentId segmentId, int logGroupId);

protected:
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

    void _IncreaseValidBlockCount(SegmentId segId, int cnt);
    bool _DecreaseValidBlockCount(SegmentId segId, int cnt, bool allowVictimSegRelease);
    bool _IncreaseOccupiedStripeCount(SegmentId segId);

    int _OnNumFreeSegmentChanged(void);

    void _UpdateSectionInfo(void);
    void _NotifySubscribersOfSegmentFreed(SegmentId segmentId, int logGroupId);

    // Data to be stored: Section 1
    ContextSection<SegmentCtxHeader> ctxHeader;

    // Data to be stored: Section 2 ~ N
    std::vector<ContextSection<SegmentInfoData*>> segmentInfoDataSections;

    // Data to be stored: Section N+1
    ContextSection<SegmentCtxExtended*> ctxExtended;

    uint64_t totalDataSize; // SectionSize(1) + SectionSize(2) + ... + SectionSize(N) + SectionSize(N+1)

    // In-memory data structures
    std::atomic<uint64_t> ctxDirtyVersion;
    std::atomic<uint64_t> ctxStoredVersion;

    SegmentInfo* segmentInfos;

    SegmentList* segmentList[SegmentState::NUM_STATES];
    SegmentList* rebuildList;
    SegmentId rebuildingSegment;

    std::mutex segCtxLock;

    bool initialized;
    bool segmentStateRebuilt;

    // Dependencies
    AllocatorAddressInfo* addrInfo;

    RebuildCtx* rebuildCtx;
    GcCtx* gcCtx;
    TelemetryPublisher* tp;
    std::vector<ISegmentFreeSubscriber*> segmentFreedSubscribers;
    EventScheduler* eventScheduler;
};

} // namespace pos

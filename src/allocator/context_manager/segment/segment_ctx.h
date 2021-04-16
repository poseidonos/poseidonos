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

#include "src/allocator/include/allocator_const.h"
#include "src/allocator/address/allocator_address_info.h"
#include "src/allocator/context_manager/segment/segment_info.h"
#include "src/allocator/context_manager/segment/segment_states.h"
#include "src/allocator/i_segment_ctx.h"
#include "src/include/address_type.h"
#include "src/lib/bitmap.h"
#include "src/meta_file_intf/async_context.h"

namespace pos
{
class MetaFileIntf;
class AllocatorContextIoCtx;

class SegmentCtx : public ISegmentCtx
{
    struct SegInfoHeader
    {
        uint32_t segInfoSig;
        uint32_t segInfoVersion;
    };

public:
    explicit SegmentCtx(AllocatorAddressInfo* info, std::string arrayName);
    virtual ~SegmentCtx(void);
    void Init(void);
    void Close(void);

    uint32_t GetGcThreshold(void) override { return thresholdSegments;}
    uint32_t GetUrgentThreshold(void) override { return urgentSegments;}
    SegmentId GetGCVictimSegment(void) override;
    uint64_t GetNumOfFreeUserDataSegment(void) override;
    void FreeUserDataSegment(SegmentId segId) override;
    void ReplaySsdLsid(StripeId currentSsdLsid) override;
    void ReplaySegmentAllocation(StripeId userLsid) override;
    void UpdateOccupiedStripeCount(StripeId lsid) override;
    void SetGcThreshold(uint32_t inputThreshold) { thresholdSegments = inputThreshold;}
    void SetUrgentThreshold(uint32_t inputThreshold) { urgentSegments = inputThreshold;}

    StripeId GetPrevSsdLsid(void);
    void SetPrevSsdLsid(StripeId stripeId);
    StripeId GetCurrentSsdLsid(void);
    void SetCurrentSsdLsid(StripeId stripeId);
    SegmentStates& GetSegmentState(SegmentId segmentId);
    void UsedSegmentStateChange(SegmentId segmentId, SegmentState state);
    BitMapMutex* GetSegmentBitmap(void) { return segmentBitmap;}

    uint32_t IncreaseValidBlockCount(SegmentId segId, uint32_t cnt);
    int32_t DecreaseValidBlockCount(SegmentId segId, uint32_t cnt);
    uint32_t GetValidBlockCount(SegmentId segId);

    uint32_t GetNumSegment(void);
    int StoreSegmentInfoSync(void);
    AllocatorContextIoCtx* StoreSegmentInfoAsync(MetaIoCbPtr cb);
    void ReleaseRequestIo(AsyncMetaFileIoCtx* ctx);
    bool IsSegmentInfoRequestIo(char* pBuf);
    bool CheckSegmentState(SegmentId segmentId, SegmentState state);

    SegmentInfo* GetSegmentInfo(void) { return segmentInfos;}
    void ResetExVictimSegment(void);
    char* GetCtxSectionInfo(AllocatorCtxType type, int& sectionSize);

private:
    int _LoadSegmentInfoSync(void);
    void _HeaderUpdate(char* pBuf);
    void _HeaderLoaded(char* pBuf);
    void _FreeSegment(SegmentId segId);

    static const int SIG_SEGMENT_INFO = 0x46495347; // "SGIF"

    // Segment
    BitMapMutex* segmentBitmap; // Unset:Free, Set:Not-Free
    StripeId prevSsdLsid;       // allocatorMetaLock
    StripeId currentSsdLsid;    // allocatorMetaLock
    SegmentStates* segmentStates;
    SegmentInfo* segmentInfos;

    // context file info
    uint32_t versionSegInfo;
    uint32_t headerSize;
    uint32_t writeBufferSize;

    // gc threshold
    uint32_t thresholdSegments;
    uint32_t urgentSegments;

    // DOCs
    AllocatorAddressInfo* addrInfo;
    MetaFileIntf* segInfoFile;

    // array
    std::string arrayName;
};

} // namespace pos

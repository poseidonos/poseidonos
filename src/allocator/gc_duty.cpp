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

#include "src/logger/logger.h"

#include "gc_duty.h"
#include "segment_info.h"

namespace ibofos
{
GcDuty::GcDuty(AllocatorAddressInfo& addrInfoI, AllocatorMetaArchive* metaI)
: thresholdSegments(20),
  urgentSegments(5),
  blockSegmentAllocForUser(false),
  addrInfo(addrInfoI),
  meta(metaI)
{
}

GcDuty::~GcDuty(void)
{
}

uint32_t
GcDuty::GetNumOfFreeUserDataSegment(void)
{
    return meta->segmentBitmap->GetNumBits() - meta->segmentBitmap->GetNumBitsSet();
}

SegmentId
GcDuty::GetMostInvalidatedSegment(void)
{
    uint32_t numUserAreaSegments = addrInfo.GetnumUserAreaSegments();
    uint32_t upperLimit = addrInfo.GetblksPerSegment();
    SegmentId mostInvalidatedSegmentId = UNMAP_SEGMENT;
    uint32_t mostInvalidCnt = 0;

    for (SegmentId id = 0; id < numUserAreaSegments; ++id)
    {
        SegmentInfo& segmentInfo = meta->GetSegmentInfo(id);
        uint32_t cnt = segmentInfo.GetinValidBlockCount();

        if (segmentInfo.Getstate() != SegmentState::SSD)
        {
            continue;
        }

        if (cnt == upperLimit)
        {
            mostInvalidatedSegmentId = id;
            break;
        }

        if (cnt > mostInvalidCnt)
        {
            mostInvalidatedSegmentId = id;
            mostInvalidCnt = cnt;
        }
    }

    return mostInvalidatedSegmentId;
}

void
GcDuty::FreeUserDataSegmentId(SegmentId segId)
{
    assert(addrInfo.GetblksPerSegment() == meta->GetSegmentInfo(segId).GetinValidBlockCount());
    meta->GetSegmentInfo(segId).FreeSegment();
    meta->segmentBitmap->ClearBit(segId);
    _FreeRebuildTargetSegment(segId);
}

void
GcDuty::_FreeRebuildTargetSegment(SegmentId segmentId)
{
    if (meta->IsRebuidTargetSegmentsEmpty())
    {
        return;
    }

    std::unique_lock<std::mutex> lock(meta->GetallocatorMetaLock());
    auto iter = meta->FindRebuildTargetSegment(segmentId);
    if (iter == meta->RebuildTargetSegmentsEnd())
    {
        return;
    }
    meta->EraseRebuildTargetSegments(iter);
    IBOF_TRACE_INFO(EID(ALLOCATOR_TARGET_SEGMENT_FREED), "segmentId:{} in Rebuild Target Freed", segmentId);
    meta->StoreRebuildSegment();
}

} // namespace ibofos

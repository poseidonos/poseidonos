/*
 *   BSD LICENSE
 *   Copyright (c) 2022 Samsung Electronics Corporation
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

#include <atomic>

#include "src/allocator/i_segment_ctx.h"
#include "src/include/address_type.h"
#include "tbb/concurrent_unordered_map.h"
#include "tbb/atomic.h"

namespace pos
{
class IArrayInfo;
class VersionedSegmentInfo : public ISegmentCtx
{
public:
    VersionedSegmentInfo(void) = default;
    VersionedSegmentInfo(uint32_t stripesPerSegment);
    virtual ~VersionedSegmentInfo(void);

    virtual void Reset(void);
    virtual void IncreaseValidBlockCount(SegmentId segId, uint32_t cnt);
    virtual void DecreaseValidBlockCount(SegmentId segId, uint32_t cnt);
    virtual void IncreaseOccupiedStripeCount(SegmentId segId);
    virtual void ResetOccupiedStripeCount(SegmentId segId);

    // This will be used in volume deletion
    virtual void ValidateBlks(VirtualBlks blks) override;
    virtual bool InvalidateBlks(VirtualBlks blks, bool isForced) override;
    virtual bool UpdateOccupiedStripeCount(StripeId lsid);

    virtual const tbb::concurrent_unordered_map<SegmentId, tbb::atomic<int>>&  GetChangedValidBlockCount(void);
    virtual const tbb::concurrent_unordered_map<SegmentId, tbb::atomic<int>>&  GetChangedOccupiedStripeCount(void);

private:
    IArrayInfo* arrayInfo;
    tbb::concurrent_unordered_map<SegmentId, tbb::atomic<int>> changedValidBlockCount;
    tbb::concurrent_unordered_map<SegmentId, tbb::atomic<int>> changedOccupiedStripeCount;

    inline SegmentId _StripeIdToSegmentId(StripeId stripeId)
    {
        return stripeId / numStripesPerSegment;
    }

    uint32_t numStripesPerSegment;
};

} // namespace pos

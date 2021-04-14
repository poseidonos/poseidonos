#pragma once

#include "gmock/gmock.h"
#include "src/allocator/i_segment_ctx.h"

namespace pos
{
class SegmentCtxMock : public ISegmentCtx
{
public:
    SegmentCtxMock(void) {}
    virtual ~SegmentCtxMock(void) {}

    MOCK_METHOD(void, ReplaySsdLsid,
        (StripeId userLsid), (override));
    MOCK_METHOD(void, ReplaySegmentAllocation,
        (StripeId userLsid), (override));
    MOCK_METHOD(void, UpdateOccupiedStripeCount,
        (StripeId userLsid), (override));

    virtual uint32_t GetGcThreshold(void) override { return 0; }
    virtual uint32_t GetUrgentThreshold(void) override { return 0; }
    virtual SegmentId GetGCVictimSegment(void) override { return 0; }
    virtual uint64_t GetNumOfFreeUserDataSegment(void) override { return 0; }
    virtual void FreeUserDataSegment(SegmentId segId) override {}
};

} // namespace pos

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/context_manager/segment/segment_ctx.h"

namespace pos
{
class MockSegmentCtx : public SegmentCtx
{
public:
    using SegmentCtx::SegmentCtx;
    MOCK_METHOD(uint32_t, GetGcThreshold, (), (override));
    MOCK_METHOD(uint32_t, GetUrgentThreshold, (), (override));
    MOCK_METHOD(SegmentId, GetGCVictimSegment, (), (override));
    MOCK_METHOD(uint64_t, GetNumOfFreeUserDataSegment, (), (override));
    MOCK_METHOD(void, ReplaySsdLsid, (StripeId currentSsdLsid), (override));
    MOCK_METHOD(void, ReplaySegmentAllocation, (StripeId userLsid), (override));
    MOCK_METHOD(void, UpdateOccupiedStripeCount, (StripeId lsid), (override));
    MOCK_METHOD(void, FreeAllInvalidatedSegment, (), (override));
};

} // namespace pos

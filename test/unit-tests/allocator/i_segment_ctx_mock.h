#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/i_segment_ctx.h"

namespace pos
{
class MockISegmentCtx : public ISegmentCtx
{
public:
    using ISegmentCtx::ISegmentCtx;
    MOCK_METHOD(void, ValidateBlks, (VirtualBlks blks), (override));
    MOCK_METHOD(bool, InvalidateBlks, (VirtualBlks blks, bool isForced), (override));
    MOCK_METHOD(bool, UpdateOccupiedStripeCount, (StripeId lsid), (override));
    MOCK_METHOD(void, AddSegmentFreeSubscriber, (ISegmentFreeSubscriber * subscriber), (override));
};

} // namespace pos

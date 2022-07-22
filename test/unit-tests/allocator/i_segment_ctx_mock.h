#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/allocator/i_segment_ctx.h"

namespace pos
{
class MockISegmentCtx : public ISegmentCtx
{
public:
    using ISegmentCtx::ISegmentCtx;
    MOCK_METHOD(void, ValidateBlks, (VirtualBlks blks, int logGroupId), (override));
    MOCK_METHOD(bool, InvalidateBlks, (VirtualBlks blks, bool allowVictimSegRelease, int logGroupId), (override));
    MOCK_METHOD(bool, UpdateOccupiedStripeCount, (StripeId lsid, int logGroupId), (override));
};

} // namespace pos

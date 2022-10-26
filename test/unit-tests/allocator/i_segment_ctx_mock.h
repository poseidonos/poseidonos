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
    MOCK_METHOD(void, ValidateBlks, (VirtualBlks blks), (override));
    MOCK_METHOD(bool, InvalidateBlks, (VirtualBlks blks, bool allowVictimSegRelease), (override));
    MOCK_METHOD(bool, UpdateOccupiedStripeCount, (StripeId lsid), (override));
    MOCK_METHOD(void, ValidateBlocksWithGroupId, (VirtualBlks blks, int logGroupId), (override));
    MOCK_METHOD(bool, InvalidateBlocksWithGroupId, (VirtualBlks blks, bool isForced, int logGroupId), (override));
    MOCK_METHOD(bool, UpdateStripeCount, (StripeId lsid, int logGroupId), (override));
};

} // namespace pos

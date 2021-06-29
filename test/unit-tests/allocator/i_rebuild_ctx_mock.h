#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/i_rebuild_ctx.h"

namespace pos
{
class MockIRebuildCtx : public IRebuildCtx
{
public:
    using IRebuildCtx::IRebuildCtx;
    MOCK_METHOD(SegmentId, GetRebuildTargetSegment, (), (override));
    MOCK_METHOD(int, ReleaseRebuildSegment, (SegmentId segmentId), (override));
    MOCK_METHOD(bool, NeedRebuildAgain, (), (override));
};

} // namespace pos

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/context_manager/rebuild/rebuild_ctx.h"

namespace pos
{
class MockRebuildCtx : public RebuildCtx
{
public:
    using RebuildCtx::RebuildCtx;
    MOCK_METHOD(SegmentId, GetRebuildTargetSegment, (), (override));
    MOCK_METHOD(int, ReleaseRebuildSegment, (SegmentId segmentId), (override));
    MOCK_METHOD(bool, NeedRebuildAgain, (), (override));
};

} // namespace pos

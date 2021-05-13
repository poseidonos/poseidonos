#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/context_manager/rebuild_ctx/rebuild_ctx.h"

namespace pos
{
class MockRebuildCtx : public RebuildCtx
{
public:
    using RebuildCtx::RebuildCtx;
    MOCK_METHOD(void, Init, (AllocatorAddressInfo * info), (override));
    MOCK_METHOD(void, Close, (), (override));
    MOCK_METHOD(SegmentId, GetRebuildTargetSegment, (), (override));
    MOCK_METHOD(bool, IsRebuildTargetSegment, (SegmentId segId), (override));
};

} // namespace pos

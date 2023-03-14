#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/context_manager/segment_ctx/segment_ctx_extended.h"

namespace pos
{
class MockSegmentCtxExtended : public SegmentCtxExtended
{
public:
    using SegmentCtxExtended::SegmentCtxExtended;
};

} // namespace pos

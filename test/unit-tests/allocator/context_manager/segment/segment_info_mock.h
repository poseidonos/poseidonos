#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/context_manager/segment/segment_info.h"

namespace pos
{
class MockSegmentInfo : public SegmentInfo
{
public:
    using SegmentInfo::SegmentInfo;
    MOCK_METHOD(uint32_t, GetValidBlockCount, (SegmentId segId), (override));
};

} // namespace pos

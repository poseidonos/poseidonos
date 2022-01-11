#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/allocator/context_manager/segment_ctx/segment_list.h"

namespace pos
{
class MockSegmentList : public SegmentList
{
public:
    using SegmentList::SegmentList;
    MOCK_METHOD(void, Reset, (), (override));
    MOCK_METHOD(SegmentId, PopSegment, (), (override));
    MOCK_METHOD(void, AddToList, (SegmentId segId), (override));
    MOCK_METHOD(uint32_t, GetNumSegments, (), (override));
    MOCK_METHOD(uint32_t, GetNumSegmentsWoLock, (), (override));
};

} // namespace pos

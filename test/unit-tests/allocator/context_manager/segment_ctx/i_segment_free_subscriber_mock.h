#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/context_manager/segment_ctx/i_segment_free_subscriber.h"

namespace pos
{
class MockISegmentFreeSubscriber : public ISegmentFreeSubscriber
{
public:
    using ISegmentFreeSubscriber::ISegmentFreeSubscriber;
    MOCK_METHOD(void, NotifySegmentFreed, (SegmentId segmentId, int logGroupId), (override));
};

} // namespace pos

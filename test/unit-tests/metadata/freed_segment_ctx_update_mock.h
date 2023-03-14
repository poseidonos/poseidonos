#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metadata/freed_segment_ctx_update.h"

namespace pos
{
class MockFreedSegmentCtxUpdateEvent : public FreedSegmentCtxUpdateEvent
{
public:
    using FreedSegmentCtxUpdateEvent::FreedSegmentCtxUpdateEvent;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos

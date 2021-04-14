#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/unit_test/map_flush_done_event.h"

namespace pos
{
class MockMapFlushDoneEvent : public MapFlushDoneEvent
{
public:
    using MapFlushDoneEvent::MapFlushDoneEvent;
    MOCK_METHOD(bool, Execute, (), (override));
};

} // namespace pos

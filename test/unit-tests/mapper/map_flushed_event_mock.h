#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/map_flushed_event.h"

namespace pos
{
class MockMapFlushedEvent : public MapFlushedEvent
{
public:
    using MapFlushedEvent::MapFlushedEvent;
    MOCK_METHOD(bool, Execute, (), (override));
};

} // namespace pos

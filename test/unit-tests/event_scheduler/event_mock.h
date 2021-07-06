#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/event_scheduler/event.h"

namespace pos
{
class MockEvent : public Event
{
public:
    using Event::Event;
    MOCK_METHOD(BackendEvent, GetEventType, (), (override));
    MOCK_METHOD(void, SetEventType, (BackendEvent), (override));
    MOCK_METHOD(bool, Execute, (), (override));
    MOCK_METHOD(bool, IsFrontEnd, (), (override));
};

} // namespace pos

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/event_scheduler/event_factory.h"

namespace pos
{
class MockEventFactory : public EventFactory
{
public:
    using EventFactory::EventFactory;
    MOCK_METHOD(EventSmartPtr, Create, (UbioSmartPtr ubio), (override));
};

} // namespace pos

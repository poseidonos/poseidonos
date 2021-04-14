#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/event/stripe_put_event.h"

namespace pos
{
class MockStripePutEvent : public StripePutEvent
{
public:
    using StripePutEvent::StripePutEvent;
    MOCK_METHOD(bool, Execute, (), (override));
};

} // namespace pos

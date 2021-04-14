#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/state/array_state_publisher.h"

namespace pos
{
class MockArrayStatePublisher : public ArrayStatePublisher
{
public:
    using ArrayStatePublisher::ArrayStatePublisher;
    MOCK_METHOD(void, Register, (IArrayStateSubscriber * subscriber), (override));
    MOCK_METHOD(void, Unregister, (IArrayStateSubscriber * subscriber), (override));
};

} // namespace pos

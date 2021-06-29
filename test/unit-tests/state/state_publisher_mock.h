#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/state/state_publisher.h"

namespace pos
{
class MockStatePublisher : public StatePublisher
{
public:
    using StatePublisher::StatePublisher;
    MOCK_METHOD(void, Add, (IStateObserver * ob, string name), (override));
    MOCK_METHOD(void, Remove, (IStateObserver * ob), (override));
    MOCK_METHOD(void, Notify, (StateContext * prev, StateContext* next), (override));
};

} // namespace pos

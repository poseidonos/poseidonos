#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/state/interface/i_state_observer.h"

namespace pos
{
class MockIStateObserver : public IStateObserver
{
public:
    using IStateObserver::IStateObserver;
    MOCK_METHOD(void, StateChanged, (StateContext * prev, StateContext* next), (override));
};

} // namespace pos

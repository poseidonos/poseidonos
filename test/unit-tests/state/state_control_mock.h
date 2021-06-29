#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/state/state_control.h"

namespace pos
{
class MockStateControl : public StateControl
{
public:
    using StateControl::StateControl;
    MOCK_METHOD(void, Subscribe, (IStateObserver * sub, string name), (override));
    MOCK_METHOD(void, Unsubscribe, (IStateObserver * sub), (override));
    MOCK_METHOD(StateContext*, GetState, (), (override));
    MOCK_METHOD(void, Invoke, (StateContext * ctx), (override));
    MOCK_METHOD(void, Remove, (StateContext * ctx), (override));
    MOCK_METHOD(bool, Exists, (SituationEnum situ), (override));
};

} // namespace pos

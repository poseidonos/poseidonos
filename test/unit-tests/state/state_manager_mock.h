#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/state/state_manager.h"

namespace pos
{
class MockStateManager : public StateManager
{
public:
    using StateManager::StateManager;
    MOCK_METHOD(IStateControl*, CreateStateControl, (string array), (override));
    MOCK_METHOD(IStateControl*, GetStateControl, (string array), (override));
    MOCK_METHOD(void, RemoveStateControl, (string array), (override));
};

} // namespace pos

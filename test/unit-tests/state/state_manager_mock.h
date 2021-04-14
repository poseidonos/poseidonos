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
};

} // namespace pos

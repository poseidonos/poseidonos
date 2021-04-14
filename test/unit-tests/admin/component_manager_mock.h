#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/admin/component_manager.h"

namespace pos
{
class MockComponentManager : public ComponentManager
{
public:
    using ComponentManager::ComponentManager;
};

} // namespace pos

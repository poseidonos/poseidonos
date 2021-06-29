#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cpu_affinity/affinity_manager.h"

namespace pos
{
class MockAffinityManager : public AffinityManager
{
public:
    using AffinityManager::AffinityManager;
};

} // namespace pos

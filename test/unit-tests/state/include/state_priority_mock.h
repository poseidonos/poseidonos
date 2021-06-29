#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/state/include/state_priority.h"

namespace pos
{
class MockStatePriority : public StatePriority
{
public:
    using StatePriority::StatePriority;
};

} // namespace pos

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/rebuild/rebuild_completed.h"

namespace pos
{
class MockRebuildCompleted : public RebuildCompleted
{
public:
    using RebuildCompleted::RebuildCompleted;
    MOCK_METHOD(bool, Execute, (), (override));
};

} // namespace pos

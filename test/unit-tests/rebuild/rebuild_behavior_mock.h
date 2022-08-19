#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/rebuild/rebuild_behavior.h"

namespace pos
{
class MockRebuildBehavior : public RebuildBehavior
{
public:
    using RebuildBehavior::RebuildBehavior;
    MOCK_METHOD(bool, Rebuild, (), (override));
    MOCK_METHOD(void, UpdateProgress, (uint32_t val), (override));
};

} // namespace pos

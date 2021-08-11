#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include <memory>
#include "src/rebuild/rebuild_behavior_factory.h"

namespace pos
{
class MockRebuildBehaviorFactory : public RebuildBehaviorFactory
{
public:
    using RebuildBehaviorFactory::RebuildBehaviorFactory;
    MOCK_METHOD(RebuildBehavior*, CreateRebuildBehavior, (unique_ptr<RebuildContext> ctx), (override));
};

} // namespace pos

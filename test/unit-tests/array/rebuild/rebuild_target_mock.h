#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/rebuild/rebuild_target.h"

namespace pos
{
class MockRebuildTarget : public RebuildTarget
{
public:
    using RebuildTarget::RebuildTarget;
    MOCK_METHOD(unique_ptr<RebuildContext>, GetRebuildCtx, (ArrayDevice * fault), (override));
};

} // namespace pos

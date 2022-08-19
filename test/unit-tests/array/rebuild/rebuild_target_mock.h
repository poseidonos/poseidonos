#include <gmock/gmock.h>
#include <vector>
#include <utility>
#include <memory>
#include "src/array/rebuild/rebuild_target.h"

namespace pos
{
class MockRebuildTarget : public RebuildTarget
{
public:
    using RebuildTarget::RebuildTarget;
    MOCK_METHOD(unique_ptr<RebuildContext>, GetRebuildCtx, (const vector<IArrayDevice*>& fault), (override));
    MOCK_METHOD(unique_ptr<RebuildContext>, GetQuickRebuildCtx, (const QuickRebuildPair& rebuildPair), (override));
};

} // namespace pos

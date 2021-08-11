#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/array/rebuild/rebuild_result.h"

namespace pos
{
class MockRebuildResult : public RebuildResult
{
public:
    using RebuildResult::RebuildResult;
};

} // namespace pos

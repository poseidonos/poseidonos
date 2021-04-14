#include <gmock/gmock.h>

#include <list>
#include <string>
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

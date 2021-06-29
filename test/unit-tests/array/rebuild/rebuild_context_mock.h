#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/rebuild/rebuild_context.h"

namespace pos
{
class MockRebuildContext : public RebuildContext
{
public:
    using RebuildContext::RebuildContext;
};

} // namespace pos

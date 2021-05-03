#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/context_manager/rebuild/rebuild_ctx.h"

namespace pos
{
class MockRebuildCtx : public RebuildCtx
{
public:
    using RebuildCtx::RebuildCtx;
};

} // namespace pos

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/context_manager/gc_ctx.h"

namespace pos
{
class MockGcCtx : public GcCtx
{
public:
    using GcCtx::GcCtx;
};

} // namespace pos

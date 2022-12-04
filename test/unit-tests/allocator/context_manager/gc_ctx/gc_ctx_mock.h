#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/allocator/context_manager/gc_ctx/gc_ctx.h"

namespace pos
{
class MockGcCtx : public GcCtx
{
public:
    using GcCtx::GcCtx;
    MOCK_METHOD(GcMode, GetCurrentGcMode, (), (override));
    MOCK_METHOD(GcMode, UpdateCurrentGcMode, (uint32_t numFreeSegments), (override));
};
} // namespace pos

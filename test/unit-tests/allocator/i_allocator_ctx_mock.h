#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/i_allocator_ctx.h"

namespace pos
{
class MockIAllocatorCtx : public IAllocatorCtx
{
public:
    using IAllocatorCtx::IAllocatorCtx;
    MOCK_METHOD(int, FlushAllocatorCtxs, (EventSmartPtr callback), (override));
    MOCK_METHOD(int, StoreAllocatorCtxs, (), (override));
    MOCK_METHOD(uint64_t, GetAllocatorCtxsStoredVersion, (), (override));
    MOCK_METHOD(void, ResetAllocatorCtxsDirtyVersion, (), (override));
};

} // namespace pos

#include "src/event_scheduler/event.h"
#include "allocator_ctx_mock.h"

namespace pos
{
AllocatorCtxMock::AllocatorCtxMock(void)
{
    ON_CALL(*this, FlushAllocatorCtxs).WillByDefault(::testing::Invoke(this,
        &AllocatorCtxMock::_FlushAllocatorCtxs));
}

AllocatorCtxMock::~AllocatorCtxMock(void)
{
}

int
AllocatorCtxMock::_FlushAllocatorCtxs(EventSmartPtr callback)
{
    bool result = callback->Execute();
    if (result == true)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

} // namespace pos

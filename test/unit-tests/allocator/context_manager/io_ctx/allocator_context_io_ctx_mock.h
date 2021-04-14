#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/context_manager/io_ctx/allocator_context_io_ctx.h"

namespace pos
{
class MockAllocatorContextIoCtx : public AllocatorContextIoCtx
{
public:
    using AllocatorContextIoCtx::AllocatorContextIoCtx;
};

} // namespace pos

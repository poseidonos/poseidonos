#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/context_manager/io_ctx/allocator_io_ctx.h"

namespace pos
{
class MockAllocatorIoCtx : public AllocatorIoCtx
{
public:
    using AllocatorIoCtx::AllocatorIoCtx;
};

} // namespace pos

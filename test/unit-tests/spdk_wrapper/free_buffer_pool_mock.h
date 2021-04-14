#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/spdk_wrapper/free_buffer_pool.h"

namespace pos
{
class MockFreeBufferPool : public FreeBufferPool
{
public:
    using FreeBufferPool::FreeBufferPool;
};

} // namespace pos

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/block_manager/block_allocator_stub.h"

namespace pos
{
class MockStubBlockAllocator : public StubBlockAllocator
{
public:
    using StubBlockAllocator::StubBlockAllocator;
};

} // namespace pos

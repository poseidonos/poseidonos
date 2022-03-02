#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/i_block_allocator.h"

namespace pos
{
class MockIBlockAllocator : public IBlockAllocator
{
public:
    using IBlockAllocator::IBlockAllocator;
    MOCK_METHOD(VirtualBlks, AllocateWriteBufferBlks, (uint32_t volumeId, uint32_t numBlks), (override));
    MOCK_METHOD(Stripe*, AllocateGcDestStripe, (uint32_t volumeId), (override));
    MOCK_METHOD(void, ProhibitUserBlkAlloc, (), (override));
    MOCK_METHOD(void, PermitUserBlkAlloc, (), (override));
    MOCK_METHOD(bool, BlockAllocating, (uint32_t volumeId), (override));
    MOCK_METHOD(void, UnblockAllocating, (uint32_t volumeId), (override));
};

} // namespace pos

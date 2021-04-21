#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/block_manager/block_manager.h"

namespace pos
{
class MockBlockManager : public BlockManager
{
public:
    using BlockManager::BlockManager;
    MOCK_METHOD(VirtualBlks, AllocateWriteBufferBlks, (uint32_t volumeId, uint32_t numBlks, bool forGC), (override));
    MOCK_METHOD(void, InvalidateBlks, (VirtualBlks blks), (override));
    MOCK_METHOD(void, ValidateBlks, (VirtualBlks blks), (override));
    MOCK_METHOD(void, ProhibitUserBlkAlloc, (), (override));
    MOCK_METHOD(void, PermitUserBlkAlloc, (), (override));
    MOCK_METHOD(bool, BlockAllocating, (uint32_t volumeId), (override));
    MOCK_METHOD(void, UnblockAllocating, (uint32_t volumeId), (override));
};

} // namespace pos

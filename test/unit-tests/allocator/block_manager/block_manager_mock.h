#include <gmock/gmock.h>

#include <list>
#include <string>
#include <utility>
#include <vector>

#include "src/allocator/block_manager/block_manager.h"

namespace pos
{
class MockBlockManager : public BlockManager
{
public:
    using BlockManager::BlockManager;
    MOCK_METHOD(void, Init, (IWBStripeAllocator* iwbstripeAllocator), (override));
    MOCK_METHOD((std::pair<VirtualBlks, StripeId>), AllocateWriteBufferBlks, (uint32_t volumeId, uint32_t numBlks), (override));
    MOCK_METHOD(Stripe*, AllocateGcDestStripe, (uint32_t volumeId), (override));
    MOCK_METHOD(void, ProhibitUserBlkAlloc, (), (override));
    MOCK_METHOD(bool, IsProhibitedUserBlkAlloc, (), (override));
    MOCK_METHOD(void, PermitUserBlkAlloc, (), (override));
    MOCK_METHOD(bool, BlockAllocating, (uint32_t volumeId), (override));
    MOCK_METHOD(void, UnblockAllocating, (uint32_t volumeId), (override));
    MOCK_METHOD(void, TurnOffBlkAllocation, (), (override));
    MOCK_METHOD(void, TurnOnBlkAllocation, (), (override));
    MOCK_METHOD(bool, TryRdLock, (uint32_t volumeId), (override));
    MOCK_METHOD(bool, Unlock, (uint32_t volumeId), (override));
    MOCK_METHOD(void, Lock, (), (override));
    MOCK_METHOD(void, Unlock, (), (override));
};

} // namespace pos

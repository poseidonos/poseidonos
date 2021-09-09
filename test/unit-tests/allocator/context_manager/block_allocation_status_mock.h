#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/allocator/context_manager/block_allocation_status.h"

namespace pos
{
class MockBlockAllocationStatus : public BlockAllocationStatus
{
public:
    using BlockAllocationStatus::BlockAllocationStatus;
    MOCK_METHOD(bool, IsUserBlockAllocationProhibited, (int volumeId), (override));
    MOCK_METHOD(bool, IsBlockAllocationProhibited, (int volumeId), (override));
    MOCK_METHOD(void, PermitUserBlockAllocation, (), (override));
    MOCK_METHOD(void, PermitBlockAllocation, (), (override));
    MOCK_METHOD(void, PermitBlockAllocation, (int volumeId), (override));
    MOCK_METHOD(void, ProhibitUserBlockAllocation, (), (override));
    MOCK_METHOD(void, ProhibitBlockAllocation, (), (override));
    MOCK_METHOD(bool, TryProhibitBlockAllocation, (int volumeId), (override));
};

} // namespace pos

#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/array/partition/partition_manager.h"

namespace pos
{
class MockPartitionManager : public PartitionManager
{
public:
    using PartitionManager::PartitionManager;
    MOCK_METHOD(int, Import, (vector<Partition*> parts, IPartitionServices* const svc), (override));
    MOCK_METHOD(const PartitionLogicalSize*, GetSizeInfo, (PartitionType type), (override));
    MOCK_METHOD(const PartitionPhysicalSize*, GetPhysicalSize, (PartitionType type), (override));
    MOCK_METHOD(void, DeletePartitions, (), (override));
    MOCK_METHOD(void, FormatPartition, (PartitionType type, uint32_t arrayId, IODispatcher* io), (override));
    MOCK_METHOD(RaidState, GetRaidState, (), (override));
    MOCK_METHOD(RaidTypeEnum, GetRaidType, (PartitionType type), (override));
    MOCK_METHOD(const vector<Partition*>, GetPartitions, (), (override));
};

} // namespace pos

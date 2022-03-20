#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/partition/partition_manager.h"

namespace pos
{
class MockPartitionManager : public PartitionManager
{
public:
    using PartitionManager::PartitionManager;
    MOCK_METHOD(const PartitionLogicalSize*, GetSizeInfo, (PartitionType type), (override));
    MOCK_METHOD(const PartitionPhysicalSize*, GetPhysicalSize, (PartitionType type), (override));
    MOCK_METHOD(int, CreatePartitions, (ArrayDevice* nvm, vector<ArrayDevice*> data,
        RaidTypeEnum metaRaid, RaidTypeEnum dataRaid, IPartitionServices* svc), (override));
    MOCK_METHOD(void, DeletePartitions, (), (override));
    MOCK_METHOD(void, FormatPartition, (PartitionType type, uint32_t arrayId, IODispatcher* io), (override));
    MOCK_METHOD(RaidState, GetRaidState, (), (override));
    MOCK_METHOD(RaidTypeEnum, GetRaidType, (PartitionType), (override));
};

} // namespace pos

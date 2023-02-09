#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/array_models/interface/i_array_info.h"

namespace pos
{
class MockIArrayInfo : public IArrayInfo
{
public:
    using IArrayInfo::IArrayInfo;
    MOCK_METHOD(string, GetName, (), (override));
    MOCK_METHOD(string, GetUniqueId, (), (override));
    MOCK_METHOD(uint32_t, GetIndex, (), (override));
    MOCK_METHOD(string, GetCreateDatetime, (), (override));
    MOCK_METHOD(string, GetUpdateDatetime, (), (override));
    MOCK_METHOD(string, GetMetaRaidType, (), (override));
    MOCK_METHOD(string, GetDataRaidType, (), (override));
    MOCK_METHOD(ArrayStateType, GetState, (), (override));
    MOCK_METHOD(StateContext*, GetStateCtx, (), (override));
    MOCK_METHOD(uint32_t, GetRebuildingProgress, (), (override));
    MOCK_METHOD(bool, IsWriteThroughEnabled, (), (override));
    MOCK_METHOD(vector<IArrayDevice*>, GetDevices, (ArrayDeviceType type), (override));
    MOCK_METHOD(const PartitionLogicalSize*, GetSizeInfo, (PartitionType type), (override));
};

} // namespace pos

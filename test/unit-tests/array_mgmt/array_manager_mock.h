#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array_mgmt/array_manager.h"

namespace pos
{
class MockArrayManager : public ArrayManager
{
public:
    using ArrayManager::ArrayManager;
    MOCK_METHOD(int, Create, (string name, DeviceSet<string> devs, string metaFt, string dataFt), (override));
    MOCK_METHOD(int, Delete, (string name), (override));
    MOCK_METHOD(int, Mount, (string name, bool isWTEnabled), (override));
    MOCK_METHOD(int, Unmount, (string name), (override));
    MOCK_METHOD(int, AddDevice, (string name, string dev), (override));
    MOCK_METHOD(int, RemoveDevice, (string name, string dev), (override));
    MOCK_METHOD(void, SetTargetAddress, (string name, string targetAddress), (override));
    MOCK_METHOD(string, GetTargetAddress, (string name), (override));
    MOCK_METHOD(ComponentsInfo*, GetInfo, (string name), (override));
    MOCK_METHOD(ComponentsInfo*, GetInfo, (uint32_t arrayIdx), (override));
    MOCK_METHOD(int, DeviceDetached, (UblockSharedPtr dev), (override));
    MOCK_METHOD(void, DeviceAttached, (UblockSharedPtr dev), (override));
    MOCK_METHOD(int, PrepareRebuild, (string name, bool& resume), (override));
    MOCK_METHOD(void, RebuildDone, (string name), (override));
    MOCK_METHOD(int, Load, (list<string> & failedArrayList), (override));
    MOCK_METHOD(int, ResetMbr, (), (override));
};

} // namespace pos

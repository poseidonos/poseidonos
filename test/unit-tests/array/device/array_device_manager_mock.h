#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/device/array_device_manager.h"

namespace pos
{
class MockArrayDeviceManager : public ArrayDeviceManager
{
public:
    using ArrayDeviceManager::ArrayDeviceManager;
    MOCK_METHOD(int, ImportByName, (DeviceSet<string> nameSet), (override));
    MOCK_METHOD(int, Import, (DeviceSet<DeviceMeta> metaSet), (override));
    MOCK_METHOD(DeviceSet<ArrayDevice*>&, Export, (), (override));
    MOCK_METHOD(DeviceSet<string>, ExportToName, (), (override));
    MOCK_METHOD(DeviceSet<DeviceMeta>, ExportToMeta, (), (override));
    MOCK_METHOD(void, Clear, (), (override));
    MOCK_METHOD(int, AddSpare, (string devName), (override));
    MOCK_METHOD(int, RemoveSpare, (string devName), (override));
    MOCK_METHOD(int, ReplaceWithSpare, (ArrayDevice* target, ArrayDevice*& out), (override));
    MOCK_METHOD((tuple<ArrayDevice*, ArrayDeviceType>), GetDev, (UblockSharedPtr uBlock), (override));
    MOCK_METHOD((tuple<ArrayDevice*, ArrayDeviceType>), GetDevBySn, (string sn), (override));
    MOCK_METHOD((tuple<ArrayDevice*, ArrayDeviceType>), GetDevByName, (string devName), (override));
    MOCK_METHOD(vector<IArrayDevice*>, GetFaulty, (), (override));
    MOCK_METHOD(vector<IArrayDevice*>, GetRebuilding, (), (override));
    MOCK_METHOD(vector<ArrayDevice*>, GetDataDevices, (), (override));
};

} // namespace pos

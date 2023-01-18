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
    MOCK_METHOD(void, Clear, (), (override));
    MOCK_METHOD(int, AddSpare, (string devName), (override));
    MOCK_METHOD(int, RemoveSpare, (string devName), (override));
    MOCK_METHOD(int, ReplaceWithSpare, (ArrayDevice* target, ArrayDevice*& out), (override));
    MOCK_METHOD(vector<ArrayDevice*>, GetDevs, (), (override));
    MOCK_METHOD(vector<ArrayDevice*>, GetFaulty, (), (override));
    MOCK_METHOD(vector<ArrayDevice*>, GetRebuilding, (), (override));
    MOCK_METHOD(vector<ArrayDevice*>, GetDataDevices, (), (override));
    MOCK_METHOD(vector<ArrayDevice*>, GetSpareDevices, (), (override));
    MOCK_METHOD(vector<ArrayDevice*>, GetAvailableSpareDevices, (), (override));
};

} // namespace pos

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
    MOCK_METHOD(void, Clear, (), (override));
    MOCK_METHOD(int, Import, (vector<ArrayDevice*> devs), (override));
    MOCK_METHOD(int, AddSpare, (string devName), (override));
    MOCK_METHOD(int, RemoveSpare, (string devName), (override));
    MOCK_METHOD(int, ReplaceWithSpare, (ArrayDevice* target, ArrayDevice*& swapOut), (override));
    MOCK_METHOD(vector<ArrayDevice*>&, GetDevs, (), (override));
};
} // namespace pos

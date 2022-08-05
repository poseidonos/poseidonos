#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/device/array_device_list.h"

namespace pos
{
class MockArrayDeviceList : public ArrayDeviceList
{
public:
    using ArrayDeviceList::ArrayDeviceList;
    MOCK_METHOD(ArrayDeviceType, Exists, (string devName), (override));
    MOCK_METHOD(int, SetNvm, (ArrayDevice * nvm), (override));
    MOCK_METHOD(int, AddData, (ArrayDevice * dev), (override));
    MOCK_METHOD(int, AddSpare, (ArrayDevice * dev), (override));
    MOCK_METHOD(int, RemoveSpare, (ArrayDevice * target), (override));
    MOCK_METHOD(int, SpareToData, (ArrayDevice * target, ArrayDevice*& swapOut), (override));
    MOCK_METHOD(void, Clear, (), (override));
    MOCK_METHOD(DeviceSet<ArrayDevice*>&, GetDevs, (), (override));
    MOCK_METHOD(DeviceSet<string>, ExportNames, (), (override));
};

} // namespace pos

#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/array/device/array_device_list.h"

namespace pos
{
class MockArrayDeviceList : public ArrayDeviceList
{
public:
    using ArrayDeviceList::ArrayDeviceList;
    MOCK_METHOD(int, Import, (vector<ArrayDevice*> devs), (override));
    MOCK_METHOD(int, AddSsd, (ArrayDevice* dev), (override));
    MOCK_METHOD(int, RemoveSsd, (ArrayDevice* target), (override));
    MOCK_METHOD(int, SpareToData, (ArrayDevice* target, ArrayDevice*& swapOut), (override));
    MOCK_METHOD(void, Clear, (), (override));
    MOCK_METHOD(vector<ArrayDevice*>&, GetDevs, (), (override));
    virtual ~MockArrayDeviceList() {}
};

} // namespace pos

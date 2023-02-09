#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/device/array_device.h"

namespace pos
{
class MockArrayDevice : public ArrayDevice
{
public:
    using ArrayDevice::ArrayDevice;
    MOCK_METHOD(string, GetName, (), (override));
    MOCK_METHOD(string, GetSerial, (), (override));
    MOCK_METHOD(uint32_t, GetDataIndex, (), (override));
    MOCK_METHOD(uint64_t, GetSize, (), (override));
    MOCK_METHOD(ArrayDeviceType, GetType, (), (override));
    MOCK_METHOD(ArrayDeviceState, GetState, (), (override));
    MOCK_METHOD(UblockSharedPtr, GetUblock, (), (override));
    MOCK_METHOD(UBlockDevice*, GetUblockPtr, (), (override));
    MOCK_METHOD(void, SetState, (ArrayDeviceState state), (override));
    MOCK_METHOD(void, SetUblock, (UblockSharedPtr uBlock), (override));
    ~MockArrayDevice() {}
};

} // namespace pos

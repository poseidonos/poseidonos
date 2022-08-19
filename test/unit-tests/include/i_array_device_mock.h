#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/include/i_array_device.h"

namespace pos
{
class MockIArrayDevice : public IArrayDevice
{
public:
    using IArrayDevice::IArrayDevice;
    MOCK_METHOD(ArrayDeviceState, GetState, (), (override));
    MOCK_METHOD(void, SetState, (ArrayDeviceState state), (override));
    MOCK_METHOD(UblockSharedPtr, GetUblock, (), (override));
    MOCK_METHOD(UBlockDevice*, GetUblockPtr, (), (override));
    MOCK_METHOD(void, SetUblock, (UblockSharedPtr uBlock), (override));
    MOCK_METHOD(string, GetName, (), (override));
    MOCK_METHOD(string, GetSerial, (), (override));
    MOCK_METHOD(uint32_t, GetDataIndex, (), (override));
};
} // namespace pos

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/device/base/device_driver.h"

namespace pos
{
class MockDeviceDriver : public DeviceDriver
{
public:
    using DeviceDriver::DeviceDriver;
    MOCK_METHOD(int, ScanDevs, (std::vector<UblockSharedPtr> * devs), (override));
    MOCK_METHOD(bool, Open, (DeviceContext * deviceContext), (override));
    MOCK_METHOD(bool, Close, (DeviceContext * deviceContext), (override));
    MOCK_METHOD(int, SubmitAsyncIO, (DeviceContext * deviceContext, UbioSmartPtr bio), (override));
    MOCK_METHOD(int, CompleteIOs, (DeviceContext * deviceContext), (override));
    MOCK_METHOD(int, CompleteErrors, (DeviceContext * deviceContext), (override));
};

} // namespace pos

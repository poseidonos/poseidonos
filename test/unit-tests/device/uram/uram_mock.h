#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/device/uram/uram.h"

namespace pos
{
class MockUramBdev : public UramBdev
{
public:
    using UramBdev::UramBdev;
    MOCK_METHOD(int, SubmitAsyncIO, (UbioSmartPtr ubio), (override));
    MOCK_METHOD(DeviceContext*, _AllocateDeviceContext, (), (override));
    MOCK_METHOD(void, _ReleaseDeviceContext, (DeviceContext * deviceContextToRelease), (override));
    MOCK_METHOD(bool, _WrapupOpenDeviceSpecific, (DeviceContext * deviceContext), (override));
};

} // namespace pos

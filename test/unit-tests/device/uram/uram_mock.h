#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/device/uram/uram.h"

namespace pos
{
class MockUram : public Uram
{
public:
    using Uram::Uram;
    MOCK_METHOD(int, SubmitAsyncIO, (UbioSmartPtr ubio), (override));
    MOCK_METHOD(DeviceContext*, _AllocateDeviceContext, (), (override));
    MOCK_METHOD(void, _ReleaseDeviceContext, (DeviceContext * deviceContextToRelease), (override));
    MOCK_METHOD(bool, WrapupOpenDeviceSpecific, (), (override));
};

} // namespace pos

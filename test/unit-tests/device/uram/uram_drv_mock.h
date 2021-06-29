#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/device/uram/uram_drv.h"

namespace pos
{
class MockUramDrv : public UramDrv
{
public:
    using UramDrv::UramDrv;
    MOCK_METHOD(int, ScanDevs, (std::vector<UblockSharedPtr> * devs), (override));
    MOCK_METHOD(bool, Open, (DeviceContext * deviceContext), (override));
    MOCK_METHOD(bool, Close, (DeviceContext * deviceContext), (override));
    MOCK_METHOD(int, CompleteIOs, (DeviceContext * deviceContext), (override));
    MOCK_METHOD(int, SubmitAsyncIO, (DeviceContext * deviceContext, UbioSmartPtr bio), (override));
};

} // namespace pos

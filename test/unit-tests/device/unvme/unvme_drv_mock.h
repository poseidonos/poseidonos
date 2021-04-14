#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/device/unvme/unvme_drv.h"

namespace pos
{
class MockUnvmeDrv : public UnvmeDrv
{
public:
    using UnvmeDrv::UnvmeDrv;
    MOCK_METHOD(int, ScanDevs, (std::vector<UblockSharedPtr> * devs), (override));
    MOCK_METHOD(bool, Open, (DeviceContext * deviceContext), (override));
    MOCK_METHOD(bool, Close, (DeviceContext * deviceContext), (override));
    MOCK_METHOD(int, CompleteIOs, (DeviceContext * deviceContext), (override));
    MOCK_METHOD(int, CompleteErrors, (DeviceContext * deviceContext), (override));
    MOCK_METHOD(int, SubmitAsyncIO, (DeviceContext * deviceContext, UbioSmartPtr bio), (override));
};

} // namespace pos

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/device/unvme/unvme_ssd.h"

namespace pos
{
class MockUnvmeSsd : public UnvmeSsd
{
public:
    using UnvmeSsd::UnvmeSsd;
    MOCK_METHOD(DeviceContext*, _AllocateDeviceContext, (), (override));
    MOCK_METHOD(void, _ReleaseDeviceContext, (DeviceContext * deviceContextToRelease), (override));
};

} // namespace pos

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/service/io_device_checker/i_device_checker.h"

namespace pos
{
class MockIDeviceChecker : public IDeviceChecker
{
public:
    using IDeviceChecker::IDeviceChecker;
    MOCK_METHOD(int, IsRecoverable, (IArrayDevice * target, UBlockDevice* uBlock), (override));
    MOCK_METHOD(IArrayDevice*, FindDevice, (string devSn), (override));
};

} // namespace pos

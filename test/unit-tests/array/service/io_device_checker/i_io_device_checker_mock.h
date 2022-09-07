#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/service/io_device_checker/i_io_device_checker.h"

namespace pos
{
class MockIIODeviceChecker : public IIODeviceChecker
{
public:
    using IIODeviceChecker::IIODeviceChecker;
    MOCK_METHOD(int, IsRecoverable, (string array, IArrayDevice* target, UBlockDevice* uBlock), (override));
    MOCK_METHOD(IArrayDevice*, FindDevice, (string array, string devSn), (override));
};

} // namespace pos

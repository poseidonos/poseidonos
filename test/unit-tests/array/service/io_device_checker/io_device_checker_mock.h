#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/service/io_device_checker/io_device_checker.h"

namespace pos
{
class MockIODeviceChecker : public IODeviceChecker
{
public:
    using IODeviceChecker::IODeviceChecker;
    MOCK_METHOD(bool, IsRecoverable, (string array, IArrayDevice* target, UBlockDevice* uBlock), (override));
    MOCK_METHOD(IArrayDevice*, FindDevice, (string array, string devSn), (override));
};

} // namespace pos

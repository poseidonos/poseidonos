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
    MOCK_METHOD(int, IsRecoverable, (unsigned int arrayIndex, IArrayDevice* target, UBlockDevice* uBlock), (override));
    MOCK_METHOD(IArrayDevice*, FindDevice, (unsigned int arrayIndex, string devSn), (override));
};

} // namespace pos

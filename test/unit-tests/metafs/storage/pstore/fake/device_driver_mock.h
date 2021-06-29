#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/storage/pstore/fake/device_driver.h"

namespace pos
{
class MockDeviceDriver : public DeviceDriver
{
public:
    using DeviceDriver::DeviceDriver;
};

} // namespace pos

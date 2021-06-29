#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/storage/pstore/mock/mock_driver.h"

namespace pos
{
class MockMockDeviceDriver : public MockDeviceDriver
{
public:
    using MockDeviceDriver::MockDeviceDriver;
};

} // namespace pos

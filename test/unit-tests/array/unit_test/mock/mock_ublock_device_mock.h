#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/unit_test/mock/mock_ublock_device.h"

class MockMockUBlockDevice : public MockUBlockDevice
{
public:
    using MockUBlockDevice::MockUBlockDevice;
};

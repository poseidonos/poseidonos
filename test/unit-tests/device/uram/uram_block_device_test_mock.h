#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/device/uram/uram_block_device_test.h"

namespace pos
{
class MockUramBlockDeviceTest : public UramBlockDeviceTest
{
public:
    using UramBlockDeviceTest::UramBlockDeviceTest;
    MOCK_METHOD(uint32_t, _SubmitIO, (UbioSmartPtr ioToSubmit, uint32_t threadIndex), (override));
    MOCK_METHOD(uint32_t, _CheckIOCompletion, (UblockSharedPtr dev, uint32_t threadIndex), (override));
};

} // namespace pos

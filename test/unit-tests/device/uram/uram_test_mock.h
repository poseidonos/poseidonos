#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/device/uram/uram_test.h"

namespace pos
{
class MockUramDrvTest : public UramDrvTest
{
public:
    using UramDrvTest::UramDrvTest;
    MOCK_METHOD(void, _PrintDevice, (), (override));
    MOCK_METHOD(bool, _TestDeviceContextInitialized, (), (override));
    MOCK_METHOD(uint32_t, _SubmitIO, (UbioSmartPtr ioToSubmit), (override));
    MOCK_METHOD(uint32_t, _CheckIOCompletion, (), (override));
};

} // namespace pos

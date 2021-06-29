#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/device/base/ublock_device_submission_adapter.h"

namespace pos
{
class MockUBlockDeviceSubmissionAdapter : public UBlockDeviceSubmissionAdapter
{
public:
    using UBlockDeviceSubmissionAdapter::UBlockDeviceSubmissionAdapter;
    MOCK_METHOD(int, Do, (UbioSmartPtr bio), (override));
};

} // namespace pos

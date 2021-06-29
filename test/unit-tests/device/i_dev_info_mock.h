#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/device/i_dev_info.h"

namespace pos
{
class MockIDevInfo : public IDevInfo
{
public:
    using IDevInfo::IDevInfo;
    MOCK_METHOD(UblockSharedPtr, GetDev, (DeviceIdentifier & devID), (override));
};

} // namespace pos

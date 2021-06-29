#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/device/i_array_device_manager.h"

namespace pos
{
class MockIArrayDevMgr : public IArrayDevMgr
{
public:
    using IArrayDevMgr::IArrayDevMgr;
    MOCK_METHOD((tuple<ArrayDevice*, ArrayDeviceType>), GetDev, (UblockSharedPtr uBlock), (override));
};

} // namespace pos

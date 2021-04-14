#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/device/i_device_event.h"

namespace pos
{
class MockIDeviceEvent : public IDeviceEvent
{
public:
    using IDeviceEvent::IDeviceEvent;
    MOCK_METHOD(int, DeviceDetached, (UblockSharedPtr dev), (override));
};

} // namespace pos

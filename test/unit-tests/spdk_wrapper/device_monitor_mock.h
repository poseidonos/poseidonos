#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/spdk_wrapper/device_monitor.h"

namespace pos
{
class MockDeviceMonitor : public DeviceMonitor
{
public:
    using DeviceMonitor::DeviceMonitor;
    MOCK_METHOD(void, Start, (), (override));
    MOCK_METHOD(void, Stop, (), (override));
    MOCK_METHOD(void, Pause, (), (override));
    MOCK_METHOD(bool, IsPaused, (), (override));
    MOCK_METHOD(void, Resume, (), (override));
};

} // namespace pos

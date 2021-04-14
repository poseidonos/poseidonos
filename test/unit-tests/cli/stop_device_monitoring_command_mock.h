#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/stop_device_monitoring_command.h"

namespace pos_cli
{
class MockStopDeviceMonitoringCommand : public StopDeviceMonitoringCommand
{
public:
    using StopDeviceMonitoringCommand::StopDeviceMonitoringCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/start_device_monitoring_command.h"

namespace pos_cli
{
class MockStartDeviceMonitoringCommand : public StartDeviceMonitoringCommand
{
public:
    using StartDeviceMonitoringCommand::StartDeviceMonitoringCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

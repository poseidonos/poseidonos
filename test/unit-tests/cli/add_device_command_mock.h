#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/add_device_command.h"

namespace pos_cli
{
class MockAddDeviceCommand : public AddDeviceCommand
{
public:
    using AddDeviceCommand::AddDeviceCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

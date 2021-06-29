#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/list_device_command.h"

namespace pos_cli
{
class MockListDeviceCommand : public ListDeviceCommand
{
public:
    using ListDeviceCommand::ListDeviceCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

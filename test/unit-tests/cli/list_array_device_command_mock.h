#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/list_array_device_command.h"

namespace pos_cli
{
class MockListArrayDeviceCommand : public ListArrayDeviceCommand
{
public:
    using ListArrayDeviceCommand::ListArrayDeviceCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

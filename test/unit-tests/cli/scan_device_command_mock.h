#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/scan_device_command.h"

namespace pos_cli
{
class MockScanDeviceCommand : public ScanDeviceCommand
{
public:
    using ScanDeviceCommand::ScanDeviceCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

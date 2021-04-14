#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/create_volume_command.h"

namespace pos_cli
{
class MockCreateVolumeCommand : public CreateVolumeCommand
{
public:
    using CreateVolumeCommand::CreateVolumeCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

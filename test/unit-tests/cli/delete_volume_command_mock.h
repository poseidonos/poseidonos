#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/delete_volume_command.h"

namespace pos_cli
{
class MockDeleteVolumeCommand : public DeleteVolumeCommand
{
public:
    using DeleteVolumeCommand::DeleteVolumeCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

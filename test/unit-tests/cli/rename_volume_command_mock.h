#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/rename_volume_command.h"

namespace pos_cli
{
class MockRenameVolumeCommand : public RenameVolumeCommand
{
public:
    using RenameVolumeCommand::RenameVolumeCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

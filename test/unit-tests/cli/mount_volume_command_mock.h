#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/mount_volume_command.h"

namespace pos_cli
{
class MockMountVolumeCommand : public MountVolumeCommand
{
public:
    using MountVolumeCommand::MountVolumeCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/unmount_volume_command.h"

namespace pos_cli
{
class MockUnmountVolumeCommand : public UnmountVolumeCommand
{
public:
    using UnmountVolumeCommand::UnmountVolumeCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

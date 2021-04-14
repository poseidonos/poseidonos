#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/mount_ibofos_command.h"

namespace pos_cli
{
class MockMountIbofosCommand : public MountIbofosCommand
{
public:
    using MountIbofosCommand::MountIbofosCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

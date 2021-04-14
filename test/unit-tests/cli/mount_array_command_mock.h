#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/mount_array_command.h"

namespace pos_cli
{
class MockMountArrayCommand : public MountArrayCommand
{
public:
    using MountArrayCommand::MountArrayCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

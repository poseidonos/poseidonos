#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/unmount_array_command.h"

namespace pos_cli
{
class MockUnmountArrayCommand : public UnmountArrayCommand
{
public:
    using UnmountArrayCommand::UnmountArrayCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

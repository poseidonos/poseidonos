#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/unmount_ibofos_command.h"

namespace pos_cli
{
class MockUnmountIbofosCommand : public UnmountIbofosCommand
{
public:
    using UnmountIbofosCommand::UnmountIbofosCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/exit_ibofos_command.h"

namespace pos_cli
{
class MockExitIbofosCommand : public ExitIbofosCommand
{
public:
    using ExitIbofosCommand::ExitIbofosCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

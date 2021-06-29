#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/set_log_level_command.h"

namespace pos_cli
{
class MockSetLogLevelCommand : public SetLogLevelCommand
{
public:
    using SetLogLevelCommand::SetLogLevelCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/get_log_level_command.h"

namespace pos_cli
{
class MockGetLogLevelCommand : public GetLogLevelCommand
{
public:
    using GetLogLevelCommand::GetLogLevelCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

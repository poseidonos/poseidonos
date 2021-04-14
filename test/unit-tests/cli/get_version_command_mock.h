#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/get_version_command.h"

namespace pos_cli
{
class MockGetVersionCommand : public GetVersionCommand
{
public:
    using GetVersionCommand::GetVersionCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

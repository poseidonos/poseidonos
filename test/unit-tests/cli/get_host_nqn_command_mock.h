#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/get_host_nqn_command.h"

namespace pos_cli
{
class MockGetHostNqnCommand : public GetHostNqnCommand
{
public:
    using GetHostNqnCommand::GetHostNqnCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

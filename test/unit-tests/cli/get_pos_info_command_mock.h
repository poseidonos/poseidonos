#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/get_pos_info_command.h"

namespace pos_cli
{
class MockGetPosInfoCommand : public GetPosInfoCommand
{
public:
    using GetPosInfoCommand::GetPosInfoCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

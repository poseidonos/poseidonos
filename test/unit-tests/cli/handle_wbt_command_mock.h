#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/handle_wbt_command.h"

namespace pos_cli
{
class MockHandleWbtCommand : public HandleWbtCommand
{
public:
    using HandleWbtCommand::HandleWbtCommand;
    MOCK_METHOD(std::string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

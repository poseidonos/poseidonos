#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/list_array_command.h"

namespace pos_cli
{
class MockListArrayCommand : public ListArrayCommand
{
public:
    using ListArrayCommand::ListArrayCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/list_wbt_command.h"

namespace pos_cli
{
class MockListWbtCommand : public ListWbtCommand
{
public:
    using ListWbtCommand::ListWbtCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

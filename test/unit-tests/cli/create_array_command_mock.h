#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/create_array_command.h"

namespace pos_cli
{
class MockCreateArrayCommand : public CreateArrayCommand
{
public:
    using CreateArrayCommand::CreateArrayCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

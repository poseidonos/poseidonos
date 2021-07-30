#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/cli/create_numa_awared_array_command.h"

namespace pos_cli
{
class MockCreateNumaAwaredArrayCommand : public CreateNumaAwaredArrayCommand
{
public:
    using CreateNumaAwaredArrayCommand::CreateNumaAwaredArrayCommand;
    MOCK_METHOD(string, Execute, (json& doc, string rid), (override));
};

} // namespace pos_cli

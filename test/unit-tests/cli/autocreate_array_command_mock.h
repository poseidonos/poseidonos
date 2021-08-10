#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/cli/autocreate_array_command.h"

namespace pos_cli
{
class MockCreateNumaAwaredArrayCommand : public AutocreateArrayCommand
{
public:
    using AutocreateArrayCommand::AutocreateArrayCommand;
    MOCK_METHOD(string, Execute, (json& doc, string rid), (override));
};

} // namespace pos_cli

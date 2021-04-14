#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/array_info_command.h"

namespace pos_cli
{
class MockArrayInfoCommand : public ArrayInfoCommand
{
public:
    using ArrayInfoCommand::ArrayInfoCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

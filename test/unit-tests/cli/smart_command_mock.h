#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/smart_command.h"

namespace pos_cli
{
class MockSmartCommand : public SmartCommand
{
public:
    using SmartCommand::SmartCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

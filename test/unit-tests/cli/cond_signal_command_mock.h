#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/cond_signal_command.h"

namespace pos_cli
{
class MockCondSignalCommand : public CondSignalCommand
{
public:
    using CondSignalCommand::CondSignalCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

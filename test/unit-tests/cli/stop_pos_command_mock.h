#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/stop_pos_command.h"

namespace pos_cli
{
class MockStopPosCommand : public StopPosCommand
{
public:
    using StopPosCommand::StopPosCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

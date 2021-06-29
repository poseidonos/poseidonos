#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/stop_rebuilding_command.h"

namespace pos_cli
{
class MockStopRebuildingCommand : public StopRebuildingCommand
{
public:
    using StopRebuildingCommand::StopRebuildingCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

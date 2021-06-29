#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/list_volume_command.h"

namespace pos_cli
{
class MockListVolumeCommand : public ListVolumeCommand
{
public:
    using ListVolumeCommand::ListVolumeCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

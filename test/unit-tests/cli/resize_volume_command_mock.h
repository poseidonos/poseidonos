#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/resize_volume_command.h"

namespace pos_cli
{
class MockResizeVolumeCommand : public ResizeVolumeCommand
{
public:
    using ResizeVolumeCommand::ResizeVolumeCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

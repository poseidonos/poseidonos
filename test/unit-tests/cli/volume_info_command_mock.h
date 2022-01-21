#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/volume_info_command.h"

namespace pos_cli
{
class VolumeInfoCommand : public VolumeInfoCommand
{
public:
    using VolumeInfoCommand::VolumeInfoCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

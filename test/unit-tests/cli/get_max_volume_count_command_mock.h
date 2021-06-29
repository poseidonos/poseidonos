#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/get_max_volume_count_command.h"

namespace pos_cli
{
class MockGetMaxVolumeCountCommand : public GetMaxVolumeCountCommand
{
public:
    using GetMaxVolumeCountCommand::GetMaxVolumeCountCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

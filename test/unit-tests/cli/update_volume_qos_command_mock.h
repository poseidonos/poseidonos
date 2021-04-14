#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/update_volume_qos_command.h"

namespace pos_cli
{
class MockUpdateVolumeQosCommand : public UpdateVolumeQosCommand
{
public:
    using UpdateVolumeQosCommand::UpdateVolumeQosCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

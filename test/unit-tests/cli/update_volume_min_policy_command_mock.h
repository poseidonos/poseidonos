#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/update_volume_min_policy_command.h"

namespace pos_cli
{
class MockUpdateVolumeMinPolicyCommand : public UpdateVolumeMinPolicyCommand
{
public:
    using UpdateVolumeMinPolicyCommand::UpdateVolumeMinPolicyCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

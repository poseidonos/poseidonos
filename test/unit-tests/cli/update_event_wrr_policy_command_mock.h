#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/update_event_wrr_policy_command.h"

namespace pos_cli
{
class MockUpdateEventWrrPolicyCommand : public UpdateEventWrrPolicyCommand
{
public:
    using UpdateEventWrrPolicyCommand::UpdateEventWrrPolicyCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/reset_event_wrr_policy_command.h"

namespace pos_cli
{
class MockResetEventWrrPolicyCommand : public ResetEventWrrPolicyCommand
{
public:
    using ResetEventWrrPolicyCommand::ResetEventWrrPolicyCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

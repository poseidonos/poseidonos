#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/reset_mbr_command.h"

namespace pos_cli
{
class MockResetMbrCommand : public ResetMbrCommand
{
public:
    using ResetMbrCommand::ResetMbrCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/apply_log_filter_command.h"

namespace pos_cli
{
class MockApplyLogFilterCommand : public ApplyLogFilterCommand
{
public:
    using ApplyLogFilterCommand::ApplyLogFilterCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

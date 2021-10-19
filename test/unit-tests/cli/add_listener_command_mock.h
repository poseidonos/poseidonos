#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/apply_log_filter_command.h"

namespace pos_cli
{
class MockApplyLogFilterCommand : public AddListenerCommand
{
public:
    using AddListenerCommand::AddListenerCommand;
};

} // namespace pos_cli

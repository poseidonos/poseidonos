#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/smart_log_command.h"

namespace pos_cli
{
class MockSMARTLOGCommand : public SMARTLOGCommand
{
public:
    using SMARTLOGCommand::SMARTLOGCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

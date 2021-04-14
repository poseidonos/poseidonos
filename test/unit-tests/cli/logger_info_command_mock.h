#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/logger_info_command.h"

namespace pos_cli
{
class MockLoggerInfoCommand : public LoggerInfoCommand
{
public:
    using LoggerInfoCommand::LoggerInfoCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/general_io/command_timeout_handler.h"

namespace pos
{
class MockCommandTimeoutHandler : public CommandTimeoutHandler
{
public:
    using CommandTimeoutHandler::CommandTimeoutHandler;
};

} // namespace pos

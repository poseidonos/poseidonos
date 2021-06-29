#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/admin/admin_command_handler.h"

namespace pos
{
class MockAdminCommandHandler : public AdminCommandHandler
{
public:
    using AdminCommandHandler::AdminCommandHandler;
};

} // namespace pos

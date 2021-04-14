#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/admin/admin_command_complete_handler.h"

namespace pos
{
class MockAdminCommandCompleteHandler : public AdminCommandCompleteHandler
{
public:
    using AdminCommandCompleteHandler::AdminCommandCompleteHandler;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos

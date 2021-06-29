#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/wbt_cmd_handler.h"

namespace pos
{
class MockWbtCmdHandler : public WbtCmdHandler
{
public:
    using WbtCmdHandler::WbtCmdHandler;
};

} // namespace pos

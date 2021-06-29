#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/wbt_metafs_cmd_handler.h"

namespace pos
{
class MockWbtMetafsCmdHandler : public WbtMetafsCmdHandler
{
public:
    using WbtMetafsCmdHandler::WbtMetafsCmdHandler;
};

} // namespace pos

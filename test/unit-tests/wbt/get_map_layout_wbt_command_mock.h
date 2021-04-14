#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/get_map_layout_wbt_command.h"

namespace pos
{
class MockGetMapLayoutWbtCommand : public GetMapLayoutWbtCommand
{
public:
    using GetMapLayoutWbtCommand::GetMapLayoutWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos

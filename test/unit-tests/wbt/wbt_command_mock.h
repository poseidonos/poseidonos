#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/wbt_command.h"

namespace pos
{
class MockWbtCommand : public WbtCommand
{
public:
    using WbtCommand::WbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos

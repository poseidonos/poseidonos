#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/get_current_ssd_lsid_wbt_command.h"

namespace pos
{
class MockGetCurrentSsdLsidWbtCommand : public GetCurrentSsdLsidWbtCommand
{
public:
    using GetCurrentSsdLsidWbtCommand::GetCurrentSsdLsidWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/set_current_ssd_lsid_wbt_command.h"

namespace pos
{
class MockSetCurrentSsdLsidWbtCommand : public SetCurrentSsdLsidWbtCommand
{
public:
    using SetCurrentSsdLsidWbtCommand::SetCurrentSsdLsidWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos

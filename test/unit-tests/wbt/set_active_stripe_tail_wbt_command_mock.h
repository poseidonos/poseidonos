#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/set_active_stripe_tail_wbt_command.h"

namespace pos
{
class MockSetActiveStripeTailWbtCommand : public SetActiveStripeTailWbtCommand
{
public:
    using SetActiveStripeTailWbtCommand::SetActiveStripeTailWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos

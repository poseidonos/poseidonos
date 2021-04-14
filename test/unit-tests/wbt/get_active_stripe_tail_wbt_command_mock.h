#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/get_active_stripe_tail_wbt_command.h"

namespace pos
{
class MockGetActiveStripeTailWbtCommand : public GetActiveStripeTailWbtCommand
{
public:
    using GetActiveStripeTailWbtCommand::GetActiveStripeTailWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos

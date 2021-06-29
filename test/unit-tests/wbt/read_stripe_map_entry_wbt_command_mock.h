#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/read_stripe_map_entry_wbt_command.h"

namespace pos
{
class MockReadStripeMapEntryWbtCommand : public ReadStripeMapEntryWbtCommand
{
public:
    using ReadStripeMapEntryWbtCommand::ReadStripeMapEntryWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos

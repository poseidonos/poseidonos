#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/write_stripe_map_entry_wbt_command.h"

namespace pos
{
class MockWriteStripeMapEntryWbtCommand : public WriteStripeMapEntryWbtCommand
{
public:
    using WriteStripeMapEntryWbtCommand::WriteStripeMapEntryWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/write_stripe_map_wbt_command.h"

namespace pos
{
class MockWriteStripeMapWbtCommand : public WriteStripeMapWbtCommand
{
public:
    using WriteStripeMapWbtCommand::WriteStripeMapWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos

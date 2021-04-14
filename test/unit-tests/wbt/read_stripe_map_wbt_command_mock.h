#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/read_stripe_map_wbt_command.h"

namespace pos
{
class MockReadStripeMapWbtCommand : public ReadStripeMapWbtCommand
{
public:
    using ReadStripeMapWbtCommand::ReadStripeMapWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/replay/replay_stripe_map_update.h"

namespace pos
{
class MockReplayStripeMapUpdate : public ReplayStripeMapUpdate
{
public:
    using ReplayStripeMapUpdate::ReplayStripeMapUpdate;
    MOCK_METHOD(int, Replay, (), (override));
};

} // namespace pos

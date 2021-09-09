#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/replay/replay_stripe_allocation.h"

namespace pos
{
class MockReplayStripeAllocation : public ReplayStripeAllocation
{
public:
    using ReplayStripeAllocation::ReplayStripeAllocation;
    MOCK_METHOD(int, Replay, (), (override));
};

} // namespace pos

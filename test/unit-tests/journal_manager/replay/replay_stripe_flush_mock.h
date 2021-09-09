#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/replay/replay_stripe_flush.h"

namespace pos
{
class MockReplayStripeFlush : public ReplayStripeFlush
{
public:
    using ReplayStripeFlush::ReplayStripeFlush;
    MOCK_METHOD(int, Replay, (), (override));
};

} // namespace pos

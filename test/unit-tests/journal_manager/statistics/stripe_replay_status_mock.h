#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/statistics/stripe_replay_status.h"

namespace pos
{
class MockStripeReplayStatus : public StripeReplayStatus
{
public:
    using StripeReplayStatus::StripeReplayStatus;
    MOCK_METHOD(void, Print, (), (override));
};

} // namespace pos

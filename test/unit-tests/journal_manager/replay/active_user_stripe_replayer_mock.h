#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/replay/active_user_stripe_replayer.h"

namespace pos
{
class MockActiveUserStripeReplayer : public ActiveUserStripeReplayer
{
public:
    using ActiveUserStripeReplayer::ActiveUserStripeReplayer;
    MOCK_METHOD(int, Replay, (), (override));
    MOCK_METHOD(void, Update, (StripeId userLsid), (override));
};

} // namespace pos

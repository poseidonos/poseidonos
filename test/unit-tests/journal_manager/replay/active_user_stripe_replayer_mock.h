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
};

} // namespace pos

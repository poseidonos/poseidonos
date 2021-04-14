#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/replay/active_wb_stripe_replayer.h"

namespace pos
{
class MockActiveWBStripeReplayer : public ActiveWBStripeReplayer
{
public:
    using ActiveWBStripeReplayer::ActiveWBStripeReplayer;
};

} // namespace pos

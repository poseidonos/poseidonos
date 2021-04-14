#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/replay/replay_stripe.h"

namespace pos
{
class MockReplayStripe : public ReplayStripe
{
public:
    using ReplayStripe::ReplayStripe;
};

} // namespace pos

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/replay/i_replay_stripe.h"

namespace pos
{
class MockIReplayStripe : public IReplayStripe
{
public:
    using IReplayStripe::IReplayStripe;
    MOCK_METHOD(void, AddLog, (ReplayLog replayLog), (override));
    MOCK_METHOD(int, Replay, (), (override));
};

} // namespace pos

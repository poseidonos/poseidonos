#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/replay/user_replay_stripe.h"

namespace pos
{
class MockUserReplayStripe : public UserReplayStripe
{
public:
    using UserReplayStripe::UserReplayStripe;
    MOCK_METHOD(void, AddLog, (ReplayLog replayLog), (override));
    MOCK_METHOD(int, Replay, (), (override));
};

} // namespace pos

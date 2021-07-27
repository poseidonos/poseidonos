#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/replay/gc_replay_stripe.h"

namespace pos
{
class MockGcReplayStripe : public GcReplayStripe
{
public:
    using GcReplayStripe::GcReplayStripe;
    MOCK_METHOD(void, AddLog, (ReplayLog replayLog), (override));
    MOCK_METHOD(int, Replay, (), (override));
};

} // namespace pos

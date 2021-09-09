#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/replay/replay_event.h"

namespace pos
{
class MockReplayEvent : public ReplayEvent
{
public:
    using ReplayEvent::ReplayEvent;
    MOCK_METHOD(int, Replay, (), (override));
    MOCK_METHOD(ReplayEventType, GetType, (), (override));
};

} // namespace pos

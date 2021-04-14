#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/replay/replay_state_changer.h"

namespace pos
{
class MockReplayStateChanger : public ReplayStateChanger
{
public:
    using ReplayStateChanger::ReplayStateChanger;
    MOCK_METHOD(void, StateChanged, (StateContext * prev, StateContext* next), (override));
};

} // namespace pos

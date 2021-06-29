#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/replay/replay_event_factory.h"

namespace pos
{
class MockReplayEventFactory : public ReplayEventFactory
{
public:
    using ReplayEventFactory::ReplayEventFactory;
};

} // namespace pos

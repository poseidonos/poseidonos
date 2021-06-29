#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/event_scheduler/spdk_event_scheduler.h"

namespace pos
{
class MockSpdkEventScheduler : public SpdkEventScheduler
{
public:
    using SpdkEventScheduler::SpdkEventScheduler;
};

} // namespace pos

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/event_scheduler/scheduler_queue.h"

namespace pos
{
class MockSchedulerQueue : public SchedulerQueue
{
public:
    using SchedulerQueue::SchedulerQueue;
};

} // namespace pos

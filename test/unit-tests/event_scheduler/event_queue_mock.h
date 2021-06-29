#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/event_scheduler/event_queue.h"

namespace pos
{
class MockEventQueue : public EventQueue
{
public:
    using EventQueue::EventQueue;
};

} // namespace pos

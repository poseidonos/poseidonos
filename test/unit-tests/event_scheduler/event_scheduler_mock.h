#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/event_scheduler/event_scheduler.h"

namespace pos
{
class MockEventScheduler : public EventScheduler
{
public:
    using EventScheduler::EventScheduler;
    MOCK_METHOD(void, EnqueueEvent, (EventSmartPtr input), (override));
    MOCK_METHOD(uint32_t, GetWorkerIDMinimumJobs, (uint32_t numa), (override));
};

} // namespace pos

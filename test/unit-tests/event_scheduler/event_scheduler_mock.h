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
    MOCK_METHOD(void, CheckAndSetQueueOccupancy, (BackendEvent eventId), (override));
    MOCK_METHOD(EventSmartPtr, PickWorkerEvent, (EventWorker* eventWorker), (override));
    MOCK_METHOD(void, IoEnqueued, (BackendEvent type, uint64_t size), (override));
    MOCK_METHOD(void, IoDequeued, (BackendEvent type, uint64_t size), (override));
    MOCK_METHOD(int32_t, GetAllowedIoCount, (BackendEvent eventId), (override));
    // MOCK_METHOD(uint32_t, GetWorkerIDMinimumJobs, (uint32_t numa), (override));
    // MOCK_METHOD((EventSmartPtr), DequeueWorkerEvent, (), (override));
    // MOCK_METHOD(uint32_t, GetWorkerQueueSize, (), (override));
};

} // namespace pos

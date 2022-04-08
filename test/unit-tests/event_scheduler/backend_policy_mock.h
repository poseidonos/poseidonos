#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>
#include <queue>

#include "src/event_scheduler/backend_policy.h"

namespace pos
{
class MockBackendPolicy : public BackendPolicy
{
public:
    using BackendPolicy::BackendPolicy;
    MOCK_METHOD(void, EnqueueEvent, (EventSmartPtr input), (override));
    MOCK_METHOD(std::queue<EventSmartPtr>, DequeueEvents, (), (override));
    MOCK_METHOD(int, Run, (), (override));
    MOCK_METHOD(EventSmartPtr, PickWorkerEvent, (EventWorker*), (override));
    MOCK_METHOD(void, CheckAndSetQueueOccupancy, (BackendEvent eventId), (override));
};

} // namespace pos

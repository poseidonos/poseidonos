#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>
#include <queue>

#include "src/event_scheduler/backend_event_minimum_policy.h"

namespace pos
{
class MockBackendEventMinimumPolicy : public BackendEventMinimumPolicy
{
public:
    using BackendEventMinimumPolicy::BackendEventMinimumPolicy;
    MOCK_METHOD(void, EnqueueEvent, (EventSmartPtr input), (override));
    MOCK_METHOD(std::queue<EventSmartPtr>, DequeueEvents, (), (override));
    MOCK_METHOD(int, Run, (), (override));
    MOCK_METHOD(EventSmartPtr, PickWorkerEvent, (EventWorker*), (override));
};

} // namespace pos

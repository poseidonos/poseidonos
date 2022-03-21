#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>
#include <queue>

#include "src/event_scheduler/backend_event_ratio_policy.h"

namespace pos
{
class MockBackendEventRatioPolicy : public BackendEventRatioPolicy
{
public:
    using BackendEventRatioPolicy::BackendEventRatioPolicy;
    MOCK_METHOD(void, EnqueueEvent, (EventSmartPtr input), (override));
    MOCK_METHOD(std::queue<EventSmartPtr>, DequeueEvents, (), (override));
    MOCK_METHOD(EventSmartPtr, DequeueWorkerEvent, (), (override));
    MOCK_METHOD(uint32_t, GetWorkerQueueSize, (), (override));
    MOCK_METHOD(int, Run, (), (override));
    MOCK_METHOD(EventSmartPtr, PickWorkerEvent, (EventWorker*), (override));
};

} // namespace pos

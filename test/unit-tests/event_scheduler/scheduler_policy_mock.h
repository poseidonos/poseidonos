#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/event_scheduler/scheduler_policy.h"

namespace pos
{
class MockISchedulerPolicy : public ISchedulerPolicy
{
public:
    using ISchedulerPolicy::ISchedulerPolicy;
    MOCK_METHOD(unsigned int, GetProperWorkerID, (), (override));
};

} // namespace pos

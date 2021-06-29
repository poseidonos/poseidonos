#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/event_scheduler/minimum_job_policy.h"

namespace pos
{
class MockMinimumJobPolicy : public MinimumJobPolicy
{
public:
    using MinimumJobPolicy::MinimumJobPolicy;
    MOCK_METHOD(unsigned int, GetProperWorkerID, (), (override));
};

} // namespace pos

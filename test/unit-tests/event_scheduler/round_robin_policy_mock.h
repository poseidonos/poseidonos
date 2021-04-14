#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/event_scheduler/round_robin_policy.h"

namespace pos
{
class MockRoundRobinPolicy : public RoundRobinPolicy
{
public:
    using RoundRobinPolicy::RoundRobinPolicy;
    MOCK_METHOD(unsigned int, GetProperWorkerID, (), (override));
};

} // namespace pos

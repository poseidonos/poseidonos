#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/event_cpu_policy.h"

namespace pos
{
class MockEventCpuPolicy : public EventCpuPolicy
{
public:
    using EventCpuPolicy::EventCpuPolicy;
    MOCK_METHOD(void, HandlePolicy, (), (override));
};

} // namespace pos

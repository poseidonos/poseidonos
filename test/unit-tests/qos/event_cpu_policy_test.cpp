#include "src/qos/event_cpu_policy.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/qos/qos_context_mock.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;

namespace pos
{
TEST(EventCpuPolicy, EventCpuPolicy_Constructor_One_Stack)
{
    NiceMock<MockQosContext> qosContext;
    EventCpuPolicy eventCpuPolicy(&qosContext);
}
TEST(EventCpuPolicy, EventCpuPolicy_Constructor_One_Heap)
{
    NiceMock<MockQosContext> qosContext;
    EventCpuPolicy* eventCpuPolicy = new EventCpuPolicy(&qosContext);
    delete eventCpuPolicy;
}

TEST(EventCpuPolicy, HandlePolicy_TestRun)
{
    NiceMock<MockQosContext> qosContext;
    EventCpuPolicy eventCpuPolicy(&qosContext);
    eventCpuPolicy.HandlePolicy();
    //do nothing
}

} // namespace pos

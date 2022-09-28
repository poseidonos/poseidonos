#include "src/qos/event_cpu_policy.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/qos/qos_context_mock.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::ReturnRef;

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

TEST(EventCpuPolicy, HandlePolicy_priority_highest)
{
    NiceMock<MockQosContext> mockQosContext;
    QosUserPolicy userPolicy;
    ON_CALL(mockQosContext, GetQosUserPolicy()).WillByDefault(ReturnRef(userPolicy));
    RebuildUserPolicy& rebuildUserPolicy = userPolicy.GetRebuildUserPolicy();
    rebuildUserPolicy.SetRebuildImpact(PRIORITY_HIGH);
    QosParameters parameters;
    ON_CALL(mockQosContext, GetQosParameters()).WillByDefault(ReturnRef(parameters));
    EventCpuPolicy eventCpuPolicy(&mockQosContext);
    eventCpuPolicy.HandlePolicy();
}
TEST(EventCpuPolicy, HandlePolicy_priority_medium)
{
    NiceMock<MockQosContext> mockQosContext;
    QosUserPolicy userPolicy;
    ON_CALL(mockQosContext, GetQosUserPolicy()).WillByDefault(ReturnRef(userPolicy));
    RebuildUserPolicy& rebuildUserPolicy = userPolicy.GetRebuildUserPolicy();
    rebuildUserPolicy.SetRebuildImpact(PRIORITY_MEDIUM);
    EventCpuPolicy eventCpuPolicy(&mockQosContext);
    eventCpuPolicy.HandlePolicy();
}
TEST(EventCpuPolicy, HandlePolicy_priority_lowest)
{
    NiceMock<MockQosContext> mockQosContext;
    QosUserPolicy userPolicy;
    ON_CALL(mockQosContext, GetQosUserPolicy()).WillByDefault(ReturnRef(userPolicy));
    RebuildUserPolicy& rebuildUserPolicy = userPolicy.GetRebuildUserPolicy();
    rebuildUserPolicy.SetRebuildImpact(PRIORITY_LOW);
    EventCpuPolicy eventCpuPolicy(&mockQosContext);
    eventCpuPolicy.HandlePolicy();
    eventCpuPolicy.HandlePolicy();
}

} // namespace pos

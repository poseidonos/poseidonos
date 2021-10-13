#include "src/qos/monitoring_manager_array.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/qos/qos_context_mock.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;
namespace pos
{
TEST(QosMonitoringManagerArray, QosMonitoringManagerArray_Constructor_One_Stack)
{
    uint32_t arrayIndex = 1;
    NiceMock<MockQosContext> mockQoscontext;
    QosMonitoringManagerArray qosMonitoringManagerArray(arrayIndex, &mockQoscontext);
}
TEST(QosMonitoringManagerArray, QosMonitoringManagerArray_Constructor_One_Heap)
{
    uint32_t arrayIndex = 1;
    NiceMock<MockQosContext> mockQoscontext;
    QosMonitoringManagerArray* qosMonitoringManagerArray = new QosMonitoringManagerArray(arrayIndex, &mockQoscontext);
    delete qosMonitoringManagerArray;
}

TEST(QosMonitoringManagerArray, UpdateContextUserVolumePolicy_Test_Run)
{
    uint32_t arrayIndex = 1;
    NiceMock<MockQosContext> mockQoscontext;
    QosMonitoringManagerArray qosMonitoringManagerArray(arrayIndex, &mockQoscontext);
    qosMonitoringManagerArray.UpdateContextUserVolumePolicy();
}

TEST(QosMonitoringManagerArray, UpdateVolumeParameter_TestRun)
{
#if 0
    uint32_t arrayIndex = 0;
    NiceMock<MockQosContext> mockQoscontext;
    QosMonitoringManagerArray qosMonitoringManagerArray(arrayIndex,&mockQoscontext);
    uint32_t volId = 0;
    qosMonitoringManagerArray.UpdateVolumeParameter(volId);
#endif
}

TEST(QosMonitoringManagerArray, UpdateContextResourceDetails_TestRun)
{
    uint32_t arrayIndex = 1;
    NiceMock<MockQosContext> mockQoscontext;
    QosMonitoringManagerArray qosMonitoringManagerArray(arrayIndex, &mockQoscontext);
    qosMonitoringManagerArray.UpdateContextResourceDetails();
}

TEST(QosMonitoringManagerArray, UpdateContextUserRebuildPolicy_Test_Run)
{
    uint32_t arrayIndex = 1;
    NiceMock<MockQosContext> mockQoscontext;
    QosMonitoringManagerArray qosMonitoringManagerArray(arrayIndex, &mockQoscontext);
    qosMonitoringManagerArray.UpdateContextUserRebuildPolicy();
}

TEST(QosMonitoringManagerArray, VolParamActivities_TestRun)
{
    uint32_t arrayIndex = 1;
    NiceMock<MockQosContext> mockQoscontext;
    uint32_t volId = 0;
    uint32_t reactor = 0;
    QosMonitoringManagerArray qosMonitoringManagerArray(arrayIndex, &mockQoscontext);
    bool expected = false, actual;
    actual = qosMonitoringManagerArray.VolParamActivities(volId, reactor);
    ASSERT_EQ(expected, actual);
}

} // namespace pos

#include "src/qos/monitoring_manager_array.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/qos/qos_context_mock.h"
#include "test/unit-tests/qos/qos_manager_mock.h"
#include "src/qos/qos_common.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::ReturnRef;
using ::testing::Return;

namespace pos
{
TEST(QosMonitoringManagerArray, QosMonitoringManagerArray_Constructor_One_Stack)
{
    uint32_t arrayIndex = 1;
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    QosMonitoringManagerArray qosMonitoringManagerArray(arrayIndex, &mockQoscontext, &mockQosManager);
}

TEST(QosMonitoringManagerArray, QosMonitoringManagerArray_Constructor_One_Heap)
{
    uint32_t arrayIndex = 1;
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    QosMonitoringManagerArray* qosMonitoringManagerArray = new QosMonitoringManagerArray(arrayIndex, &mockQoscontext, &mockQosManager);
    delete qosMonitoringManagerArray;
}

TEST(QosMonitoringManagerArray, UpdateContextUserVolumePolicy_Test_Run)
{
    uint32_t arrayIndex = 1;
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    QosUserPolicy userPolicy;
    ON_CALL(mockQoscontext, GetQosUserPolicy()).WillByDefault(ReturnRef(userPolicy));
    QosMonitoringManagerArray qosMonitoringManagerArray(arrayIndex, &mockQoscontext, &mockQosManager);
    qosMonitoringManagerArray.UpdateContextUserVolumePolicy();
}

TEST(QosMonitoringManagerArray, UpdateVolumeParameter_TestRun)
{
    uint32_t arrayIndex = 0;
    uint32_t volId = 0;
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;

    QosParameters parameters;
    AllVolumeParameter& allVolumeParam = parameters.GetAllVolumeParameter();
    VolumeParameter volParam;
    volParam.SetAvgBandwidth(100);
    volParam.SetAvgIops(100);
    allVolumeParam.InsertVolumeParameter(arrayIndex, volId, volParam);
    ON_CALL(mockQoscontext, GetQosParameters()).WillByDefault(ReturnRef(parameters));
    QosMonitoringManagerArray qosMonitoringManagerArray(arrayIndex, &mockQoscontext, &mockQosManager);
    qosMonitoringManagerArray.UpdateVolumeParameter(volId);
}

TEST(QosMonitoringManagerArray, UpdateContextResourceDetails_TestRun)
{
    uint32_t arrayIndex = 1;
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    QosMonitoringManagerArray qosMonitoringManagerArray(arrayIndex, &mockQoscontext, &mockQosManager);
    qosMonitoringManagerArray.UpdateContextResourceDetails();
}

TEST(QosMonitoringManagerArray, UpdateContextUserRebuildPolicy_Test_Run)
{
    uint32_t arrayIndex = 1;
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    QosUserPolicy userPolicy;
    ON_CALL(mockQoscontext, GetQosUserPolicy()).WillByDefault(ReturnRef(userPolicy));
    QosMonitoringManagerArray qosMonitoringManagerArray(arrayIndex, &mockQoscontext, &mockQosManager);
    qosMonitoringManagerArray.UpdateContextUserRebuildPolicy();
}

TEST(QosMonitoringManagerArray, VolParamActivities_TestRun)
{
    uint32_t arrayIndex = 1;
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    uint32_t volId = 0;
    uint32_t reactor = 0;
    QosParameters parameters;
    AllVolumeParameter& allVolumeParam = parameters.GetAllVolumeParameter();
    ON_CALL(mockQoscontext, GetQosParameters()).WillByDefault(ReturnRef(parameters));
    bw_iops_parameter volParam;
    volParam.valid = M_VALID_ENTRY;
    QosMonitoringManagerArray qosMonitoringManagerArray(arrayIndex, &mockQoscontext, &mockQosManager);
    bool expected = true, actual;
    actual = qosMonitoringManagerArray.VolParamActivities(volId, reactor);
    ASSERT_EQ(expected, actual);
}

} // namespace pos

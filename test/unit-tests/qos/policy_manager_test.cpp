#include "src/qos/policy_manager.h"
#include "src/qos/qos_manager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/qos/qos_context_mock.h"
#include "test/unit-tests/qos/qos_manager_mock.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
TEST(QosPolicyManager, QosPolicyManager_Construtor_One_Stack)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    QosPolicyManager qosPolicyManager(&mockQoscontext, &mockQosManager);
}

TEST(QosPolicyManager, QosPolicyManager_Construtor_One_Heap)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    QosPolicyManager* qosPolicyManager = new QosPolicyManager(&mockQoscontext, &mockQosManager);
    delete qosPolicyManager;
}

TEST(QosPolicyManager, Execute_InternalManager_Correction_Disable)
{
    NiceMock<MockQosContext> qosContext;
    NiceMock<MockQosManager> mockQosManager;
    // qosContext.SetApplyCorrection(false);
    QosUserPolicy userPolicy;
    ON_CALL(qosContext, GetQosUserPolicy()).WillByDefault(ReturnRef(userPolicy));
    ON_CALL(qosContext, GetApplyCorrection()).WillByDefault(Return(false));
    QosParameters parameters;
    ON_CALL(qosContext, GetQosParameters()).WillByDefault(ReturnRef(parameters));
    QosPolicyManager qosPolicyManager(&qosContext, &mockQosManager);
    qosPolicyManager.Execute();
    QosInternalManagerType expectedManager = QosInternalManager_Monitor;
    QosInternalManagerType actualManager = qosPolicyManager.GetNextManagerType();
    ASSERT_EQ(expectedManager, actualManager);
}

TEST(QosPolicyManager, Execute_InternalManager_Correction_Enable)
{
    NiceMock<MockQosContext> qosContext;
    NiceMock<MockQosManager> mockQosManager;
    QosUserPolicy userPolicy;
    ON_CALL(qosContext, GetQosUserPolicy()).WillByDefault(ReturnRef(userPolicy));
    ON_CALL(qosContext, GetApplyCorrection()).WillByDefault(Return(true));
    QosParameters parameters;
    ON_CALL(qosContext, GetQosParameters()).WillByDefault(ReturnRef(parameters));
    QosPolicyManager qosPolicyManager(&qosContext, &mockQosManager);
    qosPolicyManager.Execute();
    QosInternalManagerType expectedManager = QosInternalManager_Correction;
    QosInternalManagerType actualManager = qosPolicyManager.GetNextManagerType();
    ASSERT_EQ(expectedManager, actualManager);
}
} // namespace pos

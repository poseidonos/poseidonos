#include "src/qos/policy_manager.h"
#include "src/qos/qos_manager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/qos/qos_context_mock.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(QosPolicyManager, QosPolicyManager_Construtor_One_Stack)
{
    NiceMock<MockQosContext> mockQoscontext;
    QosPolicyManager qosPolicyManager(&mockQoscontext);
}
TEST(QosPolicyManager, QosPolicyManager_Construtor_One_Heap)
{
    NiceMock<MockQosContext> mockQoscontext;
    QosPolicyManager* qosPolicyManager = new QosPolicyManager(&mockQoscontext);
    delete qosPolicyManager;
}

TEST(QosPolicyManager, Execute_InternalManager_Correction_Disable)
{
#if 0
    QosContext qosContext;
    qosContext.SetApplyCorrection(false);
    QosPolicyManager qosPolicyManager(&qosContext);
    qosPolicyManager.Execute();
    QosInternalManagerType expectedManager = QosInternalManager_Monitor;
    QosInternalManagerType actualManager = qosPolicyManager.GetNextManagerType();
    ASSERT_EQ(expectedManager, actualManager);
#endif
}

TEST(QosPolicyManager, Execute_InternalManager_Correction_Enable)
{
    QosContext qosContext;
    QosPolicyManager qosPolicyManager(&qosContext);
    qosContext.SetApplyCorrection(true);
    qosPolicyManager.Execute();
    QosInternalManagerType expectedManager = QosInternalManager_Correction;
    QosInternalManagerType actualManager = qosPolicyManager.GetNextManagerType();
    ASSERT_EQ(expectedManager, actualManager);
}

TEST(QosPolicyManager, Execute_PolicyManager_Array)
{
    QosManager* qosManager = QosManagerSingleton::Instance();
    qosManager->UpdateArrayMap("PosArray_1");
    NiceMock<MockQosContext> qoscontext;
    QosPolicyManager qosPolicyManager(&qoscontext);
    qosPolicyManager.Execute();
}

} // namespace pos

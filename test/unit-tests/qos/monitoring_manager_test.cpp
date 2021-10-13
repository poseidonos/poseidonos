#include "src/qos/monitoring_manager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/qos/qos_context_mock.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;

namespace pos
{
TEST(QosMonitoringManager, QosMonitoringManager_Construtor_One_Stack)
{
    NiceMock<MockQosContext> mockQoscontext;
    QosMonitoringManager qosMonitoringManager(&mockQoscontext);
}
TEST(QosMonitoringManager, QosMonitoringManager_Construtor_One_Heap)
{
    NiceMock<MockQosContext> mockQoscontext;
    QosMonitoringManager* qosMonitoringManager = new QosMonitoringManager(&mockQoscontext);
    delete qosMonitoringManager;
}
TEST(QosMonitoringManager, Check_Execute_Getter_Setter_Internal_Manager)
{
    NiceMock<MockQosContext> mockQoscontext;
    QosMonitoringManager qosMonitoringManager(&mockQoscontext);
    qosMonitoringManager.Execute();
    QosInternalManagerType expectedManager = QosInternalManager_Processing;
    QosInternalManagerType actualManager = qosMonitoringManager.GetNextManagerType();
    ASSERT_EQ(expectedManager, actualManager);
}

} // namespace pos

#include "src/qos/processing_manager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/qos/qos_context_mock.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;

namespace pos
{
TEST(QosProcessingManager, QosProcessingManager_Constructor_One_Stack)
{
    NiceMock<MockQosContext> mockQoscontext;
    QosProcessingManager qosProcessingManager(&mockQoscontext);
}

TEST(QosProcessingManager, QosProcessingManager_Constructor_One_Heap)
{
    NiceMock<MockQosContext> mockQoscontext;
    QosProcessingManager* qosProcessingManager = new QosProcessingManager(&mockQoscontext);
    delete qosProcessingManager;
}
TEST(QosProcessingManager, Check_Execute_Getter_Setter_NextManager)
{
    NiceMock<MockQosContext> mockQoscontext;
    QosProcessingManager qosProcessingManager(&mockQoscontext);
    qosProcessingManager.Execute();
    QosInternalManagerType expectedManager = QosInternalManager_Policy;
    QosInternalManagerType actualManager = qosProcessingManager.GetNextManagerType();
    ASSERT_EQ(expectedManager, actualManager);
}

} // namespace pos

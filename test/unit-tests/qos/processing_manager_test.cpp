#include "src/qos/processing_manager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/qos/qos_manager.h"
#include "test/unit-tests/qos/qos_context_mock.h"
#include "test/unit-tests/qos/qos_manager_mock.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
TEST(QosProcessingManager, QosProcessingManager_Constructor_One_Stack)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    QosProcessingManager qosProcessingManager(&mockQoscontext, &mockQosManager);
}

TEST(QosProcessingManager, QosProcessingManager_Constructor_One_Heap)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    QosProcessingManager* qosProcessingManager = new QosProcessingManager(&mockQoscontext, &mockQosManager);
    delete qosProcessingManager;
}
TEST(QosProcessingManager, Check_Execute_Processing_Manager_Init)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    QosProcessingManager qosProcessingManager(&mockQoscontext, &mockQosManager);
    qosProcessingManager.Execute();
    QosInternalManagerType expectedManager = QosInternalManager_Policy;
    QosInternalManagerType actualManager = qosProcessingManager.GetNextManagerType();
    ASSERT_EQ(expectedManager, actualManager);
}
TEST(QosProcessingManager, Check_Execute_Getter_Setter_NextManager)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    std::map<uint32_t, uint32_t>& activeVolumeMap = mockQoscontext.GetActiveVolumes();
    activeVolumeMap[1] = 1;
    std::string arrayName = "POSArray";
    QosManagerSingleton::Instance()->UpdateArrayMap(arrayName);

    uint32_t arrayId = 0;
    uint32_t volId = 1;
    QosUserPolicy currUserPolicy;
    AllVolumeUserPolicy& currAllVolumePolicy = currUserPolicy.GetAllVolumeUserPolicy();
    VolumeUserPolicy volPolicy;
    volPolicy.SetMaxBandwidth(100);
    volPolicy.SetMaxIops(100);
    currAllVolumePolicy.InsertVolumeUserPolicy(arrayId, volId, volPolicy);
    ON_CALL(mockQoscontext, GetQosUserPolicy()).WillByDefault(ReturnRef(currUserPolicy));

    QosParameters parameters;
    AllVolumeParameter& allVolumeParam = parameters.GetAllVolumeParameter();
    VolumeParameter volParam;
    volParam.SetAvgBandwidth(100);
    volParam.SetAvgIops(100);
    allVolumeParam.InsertVolumeParameter(arrayId, volId, volParam);
    ON_CALL(mockQoscontext, GetQosParameters()).WillByDefault(ReturnRef(parameters));

    QosProcessingManager qosProcessingManager(&mockQoscontext, &mockQosManager);
    qosProcessingManager.Execute();
    QosInternalManagerType expectedManager = QosInternalManager_Policy;
    QosInternalManagerType actualManager = qosProcessingManager.GetNextManagerType();
    ASSERT_EQ(expectedManager, actualManager);
}

} // namespace pos

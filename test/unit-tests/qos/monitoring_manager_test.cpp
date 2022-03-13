#include "src/qos/monitoring_manager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/qos/qos_manager.h"
#include "test/unit-tests/qos/qos_context_mock.h"
#include "test/unit-tests/qos/qos_manager_mock.h"
#include "test/unit-tests/spdk_wrapper/caller/spdk_pos_nvmf_caller_mock.h"
using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
TEST(QosMonitoringManager, QosMonitoringManager_Construtor_One_Stack)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    QosMonitoringManager qosMonitoringManager(&mockQoscontext, &mockQosManager);
}

TEST(QosMonitoringManager, QosMonitoringManager_Construtor_One_Heap)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    QosMonitoringManager* qosMonitoringManager = new QosMonitoringManager(&mockQoscontext, &mockQosManager);
    delete qosMonitoringManager;
}

TEST(QosMonitoringManager, Reset)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    QosMonitoringManager* qosMonitoringManager = new QosMonitoringManager(&mockQoscontext, &mockQosManager);
    qosMonitoringManager->Reset();
    delete qosMonitoringManager;
}

TEST(QosMonitoringManager, Check_Execute_Getter_Setter_Internal_Manager_fe_qos_false)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    uint32_t arrayId = 0;
    uint32_t volId = 1;
    ON_CALL(mockQosManager, IsFeQosEnabled()).WillByDefault(Return(false));
    QosUserPolicy currUserPolicy;
    AllVolumeUserPolicy& currAllVolumePolicy = currUserPolicy.GetAllVolumeUserPolicy();
    VolumeUserPolicy volPolicy;
    volPolicy.SetMaxBandwidth(100);
    volPolicy.SetMaxIops(100);
    currAllVolumePolicy.InsertVolumeUserPolicy(arrayId, volId, volPolicy);
    ON_CALL(mockQoscontext, GetQosUserPolicy()).WillByDefault(ReturnRef(currUserPolicy));
    QosParameters parameters;
    ON_CALL(mockQoscontext, GetQosParameters()).WillByDefault(ReturnRef(parameters));
    QosMonitoringManager qosMonitoringManager(&mockQoscontext, &mockQosManager);
    qosMonitoringManager.Execute();
    QosInternalManagerType expectedManager = QosInternalManager_Processing;
    QosInternalManagerType actualManager = qosMonitoringManager.GetNextManagerType();
}

TEST(QosMonitoringManager, Check_Execute_Getter_Setter_Internal_Manager_fe_qos_true)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    NiceMock<MockSpdkPosNvmfCaller>* mockSpdkPosNvmfCaller =
        new NiceMock<MockSpdkPosNvmfCaller>;
    uint32_t arrayId = 0;
    uint32_t volId = 1;
    std::string arrayName = "POSArray";
    mockQosManager.UpdateArrayMap(arrayName);
    mockQosManager.UpdateSubsystemToVolumeMap(1, 1, arrayName);
    ON_CALL(mockQosManager, IsFeQosEnabled()).WillByDefault(Return(true));
    QosCorrection& qosCorrection = mockQoscontext.GetQosCorrection();
    AllVolumeThrottle& allVolumeThrottle = qosCorrection.GetVolumeThrottlePolicy();
    allVolumeThrottle.Reset();
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
    allVolumeParam.InsertVolumeParameter(arrayId +1 , volId+1, volParam);
    ON_CALL(mockQoscontext, GetQosParameters()).WillByDefault(ReturnRef(parameters));
    ON_CALL(mockQoscontext, AllReactorsProcessed()).WillByDefault(Return(true));
    std::vector<uint32_t>& reactorCoreList = mockQoscontext.GetReactorCoreList();
    reactorCoreList.push_back(1);
    ON_CALL(*mockSpdkPosNvmfCaller, SpdkNvmfGetReactorSubsystemMapping(_, _)).WillByDefault(Return(1));
    QosMonitoringManager qosMonitoringManager(&mockQoscontext, &mockQosManager, mockSpdkPosNvmfCaller);
    qosMonitoringManager.Execute();
    QosInternalManagerType expectedManager = QosInternalManager_Processing;
    QosInternalManagerType actualManager = qosMonitoringManager.GetNextManagerType();
}

} // namespace pos

#include "src/qos/correction_manager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/qos/qos_context_mock.h"
#include "test/unit-tests/qos/qos_manager_mock.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::ReturnRef;

namespace pos
{
TEST(QosCorrectionManager, QosCorrectionManager_Constructor_One_Stack)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    QosCorrectionManager qosCorrectionManager(&mockQoscontext, &mockQosManager);
}

TEST(QosCorrectionManager, QosCorrectionManager_Constructor_One_Heap)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    QosCorrectionManager* qosCorrectionManager = new QosCorrectionManager(&mockQoscontext, &mockQosManager);
    delete qosCorrectionManager;
}

TEST(QosCorrectionManager, Check_Execute_Getter_Setter_Internal_Manager)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    QosCorrectionManager qosCorrectionManager(&mockQoscontext, &mockQosManager);
    qosCorrectionManager.Execute();
    QosInternalManagerType expectedManager = QosInternalManager_Monitor;
    QosInternalManagerType actualManager = qosCorrectionManager.GetNextManagerType();
    ASSERT_EQ(expectedManager, actualManager);
}

TEST(QosCorrectionManager, Check_Execute_Volume_Wrr_Policy)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    QosUserPolicy userPolicy;
    ON_CALL(mockQoscontext, GetQosUserPolicy()).WillByDefault(ReturnRef(userPolicy));

    QosParameters parameters;
    ON_CALL(mockQoscontext, GetQosParameters()).WillByDefault(ReturnRef(parameters));
    QosCorrectionManager qosCorrectionManager(&mockQoscontext, &mockQosManager);
    QosCorrection& correction = mockQoscontext.GetQosCorrection();
    correction.SetCorrectionType(QosCorrection_VolumeThrottle, true);
    correction.SetCorrectionType(QosCorrection_EventWrr, true);
    std::map<uint32_t, uint32_t>& activeVolumeMap = mockQoscontext.GetActiveVolumes();
    activeVolumeMap[1] = 1;
    std::map<uint32_t, map<uint32_t, uint32_t>>& volReactorMap = mockQoscontext.GetActiveVolumeReactors();
    volReactorMap[1][1] = 1;
    qosCorrectionManager.Execute();
}

TEST(QosCorrectionManager, Check_Execute_Volume_Policy)
{
    NiceMock<MockQosContext> mockQosContext;
    NiceMock<MockQosManager> mockQosManager;
    QosUserPolicy userPolicy;
    ON_CALL(mockQosContext, GetQosUserPolicy()).WillByDefault(ReturnRef(userPolicy));
    AllVolumeUserPolicy& allVolUserPolicy = userPolicy.GetAllVolumeUserPolicy();
    allVolUserPolicy.SetMinimumGuaranteeVolume(1, 0);
    QosParameters parameters;
    ON_CALL(mockQosContext, GetQosParameters()).WillByDefault(ReturnRef(parameters));
    QosCorrectionManager qosCorrectionManager(&mockQosContext, &mockQosManager);
    QosCorrection& correction = mockQosContext.GetQosCorrection();
    correction.SetCorrectionType(QosCorrection_VolumeThrottle, true);
    std::map<uint32_t, uint32_t>& activeVolumeMap = mockQosContext.GetActiveVolumes();
    activeVolumeMap[1] = 1;
    std::map<uint32_t, map<uint32_t, uint32_t>>& volReactorMap = mockQosContext.GetActiveVolumeReactors();
    volReactorMap[1][1] = 1;
    qosCorrectionManager.Execute();
}

TEST(QosCorrectionManager, Check_Volume_Min_Policy)
{
    NiceMock<MockQosContext> mockQosContext;
    NiceMock<MockQosManager> mockQosManager;
    uint32_t arrayId = 0;
    uint32_t volId = 1;
    uint32_t globalVolId = (arrayId + 1) * MAX_VOLUME_COUNT + (volId+1);
    uint32_t globalVolId2 = (arrayId) * MAX_VOLUME_COUNT + (volId);
    mockQosContext.SetTotalConnection(globalVolId, 2);
    mockQosContext.SetTotalConnection(globalVolId2, 2);

    QosUserPolicy userPolicy;
    AllVolumeUserPolicy& allVolUserPolicy = userPolicy.GetAllVolumeUserPolicy();
    allVolUserPolicy.SetMinimumGuaranteeVolume(volId, arrayId);
    VolumeUserPolicy volumeUserPolicy;
    volumeUserPolicy.SetMaxIops(15);
    volumeUserPolicy.SetMinIops(10);
    allVolUserPolicy.InsertVolumeUserPolicy(arrayId, volId, volumeUserPolicy);
    allVolUserPolicy.InsertVolumeUserPolicy(arrayId + 1, volId + 1, volumeUserPolicy);
    ON_CALL(mockQosContext, GetQosUserPolicy()).WillByDefault(ReturnRef(userPolicy));

    QosParameters parameters;
    AllVolumeParameter& allVolumeParam = parameters.GetAllVolumeParameter();
    VolumeParameter volParam;
    volParam.SetAvgBandwidth(100);
    volParam.SetAvgIops(100);
    allVolumeParam.InsertVolumeParameter(arrayId, volId, volParam);
    allVolumeParam.InsertVolumeParameter(arrayId +1 , volId+1, volParam);
    ON_CALL(mockQosContext, GetQosParameters()).WillByDefault(ReturnRef(parameters));

    QosCorrectionManager qosCorrectionManager(&mockQosContext, &mockQosManager);
    QosCorrection& correction = mockQosContext.GetQosCorrection();
    correction.SetCorrectionType(QosCorrection_VolumeThrottle, true);
    AllVolumeThrottle& allVolumeThrottle = correction.GetVolumeThrottlePolicy();
    std::map<std::pair<uint32_t, uint32_t>, VolumeThrottle>& volumeThrottleMap = allVolumeThrottle.GetVolumeThrottleMap();
    VolumeThrottle volThrottle;
    volThrottle.SetCorrectionType(QosCorrectionDir_Decrease);
    volumeThrottleMap[{arrayId + 1, volId + 1}]=  volThrottle;

    std::map<uint32_t, uint32_t>& activeVolumeMap = mockQosContext.GetActiveVolumes();
    activeVolumeMap[1] = 1;
    std::map<uint32_t, map<uint32_t, uint32_t>>& volReactorMap = mockQosContext.GetActiveVolumeReactors();
    volReactorMap[globalVolId][1] = 1;
    qosCorrectionManager.Execute();
}

TEST(QosCorrectionManager, Check_Volume_Min_Policy_MinBw_set)
{
    NiceMock<MockQosContext> mockQosContext;
    NiceMock<MockQosManager> mockQosManager;
    uint32_t arrayId = 0;
    uint32_t volId = 1;
    uint32_t globalVolId = (arrayId + 1) * MAX_VOLUME_COUNT + (volId+1);
    uint32_t globalVolId2 = (arrayId) * MAX_VOLUME_COUNT + (volId);
    mockQosContext.SetTotalConnection(globalVolId, 2);
    mockQosContext.SetTotalConnection(globalVolId2, 2);
    QosUserPolicy userPolicy;
    AllVolumeUserPolicy& allVolUserPolicy = userPolicy.GetAllVolumeUserPolicy();
    allVolUserPolicy.SetMinimumGuaranteeVolume(volId, arrayId);
    VolumeUserPolicy volumeUserPolicy;
    volumeUserPolicy.SetMaxIops(15);
    volumeUserPolicy.SetMinBandwidth(10);
    allVolUserPolicy.InsertVolumeUserPolicy(arrayId, volId, volumeUserPolicy);
    allVolUserPolicy.InsertVolumeUserPolicy(arrayId + 1, volId + 1, volumeUserPolicy);
    ON_CALL(mockQosContext, GetQosUserPolicy()).WillByDefault(ReturnRef(userPolicy));
    QosParameters parameters;
    AllVolumeParameter& allVolumeParam = parameters.GetAllVolumeParameter();
    VolumeParameter volParam;
    volParam.SetAvgBandwidth(100);
    volParam.SetAvgIops(100);
    allVolumeParam.InsertVolumeParameter(arrayId, volId, volParam);
    allVolumeParam.InsertVolumeParameter(arrayId +1 , volId+1, volParam);
    ON_CALL(mockQosContext, GetQosParameters()).WillByDefault(ReturnRef(parameters));
    QosCorrectionManager qosCorrectionManager(&mockQosContext, &mockQosManager);
    QosCorrection& correction = mockQosContext.GetQosCorrection();
    correction.SetCorrectionType(QosCorrection_VolumeThrottle, true);
    AllVolumeThrottle& allVolumeThrottle = correction.GetVolumeThrottlePolicy();
    std::map<std::pair<uint32_t, uint32_t>, VolumeThrottle>& volumeThrottleMap = allVolumeThrottle.GetVolumeThrottleMap();
    VolumeThrottle volThrottle;
    volThrottle.SetCorrectionType(QosCorrectionDir_Decrease);
    volumeThrottleMap[{arrayId + 1, volId + 1}]=  volThrottle;
    std::map<uint32_t, uint32_t>& activeVolumeMap = mockQosContext.GetActiveVolumes();
    activeVolumeMap[1] = 1;
    std::map<uint32_t, map<uint32_t, uint32_t>>& volReactorMap = mockQosContext.GetActiveVolumeReactors();
    volReactorMap[globalVolId][1] = 1;
    qosCorrectionManager.Execute();
}

TEST(QosCorrectionManager, Check_Execute_Wrr_Policy)
{
    NiceMock<MockQosContext> mockQosContext;
    NiceMock<MockQosManager> mockQosManager;
    QosCorrectionManager qosCorrectionManager(&mockQosContext, &mockQosManager);
    QosCorrection& correction = mockQosContext.GetQosCorrection();
    correction.SetCorrectionType(QosCorrection_EventWrr, true);
    QosEventWrrWeight& eventWrrPolicy = correction.GetEventWrrWeightPolicy();
    eventWrrPolicy.Reset();
    BackendEvent eventFlush = BackendEvent_Flush;
    BackendEvent eventGc = BackendEvent_GC;
    BackendEvent eventUserRebuild = BackendEvent_UserdataRebuild;
    BackendEvent eventMetaRebuild = BackendEvent_MetadataRebuild;
    BackendEvent eventFrontIo = BackendEvent_FrontendIO;
    BackendEvent eventMetaIo = BackendEvent_MetaIO;
    eventWrrPolicy.SetCorrectionType(eventFlush, QosCorrectionDir_Increase);
    eventWrrPolicy.SetCorrectionType(eventGc, QosCorrectionDir_Increase2X);
    eventWrrPolicy.SetCorrectionType(eventUserRebuild, QosCorrectionDir_Increase4X);
    eventWrrPolicy.SetCorrectionType(eventMetaRebuild, QosCorrectionDir_Decrease);
    eventWrrPolicy.SetCorrectionType(eventFrontIo, QosCorrectionDir_Decrease2X);
    eventWrrPolicy.SetCorrectionType(eventMetaIo, QosCorrectionDir_Decrease4X);
    qosCorrectionManager.Execute();
    eventWrrPolicy.SetCorrectionType(eventFlush, QosCorrectionDir_PriorityHighest);
    eventWrrPolicy.SetCorrectionType(eventGc, QosCorrectionDir_PriorityHigher);
    eventWrrPolicy.SetCorrectionType(eventUserRebuild, QosCorrectionDir_PriorityHigh);
    eventWrrPolicy.SetCorrectionType(eventMetaRebuild, QosCorrectionDir_PriorityMedium);
    eventWrrPolicy.SetCorrectionType(eventFrontIo, QosCorrectionDir_PriorityLow);
    eventWrrPolicy.SetCorrectionType(eventMetaIo, QosCorrectionDir_PriorityLower);
    qosCorrectionManager.Execute();
    eventWrrPolicy.SetCorrectionType(eventFlush, QosCorrectionDir_PriorityLowest);
    eventWrrPolicy.SetCorrectionType(eventGc, QosCorrectionDir_Reset);
    qosCorrectionManager.Execute();
}
TEST(QosCorrectionManager, Check_Execute_Wrr_Policy_Reset)
{
    NiceMock<MockQosContext> mockQosContext;
    NiceMock<MockQosManager> mockQosManager;
    QosCorrectionManager qosCorrectionManager(&mockQosContext, &mockQosManager);
    QosCorrection& correction = mockQosContext.GetQosCorrection();
    correction.SetCorrectionType(QosCorrection_EventWrr, true);
    QosEventWrrWeight& eventWrrPolicy = correction.GetEventWrrWeightPolicy();
    eventWrrPolicy.Reset();
    qosCorrectionManager.Execute();
}
TEST(QosCorrectionManager, Check_Execute_Wrr_Policy_Default)
{
    NiceMock<MockQosContext> mockQosContext;
    NiceMock<MockQosManager> mockQosManager;
    QosCorrectionManager qosCorrectionManager(&mockQosContext, &mockQosManager);
    QosCorrection& correction = mockQosContext.GetQosCorrection();
    correction.SetCorrectionType(QosCorrection_EventWrr, true);
    QosEventWrrWeight& eventWrrPolicy = correction.GetEventWrrWeightPolicy();
    eventWrrPolicy.Reset();
    BackendEvent eventFlush = BackendEvent_Flush;
    eventWrrPolicy.SetCorrectionType(eventFlush, QosCorrectionDir_Unknown);
    qosCorrectionManager.Execute();
}

TEST(QosCorrectionManager, ResetTest)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    QosCorrectionManager qosCorrectionManager(&mockQoscontext, &mockQosManager);
    qosCorrectionManager.Reset();
}

} // namespace pos

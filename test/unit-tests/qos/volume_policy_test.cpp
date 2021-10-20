#include "src/qos/volume_policy.h"

#include "test/unit-tests/qos/qos_context_mock.h"

#include <gtest/gtest.h>

using ::testing::NiceMock;
using ::testing::ReturnRef;
using ::testing::Return;

namespace pos
{
TEST(VolumePolicy, VolumePolicy_Constructor_One_Stack)
{
    NiceMock<MockQosContext> qosContext;
    VolumePolicy volumePolicy(&qosContext);
}

TEST(VolumePolicy, VolumePolicy_Constructor_One_Heap)
{
    NiceMock<MockQosContext> qosContext;
    VolumePolicy* volPolicy = new VolumePolicy(&qosContext);
    delete volPolicy;
}
TEST(VolumePolicy, HandlePolicy_NoMinVolume)
{
    NiceMock<MockQosContext> mockQosContext;
    std::map<uint32_t, uint32_t> &activeVolMap = mockQosContext.GetActiveVolumes();
    activeVolMap[1] = 1;
    QosUserPolicy currUserPolicy;
    ON_CALL(mockQosContext, GetQosUserPolicy()).WillByDefault(ReturnRef(currUserPolicy));
    QosParameters parameters;
    ON_CALL(mockQosContext, GetQosParameters()).WillByDefault(ReturnRef(parameters));

    VolumePolicy volPolicy(&mockQosContext);
    volPolicy.HandlePolicy();
}

TEST(VolumePolicy, HandlePolicy_MinVolume_CorrectionCycleNotOver)
{
    NiceMock<MockQosContext> mockQosContext;
    ON_CALL(mockQosContext, IsCorrectionCycleOver).WillByDefault(Return(false));
    std::map<uint32_t, uint32_t> &activeVolMap = mockQosContext.GetActiveVolumes();
    activeVolMap[1] = 1;
    QosUserPolicy currUserPolicy;
    AllVolumeUserPolicy& currAllVolumePolicy = currUserPolicy.GetAllVolumeUserPolicy();
    VolumeUserPolicy volUserPolicy;
    volUserPolicy.SetMinIops(10);
    currAllVolumePolicy.InsertVolumeUserPolicy(0, 1, volUserPolicy);
    currAllVolumePolicy.SetMinimumGuaranteeVolume(1, 0);
    ON_CALL(mockQosContext, GetQosUserPolicy()).WillByDefault(ReturnRef(currUserPolicy));
    QosParameters parameters;
    ON_CALL(mockQosContext, GetQosParameters()).WillByDefault(ReturnRef(parameters));
    VolumePolicy volPolicy(&mockQosContext);
    volPolicy.HandlePolicy();
}

TEST(VolumePolicy, HandlePolicy_MinVolume_CorrectionCycleOver_VolNotExistVolParam)
{
    NiceMock<MockQosContext> mockQosContext;
    ON_CALL(mockQosContext, IsCorrectionCycleOver).WillByDefault(Return(true));
    std::map<uint32_t, uint32_t> &activeVolumeMap = mockQosContext.GetActiveVolumes();
    activeVolumeMap[1] = 1;

    QosUserPolicy currUserPolicy;
    AllVolumeUserPolicy& allVolUserPolicy = currUserPolicy.GetAllVolumeUserPolicy();
    VolumeUserPolicy volUserPolicy;
    volUserPolicy.SetMinIops(10);
    allVolUserPolicy.InsertVolumeUserPolicy(0, 1, volUserPolicy);
    allVolUserPolicy.SetMinimumGuaranteeVolume(1, 0);
    ON_CALL(mockQosContext, GetQosUserPolicy()).WillByDefault(ReturnRef(currUserPolicy));

    QosParameters parameters;
    AllVolumeParameter& allVolumeParam = parameters.GetAllVolumeParameter();

    ON_CALL(mockQosContext, GetQosParameters()).WillByDefault(ReturnRef(parameters));
    VolumePolicy volPolicy(&mockQosContext);
    volPolicy.HandlePolicy();
    volPolicy.HandlePolicy();
}

TEST(VolumePolicy, HandlePolicy_MinVolume_CorrectionCycleOver_VolExistsVolParamAvgBwNonZero)
{
    NiceMock<MockQosContext> mockQosContext;
    ON_CALL(mockQosContext, IsCorrectionCycleOver).WillByDefault(Return(true));
    std::map<uint32_t, uint32_t> &activeVolumeMap = mockQosContext.GetActiveVolumes();
    activeVolumeMap[1] = 1;

    uint32_t arrayId = 0;
    uint32_t volId = 1;
    QosUserPolicy currUserPolicy;
    AllVolumeUserPolicy& allVolUserPolicy = currUserPolicy.GetAllVolumeUserPolicy();
    VolumeUserPolicy volUserPolicy;
    volUserPolicy.SetMinIops(100);
    volUserPolicy.SetMinBandwidth(1000);
    allVolUserPolicy.InsertVolumeUserPolicy(0, 1, volUserPolicy);
    allVolUserPolicy.SetMinimumGuaranteeVolume(1, 0);
    ON_CALL(mockQosContext, GetQosUserPolicy()).WillByDefault(ReturnRef(currUserPolicy));

    QosParameters parameters;
    AllVolumeParameter& allVolumeParam = parameters.GetAllVolumeParameter();
    VolumeParameter volParam;
    volParam.SetAvgBandwidth(100);
    volParam.SetAvgIops(100);
    allVolumeParam.InsertVolumeParameter(arrayId, volId, volParam);
    ON_CALL(mockQosContext, GetQosParameters()).WillByDefault(ReturnRef(parameters));

    VolumePolicy volPolicy(&mockQosContext);
    volPolicy.HandlePolicy();
    volPolicy.HandlePolicy();
}

TEST(VolumePolicy, HandlePolicy_MinVolume_CorrectionCycleOver)
{
    NiceMock<MockQosContext> mockQosContext;
    std::map<uint32_t, uint32_t> activeVolumeMap;
    ON_CALL(mockQosContext, IsCorrectionCycleOver).WillByDefault(Return(true));
    activeVolumeMap = mockQosContext.GetActiveVolumes();
    activeVolumeMap[1] = 1;

    QosUserPolicy currUserPolicy;
    AllVolumeUserPolicy& allVolUserPolicy = currUserPolicy.GetAllVolumeUserPolicy();
    allVolUserPolicy.SetMinimumGuaranteeVolume(1, 0);
    ON_CALL(mockQosContext, GetQosUserPolicy()).WillByDefault(ReturnRef(currUserPolicy));

    QosParameters parameters;
    AllVolumeParameter& allVolumeParam = parameters.GetAllVolumeParameter();
    VolumeParameter volParam;
    volParam.IncreaseIops(10);
    volParam.SetAvgBandwidth(100);
    volParam.SetAvgIops(10);
    allVolumeParam.InsertVolumeParameter(0, 1, volParam);
    ON_CALL(mockQosContext, GetQosParameters()).WillByDefault(ReturnRef(parameters));
    VolumePolicy volPolicy(&mockQosContext);
    volPolicy.HandlePolicy();
    volPolicy.HandlePolicy();
}

} // namespace pos

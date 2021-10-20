#include "src/qos/throttling_policy_deficit.h"

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
TEST(ThrottlingPolicyDeficit, Constructor_One_Stack)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    ThrottlingPolicyDeficit throttlingPolicy(&mockQoscontext, &mockQosManager);
}

TEST(ThrottlingPolicyDeficit, Constructor_One_Heap)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    ThrottlingPolicyDeficit* throttlingPolicy = new ThrottlingPolicyDeficit(&mockQoscontext, &mockQosManager);
    delete throttlingPolicy;
}
TEST(ThrottlingPolicyDeficit, Test_GetNewWeight_VolumeUserPolicyNull)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    uint32_t arrayId = 0;
    uint32_t volId = 1;

    QosUserPolicy currUserPolicy;
    AllVolumeUserPolicy& currAllVolumePolicy = currUserPolicy.GetAllVolumeUserPolicy();
    ON_CALL(mockQoscontext, GetQosUserPolicy()).WillByDefault(ReturnRef(currUserPolicy));

    QosParameters parameters;
    AllVolumeParameter& allVolumeParam = parameters.GetAllVolumeParameter();
    VolumeParameter volParam;
    volParam.Reset();
    allVolumeParam.InsertVolumeParameter(arrayId, volId, volParam);
    bool actual = allVolumeParam.VolumeExists(arrayId, volId);
    ASSERT_EQ(actual, true);
    ON_CALL(mockQoscontext, GetQosParameters()).WillByDefault(ReturnRef(parameters));

    ThrottlingPolicyDeficit throttlingPolicy(&mockQoscontext, &mockQosManager);
    VolumeThrottle volumeThrottle;
    unsigned int retVal = throttlingPolicy.GetNewWeight(volId, arrayId, &volumeThrottle);
    ASSERT_EQ(retVal, -1);
}
TEST(ThrottlingPolicyDeficit, Test_GetNewWeight_VolumeUserPolicyNotNull)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    uint32_t arrayId = 0;
    uint32_t volId = 1;
    QosUserPolicy currUserPolicy;
    AllVolumeUserPolicy& currAllVolumePolicy = currUserPolicy.GetAllVolumeUserPolicy();
    VolumeUserPolicy volUserPolicy;
    volUserPolicy.SetMaxBandwidth(100);
    volUserPolicy.SetMaxIops(100);
    volUserPolicy.SetMinBandwidth(10);
    volUserPolicy.SetMinIops(0);
    currAllVolumePolicy.InsertVolumeUserPolicy(arrayId, volId, volUserPolicy);
    ON_CALL(mockQoscontext, GetQosUserPolicy()).WillByDefault(ReturnRef(currUserPolicy));
    QosParameters parameters;
    AllVolumeParameter& allVolumeParam = parameters.GetAllVolumeParameter();
    VolumeParameter volParam;
    volParam.Reset();
    allVolumeParam.InsertVolumeParameter(arrayId, volId, volParam);
    bool actual = allVolumeParam.VolumeExists(arrayId, volId);
    ASSERT_EQ(actual, true);
    ON_CALL(mockQoscontext, GetQosParameters()).WillByDefault(ReturnRef(parameters));
    ThrottlingPolicyDeficit throttlingPolicy(&mockQoscontext, &mockQosManager);
    VolumeThrottle volumeThrottle;
    unsigned int retVal = throttlingPolicy.GetNewWeight(volId, arrayId, &volumeThrottle);
    ASSERT_EQ(retVal, 0);
}

TEST(ThrottlingPolicyDeficit, Test_GetNewWeight_VolumeUserPolicyMinBWSet)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    std::map<uint32_t, map<uint32_t, uint32_t>> volReactorMap = mockQoscontext.GetActiveVolumeReactors();
    volReactorMap[1][1] = 1;
    std::map<uint32_t, uint32_t> &activeVolMap = mockQoscontext.GetActiveVolumes();
    activeVolMap[1] = 1;
    uint32_t arrayId = 0;
    uint32_t volId = 1;

    QosUserPolicy currUserPolicy;
    AllVolumeUserPolicy& currAllVolumePolicy = currUserPolicy.GetAllVolumeUserPolicy();
    VolumeUserPolicy volPolicy;
    volPolicy.SetMaxBandwidth(100);
    volPolicy.SetMaxIops(100);
    volPolicy.SetMinBandwidth(10);
    volPolicy.SetMinIops(0);
    currAllVolumePolicy.InsertVolumeUserPolicy(arrayId, volId, volPolicy);
    ON_CALL(mockQoscontext, GetQosUserPolicy()).WillByDefault(ReturnRef(currUserPolicy));

    QosParameters parameters;
    AllVolumeParameter& allVolumeParam = parameters.GetAllVolumeParameter();
    VolumeParameter volParam;
    volParam.IncreaseIops(10);
    volParam.SetAvgBandwidth(100);
    volParam.SetAvgIops(100);
    allVolumeParam.InsertVolumeParameter(arrayId, volId, volParam);
    ON_CALL(mockQoscontext, GetQosParameters()).WillByDefault(ReturnRef(parameters));

    ThrottlingPolicyDeficit throttlingPolicy(&mockQoscontext, &mockQosManager);
    VolumeThrottle volumeThrottle;
    volumeThrottle.SetCorrectionType(QosCorrectionDir_Increase);
    unsigned int retVal = throttlingPolicy.GetNewWeight(volId, arrayId, &volumeThrottle);
    // ASSERT_EQ(retVal, -1);
}

TEST(ThrottlingPolicyDeficit, Test_GetNewWeight_VolumeUserPolicyMaxBWSet_NineteenCycles_IncreaseCorrection)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    std::map<uint32_t, map<uint32_t, uint32_t>> &volReactorMap = mockQoscontext.GetActiveVolumeReactors();
    volReactorMap[1][1] = 1;
    std::map<uint32_t, uint32_t> &activeVolMap = mockQoscontext.GetActiveVolumes();
    activeVolMap[1] = 1;
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
    ThrottlingPolicyDeficit throttlingPolicy(&mockQoscontext, &mockQosManager);
    for (int i = 0; i < 19; i++)
    {
        throttlingPolicy.IncrementCycleCount();
    }
    VolumeThrottle volumeThrottle;
    volumeThrottle.SetCorrectionType(QosCorrectionDir_Increase);
    unsigned int retVal = throttlingPolicy.GetNewWeight(volId, arrayId, &volumeThrottle);
}

TEST(ThrottlingPolicyDeficit, Test_GetNewWeight_VolumeUserPolicyMaxBWSet_TwentyCycles_MaxLimitCorrection)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    std::map<uint32_t, map<uint32_t, uint32_t>> &volReactorMap = mockQoscontext.GetActiveVolumeReactors();
    volReactorMap[1][1] = 1;
    std::map<uint32_t, uint32_t> &activeVolMap = mockQoscontext.GetActiveVolumes();
    activeVolMap[1] = 1;
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
    ThrottlingPolicyDeficit throttlingPolicy(&mockQoscontext, &mockQosManager);
    // noOfCycles == GUARANTEE_CYCLES (20)
    for (int i = 0; i < 20; i++)
    {
        throttlingPolicy.IncrementCycleCount();
    }
    VolumeThrottle volumeThrottle;
    volumeThrottle.SetCorrectionType(QosCorrectionDir_SetMaxLimit);
    unsigned int retVal = throttlingPolicy.GetNewWeight(volId, arrayId, &volumeThrottle);
    // call it again so that beginAgain becomes false for the next run
    retVal = throttlingPolicy.GetNewWeight(volId, arrayId, &volumeThrottle);
    // call it again so that resetFlag becomes false for the next run
    retVal = throttlingPolicy.GetNewWeight(volId, arrayId, &volumeThrottle);
}

TEST(ThrottlingPolicyDeficit, GetCorrectionType_Test)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    ThrottlingPolicyDeficit throttlingPolicy(&mockQoscontext, &mockQosManager);
    bool actual, expected = false;
    uint32_t volId = 1;
    uint32_t arrayId = 1;
    actual = throttlingPolicy.GetCorrectionType(volId, arrayId);
    ASSERT_EQ(actual, expected);
}

TEST(ThrottlingPolicyDeficit, Reset_Test)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    ThrottlingPolicyDeficit throttlingPolicy(&mockQoscontext, &mockQosManager);
    throttlingPolicy.Reset();
}

TEST(ThrottlingPolicyDeficit, IncrementCycleCount_Test)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    ThrottlingPolicyDeficit throttlingPolicy(&mockQoscontext, &mockQosManager);
    throttlingPolicy.IncrementCycleCount();
}

TEST(ThrottlingPolicyDeficit, Test_GetNewWeight_VolumeUserPolicyMinIopsSet)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    std::map<uint32_t, map<uint32_t, uint32_t>> volReactorMap = mockQoscontext.GetActiveVolumeReactors();
    volReactorMap[1][1] = 1;
    std::map<uint32_t, uint32_t> &activeVolMap = mockQoscontext.GetActiveVolumes();
    activeVolMap[1] = 1;
    uint32_t arrayId = 0;
    uint32_t volId = 1;

    QosUserPolicy currUserPolicy;
    AllVolumeUserPolicy& currAllVolumePolicy = currUserPolicy.GetAllVolumeUserPolicy();
    VolumeUserPolicy volPolicy;
    volPolicy.SetMaxBandwidth(100);
    volPolicy.SetMaxIops(100);
    volPolicy.SetMinBandwidth(0);
    volPolicy.SetMinIops(10);
    currAllVolumePolicy.InsertVolumeUserPolicy(arrayId, volId, volPolicy);
    ON_CALL(mockQoscontext, GetQosUserPolicy()).WillByDefault(ReturnRef(currUserPolicy));

    QosParameters parameters;
    AllVolumeParameter& allVolumeParam = parameters.GetAllVolumeParameter();
    VolumeParameter volParam;
    volParam.IncreaseIops(10);
    volParam.SetAvgBandwidth(100);
    volParam.SetAvgIops(100);
    allVolumeParam.InsertVolumeParameter(arrayId, volId, volParam);
    ON_CALL(mockQoscontext, GetQosParameters()).WillByDefault(ReturnRef(parameters));

    ThrottlingPolicyDeficit throttlingPolicy(&mockQoscontext, &mockQosManager);
    VolumeThrottle volumeThrottle;
    volumeThrottle.SetCorrectionType(QosCorrectionDir_Increase);
    unsigned int retVal = throttlingPolicy.GetNewWeight(volId, arrayId, &volumeThrottle);
    // ASSERT_EQ(retVal, -1);
}

TEST(ThrottlingPolicyDeficit, Test_GetNewWeight_VolumeUserCorrectionNoChange)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    std::map<uint32_t, map<uint32_t, uint32_t>> &volReactorMap = mockQoscontext.GetActiveVolumeReactors();
    volReactorMap[1][1] = 1;
    std::map<uint32_t, uint32_t> &activeVolMap = mockQoscontext.GetActiveVolumes();
    activeVolMap[1] = 1;
    uint32_t arrayId = 0;
    uint32_t volId = 1;

    QosUserPolicy currUserPolicy;
    AllVolumeUserPolicy& currAllVolumePolicy = currUserPolicy.GetAllVolumeUserPolicy();
    VolumeUserPolicy volPolicy;
    volPolicy.SetMaxBandwidth(100);
    volPolicy.SetMinBandwidth(10);
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
    ThrottlingPolicyDeficit throttlingPolicy(&mockQoscontext, &mockQosManager);
    // noOfCycle == 19 (GUARANTEE_CYCLES - 1)
    for (int i = 0; i < 20; i++)
    {
        throttlingPolicy.IncrementCycleCount();
    }
    VolumeThrottle volumeThrottle;
    volumeThrottle.SetCorrectionType(QosCorrectionDir_NoChange);
    unsigned int retVal = throttlingPolicy.GetNewWeight(volId, arrayId, &volumeThrottle);
    // ASSERT_EQ(retVal, -1);
}
TEST(ThrottlingPolicyDeficit, Test_GetNewWeight_VolumeUserPolicyMaxBWSet_TwentyCycles_DecreaseCorrection)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    std::map<uint32_t, map<uint32_t, uint32_t>> &volReactorMap = mockQoscontext.GetActiveVolumeReactors();
    volReactorMap[1][1] = 1;
    std::map<uint32_t, uint32_t> &activeVolMap = mockQoscontext.GetActiveVolumes();
    activeVolMap[1] = 1;
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
    ThrottlingPolicyDeficit throttlingPolicy(&mockQoscontext, &mockQosManager);
    // noOfCycles == GUARANTEE_CYCLES (20)
    for (int i = 0; i < 20; i++)
    {
        throttlingPolicy.IncrementCycleCount();
    }
    VolumeThrottle volumeThrottle;
    volumeThrottle.SetCorrectionType(QosCorrectionDir_Decrease);
    unsigned int retVal = throttlingPolicy.GetNewWeight(volId, arrayId, &volumeThrottle);
    // call it again so that beginAgain becomes false for the next run
    retVal = throttlingPolicy.GetNewWeight(volId, arrayId, &volumeThrottle);
}
} // namespace pos

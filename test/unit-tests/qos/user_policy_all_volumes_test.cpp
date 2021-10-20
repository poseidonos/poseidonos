#include "src/qos/user_policy_all_volumes.h"

#include <gtest/gtest.h>

#include "src/qos/qos_common.h"
#include "test/unit-tests/qos/user_policy_volume_mock.h"

using ::testing::NiceMock;

namespace pos
{
TEST(AllVolumeUserPolicy, AllVolumeUserPolicy_Constructor_One_Stack)
{
    AllVolumeUserPolicy allVolUserPolicy();
}

TEST(AllVolumeUserPolicy, AllVolumeUserPolicy_Constructor_One_Heap)
{
    AllVolumeUserPolicy* allVolUserPolicy = new AllVolumeUserPolicy();
    delete allVolUserPolicy;
}

TEST(AllVolumeUserPolicy, Reset_Test)
{
    AllVolumeUserPolicy allVolUserPolicy;
    allVolUserPolicy.Reset();
    std::vector<std::pair<uint32_t, uint32_t>> setVolumes = allVolUserPolicy.GetMinimumGuaranteeVolume();
    ASSERT_EQ(setVolumes.size(), 0);
    bool setMinBw = allVolUserPolicy.IsMinPolicyInEffect();
    ASSERT_EQ(setMinBw, false);
    bool setMinBwGuarantee = allVolUserPolicy.IsMinBwPolicyInEffect();
    ASSERT_EQ(setMinBwGuarantee, false);
    bool setMaxThrottling = allVolUserPolicy.IsMaxThrottlingChanged();
    ASSERT_EQ(setMaxThrottling, false);
}

TEST(AllVolumeUserPolicy, Test_Insert_And_Get_VolumeUserPolicy)
{
    AllVolumeUserPolicy allVolUserPolicy;
    uint32_t vol = 1;
    uint32_t arrayId = 1;
    NiceMock<MockVolumeUserPolicy> volumeUserPolicy;
    volumeUserPolicy.Reset();
    allVolUserPolicy.InsertVolumeUserPolicy(arrayId, vol, volumeUserPolicy);
    VolumeUserPolicy* volumeUserPolicyReceived;
    volumeUserPolicyReceived = allVolUserPolicy.GetVolumeUserPolicy(arrayId, vol);
    ASSERT_EQ(volumeUserPolicyReceived->GetMinBandwidth(), volumeUserPolicy.GetMinBandwidth());
    ASSERT_EQ(volumeUserPolicyReceived->GetMinIops(), volumeUserPolicy.GetMinIops());
    ASSERT_EQ(volumeUserPolicyReceived->GetMaxBandwidth(), volumeUserPolicy.GetMaxBandwidth());
    ASSERT_EQ(volumeUserPolicyReceived->GetMaxIops(), volumeUserPolicy.GetMaxIops());
}

TEST(AllVolumeUserPolicy, Test_Get_And_SetMinimumGuaranteeVolume)
{
    AllVolumeUserPolicy allVolUserPolicy;
    uint32_t vol = 1;
    uint32_t arrayId = 1;
    allVolUserPolicy.SetMinimumGuaranteeVolume(vol, arrayId);
    std::vector<std::pair<uint32_t, uint32_t>> setVol = allVolUserPolicy.GetMinimumGuaranteeVolume();
    ASSERT_EQ(vol, setVol[0].first);
    ASSERT_EQ(arrayId, setVol[0].second);
    // call it again, it would return from first if
    allVolUserPolicy.SetMinimumGuaranteeVolume(vol, arrayId);
}

TEST(AllVolumeUserPolicy, Test_Get_And_SetMinimumPolicyInEffect_True)
{
    AllVolumeUserPolicy allVolUserPolicy;
    bool minBw = true;
    allVolUserPolicy.SetMinimumPolicyInEffect(minBw);
    bool setMinBw = allVolUserPolicy.IsMinPolicyInEffect();
    ASSERT_EQ(minBw, setMinBw);
}

TEST(AllVolumeUserPolicy, Test_Get_And_SetMinimumPolicyType_)
{
    AllVolumeUserPolicy allVolUserPolicy;
    bool minBwGuarantee = false;
    allVolUserPolicy.SetMinimumPolicyType(minBwGuarantee);
    bool setMinBwGuarantee = allVolUserPolicy.IsMinBwPolicyInEffect();
    ASSERT_EQ(minBwGuarantee, setMinBwGuarantee);
}

TEST(AllVolumeUserPolicy, Test_Get_And_SetMaxThrottlingChanged)
{
    AllVolumeUserPolicy allVolUserPolicy;
    bool maxThrottling = true;
    allVolUserPolicy.SetMaxThrottlingChanged(maxThrottling);
    bool setMaxThrottling = allVolUserPolicy.IsMaxThrottlingChanged();
    ASSERT_EQ(maxThrottling, setMaxThrottling);
}

TEST(AllVolumeUserPolicy, EqualOperator_Test)
{
    AllVolumeUserPolicy allVolUserPolicy1;
    AllVolumeUserPolicy allVolUserPolicy2;
    bool equal = (allVolUserPolicy1 == allVolUserPolicy2);
    ASSERT_EQ(equal, true);
    VolumeUserPolicy userPolicy;
    userPolicy.SetMinBandwidth(100);
    allVolUserPolicy1.InsertVolumeUserPolicy(0, 1, userPolicy);
    equal = (allVolUserPolicy1 == allVolUserPolicy2);
    ASSERT_EQ(equal, false);
    allVolUserPolicy2.InsertVolumeUserPolicy(0, 1, userPolicy);
    equal = (allVolUserPolicy1 == allVolUserPolicy2);
    ASSERT_EQ(equal, true);
    userPolicy.SetMinBandwidth(200);
    allVolUserPolicy1.InsertVolumeUserPolicy(0, 1, userPolicy);
    equal = (allVolUserPolicy1 == allVolUserPolicy2);
    ASSERT_EQ(equal, false);
    allVolUserPolicy2.InsertVolumeUserPolicy(0, 1, userPolicy);
    equal = (allVolUserPolicy1 == allVolUserPolicy2);
    ASSERT_EQ(equal, true);
}

} // namespace pos

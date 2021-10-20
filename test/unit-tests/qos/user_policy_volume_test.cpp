#include "src/qos/user_policy_volume.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(VolumeUserPolicy, VolumeUserPolicy_Constructor_One_Stack)
{
    VolumeUserPolicy volumeUserPolicy();
}

TEST(VolumeUserPolicy, VolumeUserPolicy_Constructor_One_Heap)
{
    VolumeUserPolicy* volUserPolicy = new VolumeUserPolicy();
    delete volUserPolicy;
}

TEST(VolumeUserPolicy, Reset_Test)
{
    VolumeUserPolicy volumeUserPolicy;
    volumeUserPolicy.Reset();
    uint64_t receivedMinBw = volumeUserPolicy.GetMinBandwidth();
    uint64_t receivedMaxBw = volumeUserPolicy.GetMaxBandwidth();
    uint64_t receivedMinIops = volumeUserPolicy.GetMinIops();
    uint64_t receivedMaxIops = volumeUserPolicy.GetMaxIops();
    ASSERT_EQ(receivedMinBw, 0);
    ASSERT_EQ(receivedMaxBw, 0);
    ASSERT_EQ(receivedMinIops, 0);
    ASSERT_EQ(receivedMaxIops, 0);
}

TEST(VolumeUserPolicy, GetSetMinBandwidth_Test)
{
    VolumeUserPolicy volumeUserPolicy;
    uint64_t minBw = 5;
    volumeUserPolicy.SetMinBandwidth(minBw);
    uint64_t receivedBw = volumeUserPolicy.GetMinBandwidth();
    ASSERT_EQ(receivedBw, minBw);
    uint64_t minBw2 = DEFAULT_MIN_BW_MBPS * (M_KBYTES * M_KBYTES / (PARAMETER_COLLECTION_INTERVAL));
    volumeUserPolicy.SetMinBandwidth(minBw2);
}

TEST(VolumeUserPolicy, GetSetMaxBandwidth_Test)
{
    VolumeUserPolicy volumeUserPolicy;
    uint64_t maxBw = 5;
    volumeUserPolicy.SetMaxBandwidth(maxBw);
    uint64_t receivedBw = volumeUserPolicy.GetMaxBandwidth();
    ASSERT_EQ(receivedBw, maxBw);
}

TEST(VolumeUserPolicy, GetSetMaxIops_Test)
{
    VolumeUserPolicy volumeUserPolicy;
    uint64_t maxIops = 10;
    volumeUserPolicy.SetMaxIops(maxIops);
    uint64_t receivedIops = volumeUserPolicy.GetMaxIops();
    ASSERT_EQ(receivedIops, maxIops);
}

TEST(VolumeUserPolicy, GetSetMinIops_Test)
{
    VolumeUserPolicy volumeUserPolicy;
    uint64_t minIops = 10;
    volumeUserPolicy.SetMinIops(minIops);
    uint64_t receivedIops = volumeUserPolicy.GetMinIops();
    ASSERT_EQ(receivedIops, minIops);
    volumeUserPolicy.SetMinIops(1000);
}

TEST(VolumeUserPolicy, EqualOperatorOverload_Test)
{
    VolumeUserPolicy volumeUserPolicy1;
    VolumeUserPolicy volumeUserPolicy2;
    uint64_t minIops = 10;
    uint64_t maxIops = 100;
    uint64_t minBandwidth = 20;
    uint64_t maxBandwidth = 200;
    volumeUserPolicy1.SetMinIops(minIops);
    volumeUserPolicy1.SetMaxIops(maxIops);
    volumeUserPolicy1.SetMinBandwidth(minBandwidth);
    volumeUserPolicy1.SetMaxBandwidth(maxBandwidth);
    volumeUserPolicy2.SetMinIops(minIops);
    volumeUserPolicy2.SetMaxIops(maxIops);
    volumeUserPolicy2.SetMinBandwidth(minBandwidth);
    volumeUserPolicy2.SetMaxBandwidth(maxBandwidth);
    bool equal = (volumeUserPolicy1 == volumeUserPolicy2);
    ASSERT_EQ(equal, true);
}
TEST(VolumeUserPolicy, EqualOperatorOverload_MinBw_Unequal_Test)
{
    VolumeUserPolicy volumeUserPolicy1;
    VolumeUserPolicy volumeUserPolicy2;
    uint64_t minIops = 10;
    uint64_t maxIops = 100;
    uint64_t minBandwidth1 = 20;
    uint64_t minBandwidth2 = 200;
    uint64_t maxBandwidth = 200;
    volumeUserPolicy1.SetMinIops(minIops);
    volumeUserPolicy1.SetMaxIops(maxIops);
    volumeUserPolicy1.SetMinBandwidth(minBandwidth1);
    volumeUserPolicy1.SetMaxBandwidth(maxBandwidth);
    volumeUserPolicy2.SetMinIops(minIops);
    volumeUserPolicy2.SetMaxIops(maxIops);
    volumeUserPolicy2.SetMinBandwidth(minBandwidth2);
    volumeUserPolicy2.SetMaxBandwidth(maxBandwidth);
    bool equal = (volumeUserPolicy1 == volumeUserPolicy2);
    ASSERT_EQ(equal, false);
}
TEST(VolumeUserPolicy, EqualOperatorOverload_MaxBw_Unequal_Test)
{
    VolumeUserPolicy volumeUserPolicy1;
    VolumeUserPolicy volumeUserPolicy2;
    uint64_t minIops = 10;
    uint64_t maxIops = 100;
    uint64_t minBandwidth = 20;
    uint64_t maxBandwidth1 = 200;
    uint64_t maxBandwidth2 = 2000;
    volumeUserPolicy1.SetMinIops(minIops);
    volumeUserPolicy1.SetMaxIops(maxIops);
    volumeUserPolicy1.SetMinBandwidth(minBandwidth);
    volumeUserPolicy1.SetMaxBandwidth(maxBandwidth1);
    volumeUserPolicy2.SetMinIops(minIops);
    volumeUserPolicy2.SetMaxIops(maxIops);
    volumeUserPolicy2.SetMinBandwidth(minBandwidth);
    volumeUserPolicy2.SetMaxBandwidth(maxBandwidth2);
    bool equal = (volumeUserPolicy1 == volumeUserPolicy2);
    ASSERT_EQ(equal, false);
}
TEST(VolumeUserPolicy, EqualOperatorOverload_MinIops_Unequal_Test)
{
    VolumeUserPolicy volumeUserPolicy1;
    VolumeUserPolicy volumeUserPolicy2;
    uint64_t minIops1 = 10;
    uint64_t minIops2 = 100;
    uint64_t maxIops = 100;
    uint64_t minBandwidth = 20;
    uint64_t maxBandwidth = 200;
    volumeUserPolicy1.SetMinIops(minIops1);
    volumeUserPolicy1.SetMaxIops(maxIops);
    volumeUserPolicy1.SetMinBandwidth(minBandwidth);
    volumeUserPolicy1.SetMaxBandwidth(maxBandwidth);
    volumeUserPolicy2.SetMinIops(minIops2);
    volumeUserPolicy2.SetMaxIops(maxIops);
    volumeUserPolicy2.SetMinBandwidth(minBandwidth);
    volumeUserPolicy2.SetMaxBandwidth(maxBandwidth);
    bool equal = (volumeUserPolicy1 == volumeUserPolicy2);
    ASSERT_EQ(equal, false);
}
TEST(VolumeUserPolicy, EqualOperatorOverload_MaxIops_Unequal_Test)
{
    VolumeUserPolicy volumeUserPolicy1;
    VolumeUserPolicy volumeUserPolicy2;
    uint64_t minIops = 10;
    uint64_t maxIops1 = 100;
    uint64_t maxIops2 = 1000;
    uint64_t minBandwidth = 20;
    uint64_t maxBandwidth = 200;
    volumeUserPolicy1.SetMinIops(minIops);
    volumeUserPolicy1.SetMaxIops(maxIops1);
    volumeUserPolicy1.SetMinBandwidth(minBandwidth);
    volumeUserPolicy1.SetMaxBandwidth(maxBandwidth);
    volumeUserPolicy2.SetMinIops(minIops);
    volumeUserPolicy2.SetMaxIops(maxIops2);
    volumeUserPolicy2.SetMinBandwidth(minBandwidth);
    volumeUserPolicy2.SetMaxBandwidth(maxBandwidth);
    bool equal = (volumeUserPolicy1 == volumeUserPolicy2);
    ASSERT_EQ(equal, false);
}
TEST(VolumeUserPolicy, IsBwPolicySet_Test)
{
    VolumeUserPolicy volumeUserPolicy;
    uint64_t minBw = 10;
    volumeUserPolicy.SetMinBandwidth(minBw);
    bool bwPolicy = volumeUserPolicy.IsBwPolicySet();
    ASSERT_EQ(bwPolicy, true);
    bool isMinVol = volumeUserPolicy.IsMinimumVolume();
    ASSERT_EQ(isMinVol, true);
}
} // namespace pos

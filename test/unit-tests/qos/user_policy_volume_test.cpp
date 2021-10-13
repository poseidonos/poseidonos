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

TEST(VolumeUserPolicy, SetMinBandwidth_Test)
{
    VolumeUserPolicy volumeUserPolicy;
    uint64_t minBw = 5;
    volumeUserPolicy.SetMinBandwidth(minBw);
    uint64_t receivedBw = volumeUserPolicy.GetMinBandwidth();
    ASSERT_EQ(receivedBw, minBw);
}

TEST(VolumeUserPolicy, GetMinBandwidth_Test)
{
    VolumeUserPolicy volumeUserPolicy;
    uint64_t minBw = 5;
    volumeUserPolicy.SetMinBandwidth(minBw);
    uint64_t receivedBw = volumeUserPolicy.GetMinBandwidth();
    ASSERT_EQ(receivedBw, minBw);
}

TEST(VolumeUserPolicy, SetMaxBandwidth_Test)
{
    VolumeUserPolicy volumeUserPolicy;
    uint64_t maxBw = 5;
    volumeUserPolicy.SetMaxBandwidth(maxBw);
    uint64_t receivedBw = volumeUserPolicy.GetMaxBandwidth();
    ASSERT_EQ(receivedBw, maxBw);
}

TEST(VolumeUserPolicy, GetMaxBandwidth_Test)
{
    VolumeUserPolicy volumeUserPolicy;
    uint64_t maxBw = 5;
    volumeUserPolicy.SetMaxBandwidth(maxBw);
    uint64_t receivedBw = volumeUserPolicy.GetMaxBandwidth();
    ASSERT_EQ(receivedBw, maxBw);
}

TEST(VolumeUserPolicy, SetMaxIops_Test)
{
    VolumeUserPolicy volumeUserPolicy;
    uint64_t maxIops = 10;
    volumeUserPolicy.SetMaxIops(maxIops);
    uint64_t receivedIops = volumeUserPolicy.GetMaxIops();
    ASSERT_EQ(receivedIops, maxIops);
}

TEST(VolumeUserPolicy, GetMaxIops_Test)
{
    VolumeUserPolicy volumeUserPolicy;
    uint64_t maxIops = 10;
    volumeUserPolicy.SetMaxIops(maxIops);
    uint64_t receivedIops = volumeUserPolicy.GetMaxIops();
    ASSERT_EQ(receivedIops, maxIops);
}

TEST(VolumeUserPolicy, SetMinIops_Test)
{
    VolumeUserPolicy volumeUserPolicy;
    uint64_t minIops = 10;
    volumeUserPolicy.SetMinIops(minIops);
    uint64_t receivedIops = volumeUserPolicy.GetMinIops();
    ASSERT_EQ(receivedIops, minIops);
}

TEST(VolumeUserPolicy, GetMinIops_Test)
{
    VolumeUserPolicy volumeUserPolicy;
    uint64_t minIops = 10;
    volumeUserPolicy.SetMinIops(minIops);
    uint64_t receivedIops = volumeUserPolicy.GetMinIops();
    ASSERT_EQ(receivedIops, minIops);
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

} // namespace pos

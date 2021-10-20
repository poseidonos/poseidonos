#include "src/qos/parameters_volume.h"

#include <gtest/gtest.h>

#include "src/qos/qos_common.h"
#include "test/unit-tests/qos/parameters_reactor_mock.h"

using ::testing::NiceMock;

namespace pos
{
TEST(VolumeParameter, VolumeParameter_Constructor_One_Stack)
{
    VolumeParameter volParam();
}

TEST(VolumeParameter, VolumeParameter_Constructor_One_Heap)
{
    VolumeParameter* volParam = new VolumeParameter();
    delete volParam;
}

TEST(VolumeParameter, Check_Reset)
{
    VolumeParameter volParam;
    volParam.Reset();
    uint64_t bw = volParam.GetBandwidth();
    ASSERT_EQ(bw, 0);
    uint64_t iops = volParam.GetIops();
    ASSERT_EQ(iops, 0);
    uint64_t avgBw = volParam.GetAvgBandwidth();
    ASSERT_EQ(avgBw, 0);
    uint64_t avgIops = volParam.GetAvgIops();
    ASSERT_EQ(avgIops, 0);
}

TEST(VolumeParameter, Test_IncreaseIops)
{
    VolumeParameter volParam;
    uint64_t iops = 5;
    volParam.Reset();
    volParam.IncreaseIops(iops);
    uint64_t receivedIops = volParam.GetIops();
    ASSERT_EQ(iops, receivedIops);
}

TEST(VolumeParameter, Test_GetBandwidth)
{
    VolumeParameter volParam;
    uint64_t bw = 5;
    volParam.Reset();
    volParam.IncreaseBandwidth(bw);
    uint64_t receivedBw = volParam.GetBandwidth();
    ASSERT_EQ(bw, receivedBw);
}

TEST(VolumeParameter, Test_GetIops)
{
    VolumeParameter volParam;
    uint64_t iops = 5;
    volParam.Reset();
    volParam.IncreaseIops(iops);
    uint64_t receivedIops = volParam.GetIops();
    ASSERT_EQ(iops, receivedIops);
}

TEST(VolumeParameter, Test_SetAvgBandwidth)
{
    VolumeParameter volParam;
    uint64_t avgBw = 5;
    volParam.SetAvgBandwidth(avgBw);
    uint64_t receivedAvgBw = volParam.GetAvgBandwidth();
    ASSERT_EQ(avgBw, receivedAvgBw);
}

TEST(VolumeParameter, Test_SetAvgIops)
{
    VolumeParameter volParam;
    uint64_t avgIops = 5;
    volParam.SetAvgIops(avgIops);
    uint64_t receivedAvgIops = volParam.GetAvgIops();
    ASSERT_EQ(avgIops, receivedAvgIops);
}

TEST(VolumeParameter, Test_GetAvgBandwidth)
{
    VolumeParameter volParam;
    uint64_t avgBw = 5;
    volParam.SetAvgBandwidth(avgBw);
    uint64_t receivedAvgBw = volParam.GetAvgBandwidth();
    ASSERT_EQ(avgBw, receivedAvgBw);
}
TEST(VolumeParameter, Test_GetAvgIops)
{
    VolumeParameter volParam;
    uint64_t avgIops = 5;
    volParam.SetAvgIops(avgIops);
    uint64_t receivedAvgIops = volParam.GetAvgIops();
    ASSERT_EQ(avgIops, receivedAvgIops);
}

TEST(VolumeParameter, Test_IsReactorExists_False)
{
    VolumeParameter volParam;
    uint32_t reactor = 0;
    bool exists = volParam.IsReactorExists(reactor);
    bool expected = false;
    ASSERT_EQ(exists, expected);
}

TEST(VolumeParameter, Test_IsReactorExists_True)
{
    VolumeParameter volParam;
    uint32_t reactor = 0;
    NiceMock<MockReactorParameter> reactorParameter;
    reactorParameter.Reset();
    volParam.InsertReactorParameter(reactor, reactorParameter);
    bool exists = volParam.IsReactorExists(reactor);
    bool expected = true;
    ASSERT_EQ(exists, expected);
}
TEST(VolumeParameter, Test_GetReactorParameter_Valid)
{
    VolumeParameter volParam;
    uint32_t reactor = 0;
    NiceMock<MockReactorParameter> reactorParameter;
    reactorParameter.Reset();
    volParam.InsertReactorParameter(reactor, reactorParameter);
    ReactorParameter insertedReactorParameter = volParam.GetReactorParameter(reactor);
    insertedReactorParameter.IncreaseBandwidth(0);
    uint64_t bw = insertedReactorParameter.GetBandwidth();
    uint64_t iops = insertedReactorParameter.GetIops();
    ASSERT_EQ(bw, 0);
    ASSERT_EQ(iops, 0);
}

TEST(VolumeParameter, Test_GetReactorParameter_Invalid)
{
    VolumeParameter volParam;
    uint32_t reactor = 0;
    EXPECT_THROW(volParam.GetReactorParameter(reactor), int);
}

TEST(VolumeParameter, Test_InsertReactorParameter)
{
    VolumeParameter volParam;
    uint32_t reactor = 0;
    NiceMock<MockReactorParameter> reactorParameter;
    reactorParameter.Reset();
    volParam.InsertReactorParameter(reactor, reactorParameter);
    ReactorParameter insertedReactorParameter = volParam.GetReactorParameter(reactor);
    insertedReactorParameter.IncreaseBandwidth(0);
    uint64_t bw = insertedReactorParameter.GetBandwidth();
    uint64_t iops = insertedReactorParameter.GetIops();
    ASSERT_EQ(bw, 0);
    ASSERT_EQ(iops, 0);
}

TEST(VolumeParameter, Test_GetReactorParameterMap)
{
    VolumeParameter volParam;
    std::map<uint32_t, ReactorParameter> receivedMap = volParam.GetReactorParameterMap();
}

TEST(VolumeParameter, SetGetBlockSize_Test)
{
    VolumeParameter volParam;
    volParam.SetBlockSize(200);
    uint64_t blkSize = volParam.GetBlockSize();
    ASSERT_EQ(blkSize, 200);
}


} // namespace pos

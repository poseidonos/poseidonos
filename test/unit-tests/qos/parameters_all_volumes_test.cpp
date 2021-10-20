#include "src/qos/parameters_all_volumes.h"

#include <gtest/gtest.h>

#include "src/qos/parameters_volume.h"
#include "test/unit-tests/qos/parameters_volume_mock.h"

using ::testing::NiceMock;

namespace pos
{
TEST(AllVolumeParameter, AllVolumeParameter_Constructor_One_Stack)
{
    AllVolumeParameter allVolParam();
}
TEST(AllVolumeParameter, AllVolumeParameter_Constructor_One_Heap)
{
    AllVolumeParameter* allVolParam = new AllVolumeParameter();
    delete allVolParam;
}

TEST(AllVolumeParameter, Check_Reset)
{
    AllVolumeParameter allVolParam;
    allVolParam.Reset();
}

TEST(AllVolumeParameter, Test_InsertVolumeParameter)
{
    AllVolumeParameter allVolParam;
    uint32_t volId = 1;
    uint32_t arrayId = 1;
    NiceMock<MockVolumeParameter> volParam;
    allVolParam.InsertVolumeParameter(arrayId, volId, volParam);
    VolumeParameter receivedVolParam = allVolParam.GetVolumeParameter(arrayId, volId);
    uint64_t bw = receivedVolParam.GetBandwidth();
    ASSERT_EQ(bw, 0);
    uint64_t iops = receivedVolParam.GetIops();
    ASSERT_EQ(iops, 0);
    uint64_t avgBw = receivedVolParam.GetAvgBandwidth();
    ASSERT_EQ(avgBw, 0);
    uint64_t avgIops = receivedVolParam.GetAvgIops();
    ASSERT_EQ(avgIops, 0);
}
TEST(AllVolumeParameter, Test_VolumeExists_True)
{
    AllVolumeParameter allVolParam;
    uint32_t volId = 1;
    uint32_t arrayId = 1;
    NiceMock<MockVolumeParameter> volParam;
    allVolParam.InsertVolumeParameter(arrayId, volId, volParam);
    bool expected = true;
    bool recd = allVolParam.VolumeExists(arrayId, volId);
    ASSERT_EQ(expected, recd);
}

TEST(AllVolumeParameter, Test_VolumeExists_False)
{
    AllVolumeParameter allVolParam;
    uint32_t volId = 1;
    uint32_t arrayId = 1;
    bool expected = false;
    bool recd = allVolParam.VolumeExists(arrayId, volId);
    ASSERT_EQ(expected, recd);
}

TEST(AllVolumeParameter, Test_GetVolumeParameter_Valid)
{
    AllVolumeParameter allVolParam;
    uint32_t volId = 1;
    uint32_t arrayId = 1;
    NiceMock<MockVolumeParameter> volParam;
    allVolParam.InsertVolumeParameter(arrayId, volId, volParam);
    VolumeParameter receivedVolParam = allVolParam.GetVolumeParameter(arrayId, volId);
    uint64_t bw = receivedVolParam.GetBandwidth();
    ASSERT_EQ(bw, 0);
    uint64_t iops = receivedVolParam.GetIops();
    ASSERT_EQ(iops, 0);
    uint64_t avgBw = receivedVolParam.GetAvgBandwidth();
    ASSERT_EQ(avgBw, 0);
    uint64_t avgIops = receivedVolParam.GetAvgIops();
    ASSERT_EQ(avgIops, 0);
}

TEST(AllVolumeParameter, Test_GetVolumeParameter_Invalid)
{
    AllVolumeParameter allVolParam;
    try
    {
        allVolParam.GetVolumeParameter(1, 99);
    }
    catch(int x)
    {
        ASSERT_EQ(x, 9006);
    }
}

TEST(AllVolumeParameter, SetGetMinVolumeBw_Test)
{
    AllVolumeParameter allVolParameter;
    allVolParameter.IncrementMinVolumesBw(0, 100);
    allVolParameter.IncrementMinVolumesBw(1, 200);
    allVolParameter.IncrementMinVolumesBw(0, 300);
    uint64_t arrayBw = allVolParameter.GetMinVolBw(0);
    ASSERT_EQ(arrayBw, 400);
}

TEST(AllVolumeParameter, SetGetTotalBw_Test)
{
    AllVolumeParameter allVolParameter;
    allVolParameter.IncrementTotalBw(0, 100);
    allVolParameter.IncrementTotalBw(1, 200);
    allVolParameter.IncrementTotalBw(0, 300);
    uint64_t arrayBw = allVolParameter.GetTotalBw(0);
    ASSERT_EQ(arrayBw, 400);
}

} // namespace pos

#include "src/qos/parameters_reactor.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(ReactorParameter, ReactorParameter_Constructor_One_Stack)
{
    ReactorParameter reactorParam();
}

TEST(ReactorParameter, ReactorParameter_Constructor_One_Heap)
{
    ReactorParameter* reactorParam = new ReactorParameter();
    delete reactorParam;
}

TEST(ReactorParameter, Test_Reset_Values)
{
    ReactorParameter reactorParam;
    reactorParam.Reset();
}
TEST(ReactorParameter, Test_IncreaseBandwidth)
{
    ReactorParameter reactorParam;
    reactorParam.Reset();
    uint64_t bw = 5;
    reactorParam.IncreaseBandwidth(bw);
    uint64_t received = reactorParam.GetBandwidth();
    uint64_t expected = 5;
    ASSERT_EQ(received, expected);
}

TEST(ReactorParameter, Test_IncreaseIops)
{
    ReactorParameter reactorParam;
    uint64_t iops = 5;
    reactorParam.IncreaseIops(iops);
    reactorParam.IncreaseBandwidth(0);
    uint64_t received = reactorParam.GetIops();
    uint64_t expected = 5;
    ASSERT_EQ(received, expected);
}

TEST(ReactorParameter, Test_GetBandwidth)
{
    ReactorParameter reactorParam;
    reactorParam.IncreaseBandwidth(0);
    uint64_t bw = reactorParam.GetBandwidth();
    ASSERT_EQ(bw, 0);
}

TEST(ReactorParameter, Test_GetIops)
{
    ReactorParameter reactorParam;
    reactorParam.IncreaseBandwidth(0);
    uint64_t iops = reactorParam.GetIops();
    ASSERT_EQ(iops, 0);
}

} // namespace pos

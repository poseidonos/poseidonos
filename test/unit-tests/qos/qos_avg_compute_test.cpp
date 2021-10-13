#include "src/qos/qos_avg_compute.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(MovingAvgCompute, MovingAvgCompute_Constructor_One_Stack)
{
    MovingAvgCompute movingAvgCompute(0);
}

TEST(MovingAvgCompute, MovingAvgCompute_Constructor_One_Heap)
{
    MovingAvgCompute* movingAvgCompute = new MovingAvgCompute(0);
    delete movingAvgCompute;
}

TEST(MovingAvgCompute, EnqueueAvgData_CountVolLessThanWindow)
{
    MovingAvgCompute movingAvgCompute(10);
    movingAvgCompute.Initilize();
    int volId = 0;
    uint64_t bw = 0;
    uint64_t iops = 0;
    bool expected = false;
    bool returned = movingAvgCompute.EnqueueAvgData(volId, bw, iops);
    ASSERT_EQ(expected, returned);
}

TEST(MovingAvgCompute, EnqueueAvgData_CountVolMoreThanWindow)
{
    MovingAvgCompute movingAvgCompute(1);
    movingAvgCompute.Initilize();
    int volId = 0;
    uint64_t bw = 0;
    uint64_t iops = 0;
    bool expected = true;
    bool returned = movingAvgCompute.EnqueueAvgData(volId, bw, iops);
    ASSERT_EQ(expected, returned);
}
TEST(MovingAvgCompute, ComputeMovingAvg_dataReadyToProcessFalse)
{
    MovingAvgCompute movingAvgCompute(1);
    movingAvgCompute.Initilize();
    int volId = 0;
    uint64_t bw = 0;
    uint64_t iops = 0;
    bool expected = true;
    bool returned = movingAvgCompute.EnqueueAvgData(volId, bw, iops);
    uint64_t expectedBw = 0;
    uint64_t returnedBw = movingAvgCompute.GetMovingAvgBw(0);
    uint64_t expectedIops = 0;
    uint64_t returnedIops = movingAvgCompute.GetMovingAvgIops(0);
    ASSERT_EQ(expected, returned);
    ASSERT_EQ(expectedBw, returnedBw);
    ASSERT_EQ(expectedIops, returnedIops);
}

TEST(MovingAvgCompute, ComputeMovingAvg_dataReadyToProcessTrue)
{
    MovingAvgCompute movingAvgCompute(1);
    movingAvgCompute.Initilize();
    int volId = 0;
    uint64_t bw = 0;
    uint64_t iops = 0;
    bool returned = movingAvgCompute.EnqueueAvgData(volId, bw, iops);
    bool dequeueAvg = movingAvgCompute.EnqueueAvgData(volId, bw, iops);
    uint64_t expectedBw = 0;
    uint64_t returnedBw = movingAvgCompute.GetMovingAvgBw(0);
    uint64_t expectedIops = 0;
    uint64_t returnedIops = movingAvgCompute.GetMovingAvgIops(0);
    ASSERT_EQ(expectedBw, returnedBw);
    ASSERT_EQ(expectedIops, returnedIops);
}

TEST(MovingAvgCompute, DequeueAvgData)
{
    MovingAvgCompute movingAvgCompute(1);
    movingAvgCompute.Initilize();
    int volId = 0;
    uint64_t bw = 0;
    uint64_t iops = 0;
    bool returned = movingAvgCompute.EnqueueAvgData(volId, bw, iops);
    bool dequeueAvg = movingAvgCompute.EnqueueAvgData(volId, bw, iops);
    uint64_t expectedBw = 0;
    uint64_t returnedBw = movingAvgCompute.GetMovingAvgBw(0);
    uint64_t expectedIops = 0;
    uint64_t returnedIops = movingAvgCompute.GetMovingAvgIops(0);
    ASSERT_EQ(expectedBw, returnedBw);
    ASSERT_EQ(expectedIops, returnedIops);
}

TEST(MovingAvgCompute, GetMovingAvg_BW)
{
    MovingAvgCompute movingAvgCompute(0);
    movingAvgCompute.Initilize();
    uint64_t expected = 0;
    uint64_t returned = movingAvgCompute.GetMovingAvgBw(0);
    ASSERT_EQ(expected, returned);
}

TEST(MovingAvgCompute, GetMovingAvg_Iops)
{
    MovingAvgCompute movingAvgCompute(0);
    movingAvgCompute.Initilize();
    uint64_t expected = 0;
    uint64_t returned = movingAvgCompute.GetMovingAvgIops(0);
    ASSERT_EQ(expected, returned);
}

TEST(MovingAvgCompute, ResetMovingAvg_)
{
}

} // namespace pos

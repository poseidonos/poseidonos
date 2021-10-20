#include "src/qos/parameter_queue.h"

#include <gtest/gtest.h>

#include "src/qos/qos_common.h"

namespace pos
{
TEST(ParameterQueue, ParameterQueue_Constructor_One_Stack)
{
    ParameterQueue parameterQueue();
}

TEST(ParameterQueue, ParameterQueue_Constructor_One_Heap)
{
    ParameterQueue* parameterQueue = new ParameterQueue();
    delete parameterQueue;
}

TEST(ParameterQueue, Check_Enqueue_Dequeue_Params)
{
    uint32_t id1 = 0;
    uint32_t id2 = 1;
    bw_iops_parameter param;
    param.currentBW = 1;
    param.currentIOs = 1;
    param.valid = 1;
    ParameterQueue parameterQueue;
    parameterQueue.EnqueueParameter(id1, id2, param);
    bw_iops_parameter enqueued;
    enqueued = parameterQueue.DequeueParameter(id1, id2);
    ASSERT_EQ(param.currentBW, enqueued.currentBW);
    ASSERT_EQ(param.currentIOs, enqueued.currentIOs);
    ASSERT_EQ(param.valid, enqueued.valid);
    for (int i = 0; i < 11; i++)
        ASSERT_EQ(param.pad[i], enqueued.pad[i]);
}

TEST(ParameterQueue, ClearParameters_Test)
{
    ParameterQueue parameterQueue;
    parameterQueue.ClearParameters(0);
}

} // namespace pos

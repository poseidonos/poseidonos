#include "src/qos/resource_cpu.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(ResourceCpu, ResourceCpu_Constructor_One_Stack)
{
    ResourceCpu resourceCpu();
}
TEST(ResourceCpu, ResourceCpu_Constructor_One_Heap)
{
    ResourceCpu* resourceCpu = new ResourceCpu();
    delete resourceCpu;
}
TEST(ResourceCpu, Reset_Check_Reset_and_Get)
{
    ResourceCpu resourceCpu;
    resourceCpu.Reset();
    BackendEvent event = BackendEvent_Start;
    uint32_t expectedPendingCpuCpunt = 0;
    uint32_t actualPendingCpuCount;
    actualPendingCpuCount = resourceCpu.GetEventPendingCpuCount(event);
    ASSERT_EQ(actualPendingCpuCount, expectedPendingCpuCpunt);
}

TEST(ResourceCpu, Check_Setter_Getter_PendingCpuCount)
{
    ResourceCpu resourceCpu;
    BackendEvent event = BackendEvent_Start;
    uint32_t count = 1;
    resourceCpu.SetEventPendingCpuCount(event, count);
    uint32_t expectedPendingcount = 1;
    uint32_t actualPendingCount;
    actualPendingCount = resourceCpu.GetEventPendingCpuCount(event);
    ASSERT_EQ(actualPendingCount, expectedPendingcount);
}

TEST(ResourceCpu, SetTotalGeneratedEvents_Check_Setter)
{
    ResourceCpu resourceCpu;
    BackendEvent event = BackendEvent_Start;
    uint32_t count = 1;
    resourceCpu.SetTotalGeneratedEvents(event, count);
}

} // namespace pos

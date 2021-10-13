#include "src/qos/resource_array.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(ResourceArray, ResourceArray_Constructore_One_Stack)
{
    ResourceArray resourceArray();
}

TEST(ResourceArray, ResourceArray_Constructore_One_Heap)
{
    ResourceArray* resourceArray = new ResourceArray();
    delete resourceArray;
}

TEST(ResourceArray, Reset_Check_gcFreeSegments_zero)
{
    ResourceArray resourceArray;
    resourceArray.Reset();
    uint32_t expectedGcFreeSegmentCount = 0;
    uint32_t actualGcFreeSegmentCount;
    actualGcFreeSegmentCount = resourceArray.GetGcFreeSegment();
    ASSERT_EQ(actualGcFreeSegmentCount, expectedGcFreeSegmentCount);
}

TEST(ResourceArray, Check_SetterGetter_for_GcFreeSegment)
{
    ResourceArray resourceArray;
    uint32_t setGcFreeSegmentCount = 2;
    uint32_t actualGcFreeSegmentCount;
    resourceArray.SetGcFreeSegment(setGcFreeSegmentCount);
    actualGcFreeSegmentCount = resourceArray.GetGcFreeSegment();
    ASSERT_EQ(actualGcFreeSegmentCount, setGcFreeSegmentCount);
}

TEST(ResourceArray, GetGcThreshold_FreeSegments_Zero)
{
    ResourceArray resourceArray;
    uint32_t setGcFreeSegmentCount = 0;
    resourceArray.SetGcFreeSegment(setGcFreeSegmentCount);
    QosGcState expectedGcState = QosGcState_Normal;
    QosGcState actualGcState;
    actualGcState = resourceArray.GetGcThreshold();
    ASSERT_EQ(expectedGcState, actualGcState);
}
TEST(ResourceArray, GetGcThreshold_FreeSegments_gc_UpperTh)
{
    ResourceArray resourceArray;
    uint32_t setGcFreeSegmentCount = UPPER_GC_TH;
    resourceArray.SetGcFreeSegment(setGcFreeSegmentCount);
    QosGcState expectedGcState = QosGcState_Normal;
    QosGcState actualGcState;
    actualGcState = resourceArray.GetGcThreshold();
    ASSERT_EQ(expectedGcState, actualGcState);
}
TEST(ResourceArray, GetGcThreshold_FreeSegments_gc_MedTh)
{
    ResourceArray resourceArray;
    uint32_t setGcFreeSegmentCount = MID_GC_TH + 1;
    resourceArray.SetGcFreeSegment(setGcFreeSegmentCount);
    QosGcState expectedGcState = QosGcState_Medium;
    QosGcState actualGcState;
    actualGcState = resourceArray.GetGcThreshold();
    ASSERT_EQ(expectedGcState, actualGcState);
}

TEST(ResourceArray, GetGcThreshold_FreeSegments_gc_LowTh)
{
    ResourceArray resourceArray;
    uint32_t setGcFreeSegmentCount = LOW_GC_TH + 1;
    resourceArray.SetGcFreeSegment(setGcFreeSegmentCount);
    QosGcState expectedGcState = QosGcState_High;
    QosGcState actualGcState;
    actualGcState = resourceArray.GetGcThreshold();
    ASSERT_EQ(expectedGcState, actualGcState);
}
TEST(ResourceArray, GetGcThreshold_FreeSegments_gc_CritTh)
{
    ResourceArray resourceArray;
    uint32_t setGcFreeSegmentCount = LOW_GC_TH - 1;
    resourceArray.SetGcFreeSegment(setGcFreeSegmentCount);
    QosGcState expectedGcState = QosGcState_Critical;
    QosGcState actualGcState;
    actualGcState = resourceArray.GetGcThreshold();
    ASSERT_EQ(expectedGcState, actualGcState);
}

} // namespace pos

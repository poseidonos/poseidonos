#include "src/qos/parameters_event.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(EventParameter, EventParameter_Constructor_One_Stack)
{
    EventParameter event();
}

TEST(EventParameter, EventParameter_Constructor_One_Heap)
{
    EventParameter* event = new EventParameter();
    delete event;
}

TEST(EventParameter, Check_Reset_Bandwidth)
{
    EventParameter event;
    event.Reset();
}

TEST(EventParameter, Check_SetBandwidth_Value)
{
    EventParameter event;
    uint64_t bw = 5;
    event.SetBandwidth(bw);
}

TEST(EventParameter, Check_IncreaseBandwidth_Value)
{
    EventParameter event;
    uint64_t bw = 5;
    event.Reset();
    event.IncreaseBandwidth(bw);
}

} // namespace pos

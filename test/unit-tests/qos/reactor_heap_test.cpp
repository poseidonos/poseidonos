#include "src/qos/reactor_heap.h"
#include <gtest/gtest.h>
#include "spdk/pos.h"

namespace pos
{

TEST(ReactorHeap, ReactorHeap_Constructor_One_Stack)
{
    ReactorHeap reactorHeap();
}

TEST(ReactorHeap, ReactorHeap_Constructor_One_Heap)
{
    ReactorHeap* reactorHeap = new ReactorHeap();
    delete reactorHeap;
}

TEST(ReactorHeap, ReactorHeap_Insert)
{
    uint64_t bwWeight = 10*1024*1024;
    uint32_t reactor = 0;
    ReactorHeap reactorHeap;
    reactorHeap.InsertPairInHeap(bwWeight, reactor);
    ASSERT_EQ(reactorHeap.GetHeapSize(), 1);
}

TEST(ReactorHeap, ReactorHeap_GetAllReactorId)
{
    ReactorHeap reactorHeap;
    reactorHeap.InsertPairInHeap(1024*1024, 0);
    reactorHeap.InsertPairInHeap(10*1024*1024, 1);
    reactorHeap.InsertPairInHeap(20*1024*1024, 2);

    std::vector<uint32_t> recdIds;
    recdIds = reactorHeap.GetAllReactorIds();
    uint32_t recdId1 = recdIds.front();
    ASSERT_EQ(recdId1, 0);
    recdIds.erase(recdIds.begin());
    uint32_t recdId2 = recdIds.front();
    ASSERT_EQ(recdId2, 1);
    recdIds.erase(recdIds.begin());
    uint32_t recdId3 = recdIds.front();
    ASSERT_EQ(recdId3, 2);
}

TEST(ReactorHeap, ReactorHeap_Clear_Test)
{
    ReactorHeap reactorHeap;
    reactorHeap.InsertPairInHeap(1024*1024, 0);
    reactorHeap.InsertPairInHeap(10*1024*1024, 1);
    reactorHeap.ClearHeap();
    ASSERT_EQ(reactorHeap.GetHeapSize(), 0);
}

TEST(ReactorHeap, ReactorHeap_HeapSize)
{
    ReactorHeap reactorHeap;
    reactorHeap.InsertPairInHeap(1024*1024, 0);
    reactorHeap.InsertPairInHeap(10*1024*1024, 1);
    ASSERT_EQ(reactorHeap.GetHeapSize(), 2);
}
} // namespace pos

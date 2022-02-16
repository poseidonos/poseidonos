#include "src/allocator/context_manager/segment_ctx/segment_list.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(SegmentList, SegmentList_Constructor)
{
    {
        SegmentList list;
    }

    {
        SegmentList* list = new SegmentList();
        delete list;
    }
}

TEST(SegmentList, Reset_testWhenListIsEmpty)
{
    SegmentList list;
    list.Reset();

    EXPECT_EQ(list.GetNumSegments(), 0);
    EXPECT_EQ(list.PopSegment(), UNMAP_SEGMENT);
}

TEST(SegmentList, Reset_testWhenListIsNotEmpty)
{
    SegmentList list;

    list.AddToList(0);
    list.AddToList(1);
    list.AddToList(2);
    list.AddToList(5);
    
    EXPECT_EQ(list.GetNumSegments(), 4);
    EXPECT_EQ(list.Contains(0), true);
    EXPECT_EQ(list.Contains(1), true);
    EXPECT_EQ(list.Contains(2), true);
    EXPECT_EQ(list.Contains(5), true);

    list.Reset();

    EXPECT_EQ(list.GetNumSegments(), 0);
    EXPECT_EQ(list.PopSegment(), UNMAP_SEGMENT);
}

TEST(SegmentList, PopSegment_testWhenListIsEmpty)
{
    SegmentList list;
    EXPECT_EQ(list.PopSegment(), UNMAP_SEGMENT);
    EXPECT_EQ(list.GetNumSegments(), 0);
    EXPECT_EQ(list.GetNumSegmentsWoLock(), 0);
}

TEST(SegmentList, AddToList_testAddAndPop)
{
    SegmentList list;

    list.AddToList(0);
    list.AddToList(1);
    list.AddToList(2);

    EXPECT_EQ(list.GetNumSegments(), 3);

    EXPECT_EQ(list.PopSegment(), 0);
    EXPECT_EQ(list.GetNumSegments(), 2);
    EXPECT_EQ(list.GetNumSegmentsWoLock(), 2);

    EXPECT_EQ(list.PopSegment(), 1);
    EXPECT_EQ(list.GetNumSegments(), 1);
    EXPECT_EQ(list.GetNumSegmentsWoLock(), 1);

    EXPECT_EQ(list.PopSegment(), 2);
    EXPECT_EQ(list.GetNumSegments(), 0);
    EXPECT_EQ(list.GetNumSegmentsWoLock(), 0);

    EXPECT_EQ(list.PopSegment(), UNMAP_SEGMENT);
    EXPECT_EQ(list.GetNumSegments(), 0);
    EXPECT_EQ(list.GetNumSegmentsWoLock(), 0);
}

TEST(SegmentList, RemoveFromList_testRemove)
{
    SegmentList list;
    list.AddToList(0);
    list.AddToList(10);
    list.AddToList(20);

    EXPECT_EQ(list.GetNumSegments(), 3);

    list.RemoveFromList(2);
    EXPECT_EQ(list.GetNumSegments(), 3);

    list.RemoveFromList(10);
    EXPECT_EQ(list.GetNumSegments(), 2);
}
} // namespace pos

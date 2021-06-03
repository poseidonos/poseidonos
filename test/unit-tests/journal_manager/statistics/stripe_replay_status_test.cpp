#include "src/journal_manager/statistics/stripe_replay_status.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(StripeReplayStatus, BlockWritten_testIfStatusUpdatedWhenBlockMapReplayed)
{
    // Given
    StripeId vsid = 0;
    VirtualBlks virtualBlks;
    virtualBlks.startVsa.stripeId = vsid;
    virtualBlks.startVsa.offset = 0;
    virtualBlks.numBlks = 1;
    StripeReplayStatus status(vsid);

    // When: Status Updated when block offset is not set
    // Then: Status will be fault
    EXPECT_DEATH(status.BlockWritten(virtualBlks.startVsa.offset, virtualBlks.numBlks), "");

    // When: Status Updated when block offset setted
    status.SetFirstBlockOffset(0);
    status.SetLastBlockOffset(10);
    status.BlockWritten(virtualBlks.startVsa.offset, virtualBlks.numBlks);

    // Then: NumUpdatedBlockMaps is changed
    EXPECT_EQ(status.GetNumUpdatedBlockMaps(), virtualBlks.numBlks);

    status.Print();
}

TEST(StripeReplayStatus, StripeFlushed_testIfStatusUpdatedWhenStripFlushed)
{
    // Given
    StripeId vsid = 0;
    StripeReplayStatus status(vsid);

    // When
    status.StripeFlushed();

    // Then
    EXPECT_EQ(status.GetStripeMapReplayed(), true);

    status.Print();
}

TEST(StripeReplayStatus, StripeFlushed_testIfStatusUpdatedWhenStripFlushedMoreThanOnce)
{
    // Given
    StripeId vsid = 0;
    StripeReplayStatus status(vsid);

    // When: Strip flush replayed more than once
    status.StripeFlushed();
    status.StripeFlushed();

    // Then: Error log printed and stripeMapReplayed status is true
    EXPECT_EQ(status.GetStripeMapReplayed(), true);

    status.Print();
}

TEST(StripeReplayStatus, BlockInvalidated_testIfStatusUpdated)
{
    // Given
    StripeId vsid = 0;
    uint32_t numBlocksToInvalidate = 10;
    StripeReplayStatus status(vsid);

    // When
    status.BlockInvalidated(numBlocksToInvalidate);

    // Then
    EXPECT_EQ(status.GetNumInvalidatedBlocks(), numBlocksToInvalidate);

    status.Print();
}

TEST(StripeReplayStatus, SegmentAllocated_testIfStatusUpdated)
{
    // Given
    StripeId vsid = 0;
    uint32_t numBlocksToInvalidate = 10;
    StripeReplayStatus status(vsid);

    // When
    status.SegmentAllocated();

    // Then
    EXPECT_EQ(status.GetSegmentAllocated(), true);

    status.Print();
}

TEST(StripeReplayStatus, StripeAllocated_testIfStatusUpdated)
{
    // Given
    StripeId vsid = 0;
    uint32_t numBlocksToInvalidate = 10;
    StripeReplayStatus status(vsid);

    // When
    status.StripeAllocated();

    // Then
    EXPECT_EQ(status.GetStripeAllocated(), true);

    status.Print();
}
} // namespace pos

#include "src/metadata/segment_context_updater.h"

#include "test/unit-tests/array_models/dto/partition_logical_size_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/i_versioned_segment_context_mock.h"
#include "test/unit-tests/allocator/i_segment_ctx_mock.h"

#include <gtest/gtest.h>

using ::testing::NiceMock;
using ::testing::_;

namespace pos
{
TEST(SegmentContextUpdater, SegmentContextUpdater_testIfConstructedSuccessfully)
{
    NiceMock<MockISegmentCtx> segmentCtx;
    NiceMock<MockIVersionedSegmentContext> versionedCtx;
    PartitionLogicalSize sizeInfo;

    SegmentContextUpdater updater(&segmentCtx, &versionedCtx, &sizeInfo);
}

TEST(SegmentContextUpdater, ValidateBlks_testIfBothContextIsUpdatedProperly)
{
    NiceMock<MockISegmentCtx> segmentCtx;
    NiceMock<MockIVersionedSegmentContext> versionedCtx;
    PartitionLogicalSize sizeInfo;
    sizeInfo.stripesPerSegment = 128;

    SegmentContextUpdater updater(&segmentCtx, &versionedCtx, &sizeInfo);
    
    VirtualBlks blks = {
        .startVsa = { .stripeId = 1000, .offset = 0 },
        .numBlks = 10};

    EXPECT_CALL(segmentCtx, ValidateBlks(blks)).Times(1);
    EXPECT_CALL(versionedCtx, IncreaseValidBlockCount(_, blks.startVsa.stripeId / sizeInfo.stripesPerSegment, blks.numBlks)).Times(1);

    updater.ValidateBlks(blks);
}

TEST(SegmentContextUpdater, InvalidateBlks_testIfBothContextIsUpdatedProperly)
{
    NiceMock<MockISegmentCtx> segmentCtx;
    NiceMock<MockIVersionedSegmentContext> versionedCtx;
    PartitionLogicalSize sizeInfo;
    sizeInfo.stripesPerSegment = 128;

    SegmentContextUpdater updater(&segmentCtx, &versionedCtx, &sizeInfo);
    
    VirtualBlks blks = {
        .startVsa = { .stripeId = 1000, .offset = 0 },
        .numBlks = 10};

    EXPECT_CALL(segmentCtx, InvalidateBlks(blks, true)).Times(1);
    EXPECT_CALL(versionedCtx, DecreaseValidBlockCount(_, blks.startVsa.stripeId / sizeInfo.stripesPerSegment, blks.numBlks)).Times(1);

    updater.InvalidateBlks(blks, true);
}

TEST(SegmentContextUpdater, UpdateOccupiedStripeCount_testIfBothContextIsUpdatedProperly)
{
    NiceMock<MockISegmentCtx> segmentCtx;
    NiceMock<MockIVersionedSegmentContext> versionedCtx;
    PartitionLogicalSize sizeInfo;
    sizeInfo.stripesPerSegment = 128;

    SegmentContextUpdater updater(&segmentCtx, &versionedCtx, &sizeInfo);

    StripeId lsid = 2031;
    EXPECT_CALL(segmentCtx, UpdateOccupiedStripeCount(lsid)).Times(1);
    EXPECT_CALL(versionedCtx, IncreaseOccupiedStripeCount(_, lsid / sizeInfo.stripesPerSegment)).Times(1);

    updater.UpdateOccupiedStripeCount(lsid);
}

} // namespace pos

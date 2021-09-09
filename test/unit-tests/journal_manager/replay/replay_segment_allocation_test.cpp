#include "src/journal_manager/replay/replay_segment_allocation.h"

#include <gtest/gtest.h>

#include "test/unit-tests/allocator/i_context_replayer_mock.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/journal_manager/statistics/stripe_replay_status_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(ReplaySegmentAllocation, ReplaySegmentAllocation_testIfConstructedSuccessfully)
{
    NiceMock<MockIContextReplayer> contextReplayer;
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockStripeReplayStatus> status;

    ReplaySegmentAllocation segmentAllocationEvent(&contextReplayer, &arrayInfo, &status, 0);
}

TEST(ReplaySegmentAllocation, Replay_testIfSegmentAllocationIsReplayedSuccessfully)
{
    NiceMock<MockIContextReplayer> contextReplayer;
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockStripeReplayStatus> status;

    StripeId userLsid = 4096;

    PartitionLogicalSize userPartitionSize;
    userPartitionSize.stripesPerSegment = 1024;
    ON_CALL(arrayInfo, GetSizeInfo(PartitionType::USER_DATA)).WillByDefault(Return(&userPartitionSize));

    EXPECT_CALL(contextReplayer, ReplaySegmentAllocation(4096));
    ReplaySegmentAllocation segmentAllocationEvent(&contextReplayer, &arrayInfo, &status, userLsid);

    int result = segmentAllocationEvent.Replay();
    EXPECT_EQ(result, 0);
}

TEST(ReplaySegmentAllocation, Replay_testIfSegmentAllocationIsReplayedSuccessfullyWhenLsidIsNotAligned)
{
    NiceMock<MockIContextReplayer> contextReplayer;
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockStripeReplayStatus> status;

    StripeId userLsid = 5000;

    PartitionLogicalSize userPartitionSize;
    userPartitionSize.stripesPerSegment = 1024;
    ON_CALL(arrayInfo, GetSizeInfo(PartitionType::USER_DATA)).WillByDefault(Return(&userPartitionSize));

    EXPECT_CALL(contextReplayer, ReplaySegmentAllocation(4096));
    ReplaySegmentAllocation segmentAllocationEvent(&contextReplayer, &arrayInfo, &status, userLsid);

    int result = segmentAllocationEvent.Replay();
    EXPECT_EQ(result, 0);
}

TEST(ReplaySegmentAllocation, GetType_testIfCorrectTypeIsReturned)
{
    ReplaySegmentAllocation segmentAllocationEvent(nullptr, nullptr, nullptr, 0);
    EXPECT_EQ(segmentAllocationEvent.GetType(), ReplayEventType::SEGMENT_ALLOCATION);
}

} // namespace pos

#include "src/journal_manager/replay/replay_block_map_update.h"

#include <gtest/gtest.h>

#include "test/unit-tests/allocator/i_segment_ctx_mock.h"
#include "test/unit-tests/journal_manager/replay/active_wb_stripe_replayer_mock.h"
#include "test/unit-tests/journal_manager/statistics/stripe_replay_status_mock.h"
#include "test/unit-tests/mapper/i_vsamap_mock.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SetArgReferee;

namespace pos
{
TEST(ReplayBlockMapUpdate, ReplayBlockMapUpdate_testIfConstructedSuccessfully)
{
    NiceMock<MockIVSAMap> vsaMap;
    NiceMock<MockISegmentCtx> segmentCtx;
    NiceMock<MockStripeReplayStatus> stripeReplayStatus;
    PendingStripeList list;
    NiceMock<MockActiveWBStripeReplayer> wbStripeReplayer(list);

    int volId = 2;
    BlkAddr startRba = 100;
    VirtualBlkAddr startVsa = {
        .stripeId = 20,
        .offset = 0};
    uint64_t numBlks = 10;
    bool replaySegmentInfo = false;

    ReplayBlockMapUpdate blockMapUpdateEvent(&vsaMap, &segmentCtx,
        &stripeReplayStatus, &wbStripeReplayer,
        volId, startRba, startVsa, numBlks, replaySegmentInfo);
}

TEST(ReplayBlockMapUpdate, Replay_testIfBlockMapIsUpdated)
{
    // Given
    NiceMock<MockIVSAMap> vsaMap;
    NiceMock<MockISegmentCtx> segmentCtx;
    NiceMock<MockStripeReplayStatus> stripeReplayStatus;
    PendingStripeList list;
    NiceMock<MockActiveWBStripeReplayer> wbStripeReplayer(list);

    int volId = 2;
    BlkAddr startRba = 100;
    VirtualBlkAddr startVsa = {
        .stripeId = 20,
        .offset = 20};
    uint32_t numBlks = 60;
    bool replaySegmentInfo = false;

    ReplayBlockMapUpdate blockMapUpdateEvent(&vsaMap, &segmentCtx,
        &stripeReplayStatus, &wbStripeReplayer,
        volId, startRba, startVsa, numBlks, replaySegmentInfo);

    // When: All stored vsa is UNMAP
    ON_CALL(vsaMap, GetVSAWithSyncOpen).WillByDefault(Return(UNMAP_VSA));

    // Then: All vsas should be updated
    for (uint32_t offset = 0; offset < numBlks; offset++)
    {
        BlkAddr currentRba = startRba + offset;
        VirtualBlks blks = {
            .startVsa = {
                .stripeId = startVsa.stripeId,
                .offset = startVsa.offset + offset},
            .numBlks = 1};
        EXPECT_CALL(vsaMap, SetVSAsWithSyncOpen(volId, currentRba, blks)).Times(1);
        EXPECT_CALL(stripeReplayStatus, BlockWritten(startVsa.offset + offset, 1));
    }

    int result = blockMapUpdateEvent.Replay();
    EXPECT_EQ(result, 0);
}

TEST(ReplayBlockMapUpdate, Replay_testIfBlockMapIsNotUpdatedWhenMapIsLatest)
{
    // Given
    NiceMock<MockIVSAMap> vsaMap;
    NiceMock<MockISegmentCtx> segmentCtx;
    NiceMock<MockStripeReplayStatus> stripeReplayStatus;
    PendingStripeList list;
    NiceMock<MockActiveWBStripeReplayer> wbStripeReplayer(list);

    int volId = 2;
    BlkAddr startRba = 100;
    VirtualBlkAddr startVsa = {
        .stripeId = 20,
        .offset = 20};
    uint32_t numBlks = 60;
    bool replaySegmentInfo = false;

    ReplayBlockMapUpdate blockMapUpdateEvent(&vsaMap, &segmentCtx,
        &stripeReplayStatus, &wbStripeReplayer,
        volId, startRba, startVsa, numBlks, replaySegmentInfo);

    // When: Some vsa is UNMAP, and some are not
    for (uint32_t offset = 0; offset < numBlks / 2; offset++)
    {
        BlkAddr currentRba = startRba + offset;
        EXPECT_CALL(vsaMap, GetVSAWithSyncOpen(volId, currentRba)).WillOnce(Return(UNMAP_VSA));
    }
    for (uint32_t offset = numBlks / 2; offset < numBlks; offset++)
    {
        BlkAddr currentRba = startRba + offset;
        VirtualBlkAddr currentVsa = {
            .stripeId = startVsa.stripeId,
            .offset = startVsa.offset + offset};

        EXPECT_CALL(vsaMap, GetVSAWithSyncOpen(volId, currentRba)).WillOnce(Return(currentVsa));
    }

    // Then: Only UNMAP vsa should be updated
    for (uint32_t offset = 0; offset < numBlks / 2; offset++)
    {
        BlkAddr currentRba = startRba + offset;
        VirtualBlks blks = {
            .startVsa = {
                .stripeId = startVsa.stripeId,
                .offset = startVsa.offset + offset},
            .numBlks = 1};
        EXPECT_CALL(vsaMap, SetVSAsWithSyncOpen(volId, currentRba, blks)).Times(1);
        EXPECT_CALL(stripeReplayStatus, BlockWritten(startVsa.offset + offset, 1));
    }
    for (uint32_t offset = numBlks / 2; offset < numBlks; offset++)
    {
        BlkAddr currentRba = startRba + offset;
        EXPECT_CALL(vsaMap, SetVSAsWithSyncOpen(volId, currentRba, _)).Times(0);
    }

    int result = blockMapUpdateEvent.Replay();
    EXPECT_EQ(result, 0);
}

TEST(ReplayBlockMapUpdate, Replay_testIfOldBlockIsInvalidated)
{
    // Given
    NiceMock<MockIVSAMap> vsaMap;
    NiceMock<MockISegmentCtx> segmentCtx;
    NiceMock<MockStripeReplayStatus> stripeReplayStatus;
    PendingStripeList list;
    NiceMock<MockActiveWBStripeReplayer> wbStripeReplayer(list);

    int volId = 2;
    BlkAddr startRba = 100;
    VirtualBlkAddr startVsa = {
        .stripeId = 20,
        .offset = 20};
    uint32_t numBlks = 60;
    bool replaySegmentInfo = true;

    ReplayBlockMapUpdate blockMapUpdateEvent(&vsaMap, &segmentCtx,
        &stripeReplayStatus, &wbStripeReplayer,
        volId, startRba, startVsa, numBlks, replaySegmentInfo);

    // When: Old vsa is different from vsa in the log
    for (uint32_t offset = 0; offset < numBlks; offset++)
    {
        BlkAddr currentRba = startRba + offset;
        VirtualBlkAddr oldVsa = {
            .stripeId = 200,
            .offset = startVsa.offset + offset};
        ON_CALL(vsaMap, GetVSAWithSyncOpen(volId, currentRba)).WillByDefault(Return(oldVsa));
    }

    // Then: All previous blocks should be invalidated
    for (uint32_t offset = 0; offset < numBlks; offset++)
    {
        VirtualBlks blksToInvalidate = {
            .startVsa = {
                .stripeId = 200,
                .offset = startVsa.offset + offset},
            .numBlks = 1};
        EXPECT_CALL(segmentCtx, InvalidateBlks(blksToInvalidate)).Times(1);
    }
    EXPECT_CALL(stripeReplayStatus, BlockInvalidated).Times(numBlks);

    int result = blockMapUpdateEvent.Replay();
    EXPECT_EQ(result, 0);
}

TEST(ReplayBlockMapUpdate, Replay_testIfOldBlockIsNotInvalidatedWhenOldVsaIsUnmap)
{
    // Given
    NiceMock<MockIVSAMap> vsaMap;
    NiceMock<MockISegmentCtx> segmentCtx;
    NiceMock<MockStripeReplayStatus> stripeReplayStatus;
    PendingStripeList list;
    NiceMock<MockActiveWBStripeReplayer> wbStripeReplayer(list);

    int volId = 2;
    BlkAddr startRba = 100;
    VirtualBlkAddr startVsa = {
        .stripeId = 20,
        .offset = 20};
    uint32_t numBlks = 60;
    bool replaySegmentInfo = true;

    ReplayBlockMapUpdate blockMapUpdateEvent(&vsaMap, &segmentCtx,
        &stripeReplayStatus, &wbStripeReplayer,
        volId, startRba, startVsa, numBlks, replaySegmentInfo);

    // When: Old vsa is different from vsa in the log
    ON_CALL(vsaMap, GetVSAWithSyncOpen).WillByDefault(Return(UNMAP_VSA));

    // Then: No blocks should be invalidated
    EXPECT_CALL(segmentCtx, InvalidateBlks).Times(0);
    EXPECT_CALL(stripeReplayStatus, BlockInvalidated).Times(0);

    int result = blockMapUpdateEvent.Replay();
    EXPECT_EQ(result, 0);
}

TEST(ReplayBlockMapUpdate, GetType_testIfReturnTypeCorrectly)
{
    ReplayBlockMapUpdate blockMapUpdateEvent(nullptr, nullptr, nullptr, nullptr,
        0, 0, UNMAP_VSA, 0, false);

    ReplayEventType actual = blockMapUpdateEvent.GetType();
    EXPECT_EQ(actual, ReplayEventType::BLOCK_MAP_UPDATE);
}

} // namespace pos

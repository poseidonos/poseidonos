#include "src/journal_manager/replay/active_user_stripe_replayer.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <random>

#include "src/logger/logger.h"
#include "test/unit-tests/allocator/i_segment_ctx_mock.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"

using ::testing::_;
using testing::NiceMock;
using ::testing::Return;

namespace pos
{
const uint32_t numStripesPerSegment = 64;
const int numBlksPerChunk = 32;
const int numChunksPerStripe = 4;
const uint64_t numBlksPerStripe = numBlksPerChunk * numChunksPerStripe;
const int numUserSegments = 64;
const uint32_t numUserStripes = numStripesPerSegment * numUserSegments;

void
UpdateUserStripeSequentially(ActiveUserStripeReplayer* userStripeReplayer, uint32_t beginLSID, uint32_t endLSID)
{
    for (uint32_t i = beginLSID; i <= endLSID; i++)
    {
        userStripeReplayer->Update(i);
    }
}

void
UpdateUserStripeRandomly(ActiveUserStripeReplayer* userStripeReplayer, uint32_t beginLSID, uint32_t endLSID)
{
    std::vector<StripeId> userLsids;
    for (StripeId i = beginLSID; i <= endLSID; i++)
    {
        userLsids.push_back(i);
    }
    auto rng = std::default_random_engine{};
    std::shuffle(userLsids.begin(), userLsids.end(), rng);

    for (auto it = userLsids.begin(); it != userLsids.end(); it++)
    {
        userStripeReplayer->Update(*it);
    }
}

void
ReplayUserStripe(ActiveUserStripeReplayer* userStripeReplayer, MockISegmentCtx* allocatorSegmentCtx, MockIArrayInfo* arrayInfo, StripeId expectLSID)
{
    PartitionLogicalSize userSizeInfo;
    userSizeInfo.minWriteBlkCnt = 0;
    userSizeInfo.blksPerChunk = numBlksPerChunk;
    userSizeInfo.blksPerStripe = numBlksPerStripe;
    userSizeInfo.chunksPerStripe = numChunksPerStripe;
    userSizeInfo.stripesPerSegment = numStripesPerSegment;
    userSizeInfo.totalSegments = numUserSegments;
    userSizeInfo.totalStripes = numUserStripes;

    EXPECT_CALL(*arrayInfo, GetSizeInfo(_)).WillOnce(Return(&userSizeInfo));
    EXPECT_CALL(*allocatorSegmentCtx, ReplaySsdLsid(expectLSID));
    userStripeReplayer->Replay();
}

TEST(ActiveUserStripeReplayer, Replay_ReplaySequentialLsidInOneSegment)
{
    // Given
    NiceMock<MockISegmentCtx> allocatorSegmentCtx;
    MockIArrayInfo arrayInfo;

    {
        ActiveUserStripeReplayer userStripeReplayer(&allocatorSegmentCtx, &arrayInfo);

        // When : Find sequential LSIDs from beginning to middle within a segment
        UpdateUserStripeSequentially(&userStripeReplayer, 0, numStripesPerSegment / 2);

        // Then : Replay the last LSID to current SSD LSID
        ReplayUserStripe(&userStripeReplayer, &allocatorSegmentCtx, &arrayInfo, numStripesPerSegment / 2);
    }
    {
        ActiveUserStripeReplayer userStripeReplayer(&allocatorSegmentCtx, &arrayInfo);

        // When : Find sequential LSIDs from middle to end within a segment
        UpdateUserStripeSequentially(&userStripeReplayer, numStripesPerSegment / 2, numStripesPerSegment - 1);

        // Then : Replay the last LSID to current SSD LSID
        ReplayUserStripe(&userStripeReplayer, &allocatorSegmentCtx, &arrayInfo, numStripesPerSegment - 1);
    }
    {
        ActiveUserStripeReplayer userStripeReplayer(&allocatorSegmentCtx, &arrayInfo);

        // When : Find sequential LSIDs from beginning to end within a segment
        UpdateUserStripeSequentially(&userStripeReplayer, 0, numStripesPerSegment - 1);

        // Then : Replay the last LSID to current SSD LSID
        ReplayUserStripe(&userStripeReplayer, &allocatorSegmentCtx, &arrayInfo, numStripesPerSegment - 1);
    }
}

TEST(ActiveUserStripeReplayer, Replay_ReplayRandomLsidInOneSegment)
{
    // Given
    NiceMock<MockISegmentCtx> allocatorSegmentCtx;
    NiceMock<MockIArrayInfo> arrayInfo;

    {
        ActiveUserStripeReplayer userStripeReplayer(&allocatorSegmentCtx, &arrayInfo);

        // When : Find LSIDs randomly from beginning to middle within a segment
        UpdateUserStripeRandomly(&userStripeReplayer, 0, numStripesPerSegment / 2);

        // Then : Replay the last LSID to current SSD LSID
        ReplayUserStripe(&userStripeReplayer, &allocatorSegmentCtx, &arrayInfo, numStripesPerSegment / 2);
    }
    {
        ActiveUserStripeReplayer userStripeReplayer(&allocatorSegmentCtx, &arrayInfo);

        // When : Find LSIDs randomly from middle to end within a segment
        UpdateUserStripeRandomly(&userStripeReplayer, numStripesPerSegment / 2, numStripesPerSegment - 1);

        // Then : Replay the last LSID to current SSD LSID
        ReplayUserStripe(&userStripeReplayer, &allocatorSegmentCtx, &arrayInfo, numStripesPerSegment - 1);
    }
    {
        ActiveUserStripeReplayer userStripeReplayer(&allocatorSegmentCtx, &arrayInfo);

        // When : Find LSIDs randomly from beginning to end within a segment
        UpdateUserStripeRandomly(&userStripeReplayer, 0, numStripesPerSegment - 1);

        // Then : Replay the last LSID to current SSD LSID
        ReplayUserStripe(&userStripeReplayer, &allocatorSegmentCtx, &arrayInfo, numStripesPerSegment - 1);
    }
}

TEST(ActiveUserStripeReplayer, Replay_ReplaySequentialLsidInSeveralSegment)
{
    // Given
    NiceMock<MockISegmentCtx> allocatorSegmentCtx;
    NiceMock<MockIArrayInfo> arrayInfo;
    uint32_t numSegment = 3;
    uint32_t lastLSID = numStripesPerSegment * numSegment + numStripesPerSegment / 2 - 1;

    {
        ActiveUserStripeReplayer userStripeReplayer(&allocatorSegmentCtx, &arrayInfo);

        // When : Find sequential LSIDs from beginning to middle within several segment
        UpdateUserStripeSequentially(&userStripeReplayer, 0, lastLSID);

        // Then : Replay the last LSID to current SSD LSID
        ReplayUserStripe(&userStripeReplayer, &allocatorSegmentCtx, &arrayInfo, lastLSID);
    }
    {
        ActiveUserStripeReplayer userStripeReplayer(&allocatorSegmentCtx, &arrayInfo);

        // When : Find sequential LSIDs from middle to middle within several segment
        UpdateUserStripeSequentially(&userStripeReplayer, numStripesPerSegment / 2, lastLSID);

        // Then : Replay the last LSID to current SSD LSID
        ReplayUserStripe(&userStripeReplayer, &allocatorSegmentCtx, &arrayInfo, lastLSID);
    }
    {
        lastLSID = numStripesPerSegment * (numSegment + 1) - 1;
        ActiveUserStripeReplayer userStripeReplayer(&allocatorSegmentCtx, &arrayInfo);

        // When : Find sequential LSIDs from beginning to end within several segment
        UpdateUserStripeSequentially(&userStripeReplayer, 0, lastLSID);

        // Then : Replay the last LSID to current SSD LSID
        ReplayUserStripe(&userStripeReplayer, &allocatorSegmentCtx, &arrayInfo, numStripesPerSegment - 1);
    }
    {
        ActiveUserStripeReplayer userStripeReplayer(&allocatorSegmentCtx, &arrayInfo);

        // When : Find sequential LSIDs from middle to end within several segment
        UpdateUserStripeSequentially(&userStripeReplayer, numStripesPerSegment / 2, lastLSID);

        // Then :
        ReplayUserStripe(&userStripeReplayer, &allocatorSegmentCtx, &arrayInfo, numStripesPerSegment - 1);
    }
}

TEST(ActiveUserStripeReplayer, Replay_ReplayRandomLsidInSeveralSegment)
{
    // Given
    NiceMock<MockISegmentCtx> allocatorSegmentCtx;
    NiceMock<MockIArrayInfo> arrayInfo;
    uint32_t numSegment = 3;
    uint32_t lastLSID = numStripesPerSegment * numSegment + numStripesPerSegment / 2 - 1;

    {
        ActiveUserStripeReplayer userStripeReplayer(&allocatorSegmentCtx, &arrayInfo);

        // When : Find LSIDs randomly from beginning to middle within several segment
        UpdateUserStripeRandomly(&userStripeReplayer, 0, lastLSID);

        // Then : Replay the last LSID to current SSD LSID
        ReplayUserStripe(&userStripeReplayer, &allocatorSegmentCtx, &arrayInfo, lastLSID);
    }
    {
        ActiveUserStripeReplayer userStripeReplayer(&allocatorSegmentCtx, &arrayInfo);

        // When : Find LSIDs randomly from middle to middle within several segment
        UpdateUserStripeRandomly(&userStripeReplayer, numStripesPerSegment / 2, lastLSID);

        // Then : Replay the last LSID to current SSD LSID
        ReplayUserStripe(&userStripeReplayer, &allocatorSegmentCtx, &arrayInfo, lastLSID);
    }
    {
        lastLSID = numStripesPerSegment * (numSegment + 1) - 1;
        ActiveUserStripeReplayer userStripeReplayer(&allocatorSegmentCtx, &arrayInfo);

        // When : Find LSIDs randomly from beginning to end within several segment
        UpdateUserStripeRandomly(&userStripeReplayer, 0, lastLSID);

        // Then : Replay the last LSID to current SSD LSID
        ReplayUserStripe(&userStripeReplayer, &allocatorSegmentCtx, &arrayInfo, numStripesPerSegment - 1);
    }
    {
        ActiveUserStripeReplayer userStripeReplayer(&allocatorSegmentCtx, &arrayInfo);

        // When : Find LSIDs randomly from middle to end within several segment
        UpdateUserStripeRandomly(&userStripeReplayer, numStripesPerSegment / 2, lastLSID);

        // Then : Replay the end LSID to current SSD LSID
        ReplayUserStripe(&userStripeReplayer, &allocatorSegmentCtx, &arrayInfo, numStripesPerSegment - 1);
    }
}

// TODO(huijeong.kim): These scenarios to checks whether ActiveUserStripeReplayer
// operates normally even if the same segment is reused after GC
// Additional replayer refactoring is required for the case to succeed.
// Therefore, these tests are currently failed
TEST(ActiveUserStripeReplayer, DISABLED_Replay_ReplaySequentialLsidAfterGC)
{
    // Given
    NiceMock<MockISegmentCtx> allocatorSegmentCtx;
    NiceMock<MockIArrayInfo> arrayInfo;
    uint32_t numSegment = 3;
    uint32_t beforeGc = 10;
    uint32_t numTests = numStripesPerSegment * numSegment;

    ActiveUserStripeReplayer userStripeReplayer(&allocatorSegmentCtx, &arrayInfo);

    // When : Find sequential LSIDs from beginning to middle within several segment
    UpdateUserStripeSequentially(&userStripeReplayer, beforeGc, numTests - beforeGc);
    UpdateUserStripeSequentially(&userStripeReplayer, 0, beforeGc);
    // Then : Replay the last LSID to current SSD LSID
    ReplayUserStripe(&userStripeReplayer, &allocatorSegmentCtx, &arrayInfo, beforeGc);

    // When : Find sequential LSIDs from beginning to middle within several segment
    UpdateUserStripeSequentially(&userStripeReplayer, beforeGc, numStripesPerSegment - beforeGc);
    // Then : Replay the last LSID to current SSD LSID
    ReplayUserStripe(&userStripeReplayer, &allocatorSegmentCtx, &arrayInfo, numStripesPerSegment - 1);
}

TEST(ActiveUserStripeReplayer, DISABLED_Replay_ReplayRandomLsidAfterGC)
{
    // Given
    NiceMock<MockISegmentCtx> allocatorSegmentCtx;
    NiceMock<MockIArrayInfo> arrayInfo;
    uint32_t numSegment = 3;
    uint32_t beforeGc = 10;
    uint32_t numTests = numStripesPerSegment * numSegment;

    ActiveUserStripeReplayer userStripeReplayer(&allocatorSegmentCtx, &arrayInfo);

    // When : Find sequential LSIDs from beginning to middle within several segment
    UpdateUserStripeRandomly(&userStripeReplayer, beforeGc, numTests - beforeGc);
    UpdateUserStripeRandomly(&userStripeReplayer, 0, beforeGc);
    // Then : Replay the last LSID to current SSD LSID
    ReplayUserStripe(&userStripeReplayer, &allocatorSegmentCtx, &arrayInfo, beforeGc);

    // When : Find sequential LSIDs from beginning to middle within several segment
    UpdateUserStripeRandomly(&userStripeReplayer, beforeGc, numStripesPerSegment - beforeGc);
    // Then : Replay the last LSID to current SSD LSID
    ReplayUserStripe(&userStripeReplayer, &allocatorSegmentCtx, &arrayInfo, numStripesPerSegment - 1);
}
} // namespace pos

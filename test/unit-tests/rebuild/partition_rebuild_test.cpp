#include "src/rebuild/partition_rebuild.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <list>

#include "test/unit-tests/array/rebuild/rebuild_context_mock.h"
#include "test/unit-tests/array_models/dto/partition_physical_size_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/rebuild/rebuild_behavior_factory_mock.h"
#include "test/unit-tests/rebuild/rebuild_behavior_mock.h"

using ::testing::_;
using ::testing::ByMove;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
TEST(PartitionRebuild, StartRebuild_testIfRebuildStartsWithoutBehavior)
{
    // Given
    PartitionRebuild* partRebuild = new PartitionRebuild(nullptr);

    // When
    partRebuild->Start(nullptr);
}

TEST(PartitionRebuild, StartRebuild_testIfRebuildStateIsRebuilding)
{
    string arrayName = "POSArray";
    NiceMock<MockEventScheduler> mockEventScheduler;
    ON_CALL(mockEventScheduler, EnqueueEvent(_)).WillByDefault(Return());
    unique_ptr<RebuildContext> ctx = make_unique<RebuildContext>();
    ctx->logger = new RebuildLogger(arrayName);
    ctx->prog = new RebuildProgress(arrayName);
    ctx->stripeCnt = 1024;
    PartitionPhysicalSize size;
    ctx->size = &size;
    MockRebuildBehavior* bhvr = new MockRebuildBehavior(move(ctx));
    PartitionRebuild* partRebuild = new PartitionRebuild(bhvr, &mockEventScheduler);

    // When
    partRebuild->Start(nullptr);

    // Then
    ASSERT_EQ(RebuildState::REBUILDING, partRebuild->GetResult());
}

TEST(PartitionRebuild, StopRebuild_testIfRebuildStateIsCancelled)
{
    // Given
    string arrayName = "POSArray";
    unique_ptr<RebuildContext> ctx = make_unique<RebuildContext>();
    ctx->logger = new RebuildLogger(arrayName);
    ctx->prog = new RebuildProgress(arrayName);
    PartitionPhysicalSize size;
    ctx->size = &size;
    ctx->SetResult(RebuildState::REBUILDING);
    NiceMock<MockEventScheduler> mockEventScheduler;
    ON_CALL(mockEventScheduler, EnqueueEvent(_)).WillByDefault(Return());
    MockRebuildBehavior* bhvr = new MockRebuildBehavior(move(ctx));
    PartitionRebuild* partRebuild = new PartitionRebuild(bhvr, &mockEventScheduler);

    // When
    partRebuild->Stop();

    // Then
    ASSERT_EQ(RebuildState::CANCELLED, partRebuild->GetResult());
}

TEST(PartitionRebuild, TotalStripeCnt_testIfTotalStripeCountIsCorrect)
{
    // Given
    string arrayName = "POSArray";
    unique_ptr<RebuildContext> ctx = make_unique<RebuildContext>();
    ctx->logger = new RebuildLogger(arrayName);
    ctx->prog = new RebuildProgress(arrayName);
    PartitionPhysicalSize size;
    ctx->size = &size;
    ctx->stripeCnt = 1024;
    MockRebuildBehavior* bhvr = new MockRebuildBehavior(move(ctx));
    NiceMock<MockEventScheduler> mockEventScheduler;
    ON_CALL(mockEventScheduler, EnqueueEvent(_)).WillByDefault(Return());
    PartitionRebuild* partRebuild = new PartitionRebuild(bhvr, &mockEventScheduler);

    // When
    uint64_t actual = partRebuild->TotalStripes();

    // Then
    ASSERT_EQ(1024, actual);
}
} // namespace pos

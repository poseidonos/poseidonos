#include "src/rebuild/partition_rebuild.h"
#include "test/unit-tests/array/rebuild/rebuild_context_mock.h"
#include "test/unit-tests/array_models/dto/partition_physical_size_mock.h"
#include "test/unit-tests/rebuild/rebuild_behavior_mock.h"
#include "test/unit-tests/rebuild/rebuild_behavior_factory_mock.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <list>

using ::testing::_;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ByMove;
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
    unique_ptr<RebuildContext> ctx = make_unique<RebuildContext>();
    ctx->logger = new RebuildLogger(arrayName);
    ctx->prog  = new RebuildProgress(arrayName);
    ctx->stripeCnt = 1024;
    MockRebuildBehavior* bhvr = new MockRebuildBehavior(move(ctx), nullptr);
    PartitionRebuild* partRebuild = new PartitionRebuild(bhvr);

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
    ctx->prog  = new RebuildProgress(arrayName);
    ctx->result = RebuildState::REBUILDING;
    MockRebuildBehavior* bhvr = new MockRebuildBehavior(move(ctx), nullptr);
    PartitionRebuild* partRebuild = new PartitionRebuild(bhvr);

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
    ctx->prog  = new RebuildProgress(arrayName);
    ctx->stripeCnt = 1024;
    MockRebuildBehavior* bhvr = new MockRebuildBehavior(move(ctx), nullptr);
    PartitionRebuild* partRebuild = new PartitionRebuild(bhvr);

    // When
    uint64_t actual = partRebuild->TotalStripes();

    // Then
    ASSERT_EQ(1024, actual);
}
} // namespace pos

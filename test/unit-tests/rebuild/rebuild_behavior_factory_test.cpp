#include <gtest/gtest.h>
#include "src/rebuild/rebuild_behavior_factory.h"
#include "src/include/partition_type.h"
#include "src/array_models/dto/partition_physical_size.h"

namespace pos
{
TEST(RebuildBehaviorFactory, testIfRebuildBehaviorForJournalPartitionIsCreatedInWTMode)
{
    // Given
    RebuildBehaviorFactory factory(nullptr);
    unique_ptr<RebuildContext> ctx = make_unique<RebuildContext>();
    const PartitionPhysicalSize physicalSize{ // not interesting
        .startLba = 0,
        .lastLba = 0,
        .blksPerChunk = 10,
        .chunksPerStripe = 5,
        .stripesPerSegment = 20,
        .totalSegments = 100};

    ctx->part = PartitionType::JOURNAL_SSD;
    ctx->size = &physicalSize;

    // When
    RebuildBehavior* bhvr = factory.CreateRebuildBehavior(move(ctx));
    StripeBasedRaceRebuild* rbd = dynamic_cast<StripeBasedRaceRebuild*>(bhvr);

    // Then
    ASSERT_NE(rbd, nullptr);
}
}  // namespace pos

#include "src/rebuild/array_rebuild.h"
#include "test/unit-tests/device/base/ublock_device_mock.h"
#include "test/unit-tests/array/device/array_device_mock.h"
#include "test/unit-tests/array/rebuild/rebuild_target_mock.h"
#include "test/unit-tests/array/rebuild/rebuild_context_mock.h"
#include "test/unit-tests/array_models/dto/partition_physical_size_mock.h"
#include "test/unit-tests/rebuild/rebuild_behavior_mock.h"
#include "test/unit-tests/rebuild/stripe_based_race_rebuild_mock.h"
#include "test/unit-tests/rebuild/segment_based_rebuild_mock.h"
#include "test/unit-tests/rebuild/partition_rebuild_mock.h"
#include "test/unit-tests/rebuild/rebuild_behavior_factory_mock.h"
#include "test/unit-tests/allocator/i_context_manager_mock.h"
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
TEST(ArrayRebuild, ArrayRebuild_testConstructor)
{
    // Given
    string arrayName = "POSArray";
    list<RebuildTarget*> targetPartitions;
    MockRebuildTarget metaPart(PartitionType::META_SSD);
    MockRebuildTarget dataPart(PartitionType::USER_DATA);
    EXPECT_CALL(metaPart, GetRebuildCtx).WillRepeatedly(Return(ByMove(nullptr)));
    EXPECT_CALL(dataPart, GetRebuildCtx).WillRepeatedly(Return(ByMove(nullptr)));
    targetPartitions.push_back(&dataPart);
    targetPartitions.push_back(&metaPart);

    // When
    ArrayRebuild* ar = new ArrayRebuild(arrayName, 0, nullptr, nullptr, targetPartitions, nullptr);

    // Then
}

TEST(ArrayRebuild, Start_testIfJobCanStartWhenTaskIsNotEmpty)
{
    // Given
    string arrayName = "POSArray";
    shared_ptr<MockUBlockDevice> mockDev = make_shared<MockUBlockDevice>("unvme-ns-0", 0, nullptr);
    MockArrayDevice arrayDev(mockDev);
    RebuildProgress* prog = new RebuildProgress(arrayName);
    RebuildLogger* logger = new RebuildLogger(arrayName);
    MockRebuildBehaviorFactory mockFactory(nullptr);
    MockRebuildBehavior* mockBehavior = new MockRebuildBehavior(nullptr, nullptr);
    EXPECT_CALL(mockFactory, CreateRebuildBehavior).WillRepeatedly(Return(mockBehavior));
    list<PartitionRebuild*> targetPartitions;
    MockPartitionRebuild mockPartitionRebuild(mockBehavior);
    targetPartitions.push_back(&mockPartitionRebuild);

    // When
    ArrayRebuild* ar = new ArrayRebuild();
    ar->Init(arrayName, &arrayDev, nullptr, targetPartitions, prog, logger);
    ar->Start();
    // Then
}

TEST(ArrayRebuild, StartRebuild_testIfJobContainsEmptyTaskWhenArrayRebuildStart)
{
    // Given
    string arrayName = "POSArray";
    shared_ptr<MockUBlockDevice> mockDev = make_shared<MockUBlockDevice>("unvme-ns-0", 0, nullptr);
    MockArrayDevice arrayDev(mockDev);
    list<RebuildTarget*> targetPartitions;

    // When
    ArrayRebuild* ar = new ArrayRebuild(arrayName, 0, &arrayDev, nullptr, targetPartitions, nullptr);
    ar->Start();
    // Then
}

TEST(ArrayRebuild, DiscardRebuild_testIfNeedToDiscardBecauseThereAreNoTasks)
{
    // Given
    string arrayName = "POSArray";
    shared_ptr<MockUBlockDevice> mockDev = make_shared<MockUBlockDevice>("unvme-ns-0", 0, nullptr);
    MockArrayDevice arrayDev(mockDev);
    list<RebuildTarget*> targetPartitions;

    // When
    ArrayRebuild* ar = new ArrayRebuild(arrayName, 0, &arrayDev, nullptr, targetPartitions, nullptr);
    ar->Discard();
    RebuildState state = ar->GetState();

    // Then
    ASSERT_EQ(RebuildState::FAIL, state);
}

TEST(ArrayRebuild, StopRebuild_testIfNeedToStopRebuild)
{
    // Given
    string arrayName = "POSArray";
    shared_ptr<MockUBlockDevice> mockDev = make_shared<MockUBlockDevice>("unvme-ns-0", 0, nullptr);
    MockArrayDevice arrayDev(mockDev);
    RebuildProgress* prog = new RebuildProgress(arrayName);
    RebuildLogger* logger = new RebuildLogger(arrayName);
    MockRebuildBehaviorFactory mockFactory(nullptr);
    MockRebuildBehavior* mockBehavior = new MockRebuildBehavior(nullptr, nullptr);
    EXPECT_CALL(mockFactory, CreateRebuildBehavior).WillRepeatedly(Return(mockBehavior));
    list<PartitionRebuild*> targetPartitions;
    MockPartitionRebuild mockPartitionRebuild(mockBehavior);
    targetPartitions.push_back(&mockPartitionRebuild);

    // When
    ArrayRebuild* ar = new ArrayRebuild();
    ar->Init(arrayName, &arrayDev, nullptr, targetPartitions, prog, logger);
    ar->Start();
    ar->Stop();
}

TEST(ArrayRebuild, GetState_testIfStateIsReadyBeforeRebuildStarts)
{
    // Given
    string arrayName = "POSArray";
    list<RebuildTarget*> targetPartitions;
    MockRebuildTarget metaPart(PartitionType::META_SSD);
    MockRebuildTarget dataPart(PartitionType::USER_DATA);
    EXPECT_CALL(metaPart, GetRebuildCtx).WillRepeatedly(Return(ByMove(nullptr)));
    EXPECT_CALL(dataPart, GetRebuildCtx).WillRepeatedly(Return(ByMove(nullptr)));
    targetPartitions.push_back(&dataPart);
    targetPartitions.push_back(&metaPart);

    // When
    ArrayRebuild* ar = new ArrayRebuild(arrayName, 0, nullptr, nullptr, targetPartitions, nullptr);
    RebuildState state = ar->GetState();

    // Then
    ASSERT_EQ(RebuildState::READY, state);
}

TEST(ArrayRebuild, GetProgress_testIfProgressIsZeroBeforeStartRebuild)
{
    // Given
    string arrayName = "POSArray";
    shared_ptr<MockUBlockDevice> mockDev = make_shared<MockUBlockDevice>("unvme-ns-0", 0, nullptr);
    MockArrayDevice arrayDev(mockDev);
    RebuildProgress* prog = new RebuildProgress(arrayName);
    prog->Update("meta", 0, 100);
    prog->Update("data", 0, 100);
    RebuildLogger* logger = new RebuildLogger(arrayName);
    MockRebuildBehaviorFactory mockFactory(nullptr);
    MockRebuildBehavior* mockBehavior = new MockRebuildBehavior(nullptr, nullptr);
    EXPECT_CALL(mockFactory, CreateRebuildBehavior).WillRepeatedly(Return(mockBehavior));
    list<PartitionRebuild*> targetPartitions;
    MockPartitionRebuild mockPartitionRebuild(mockBehavior);
    targetPartitions.push_back(&mockPartitionRebuild);

    // When
    ArrayRebuild* ar = new ArrayRebuild();
    ar->Init(arrayName, &arrayDev, nullptr, targetPartitions, prog, logger);

    // Then
    uint64_t progress = ar->GetProgress();
    ASSERT_EQ(0, progress);
}

} // namespace pos

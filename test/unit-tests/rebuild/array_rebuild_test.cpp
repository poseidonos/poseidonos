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
// string arrayName, uint32_t arrayId, vector<IArrayDevice*>& dst,
//     RebuildComplete cb, list<RebuildTarget*>& tgt, RebuildBehaviorFactory* factor
    // When
    vector<IArrayDevice*> devs;
    ArrayRebuild* ar = new ArrayRebuild(arrayName, 0, devs, nullptr, targetPartitions, nullptr);

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
    vector<IArrayDevice*> devs{&arrayDev};
    ArrayRebuild* ar = new ArrayRebuild(arrayName, 0, devs, nullptr, targetPartitions, nullptr);
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
    vector<IArrayDevice*> devs{&arrayDev};
    ArrayRebuild* ar = new ArrayRebuild(arrayName, 0, devs, nullptr, targetPartitions, nullptr);
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
    MockRebuildBehaviorFactory mockFactory(nullptr);
    MockRebuildBehavior* mockBehavior = new MockRebuildBehavior(nullptr);
    EXPECT_CALL(mockFactory, CreateRebuildBehavior).WillRepeatedly(Return(mockBehavior));
    list<RebuildTarget*> targetPartitions;
    MockRebuildTarget mockRebuildTarget(PartitionType::USER_DATA);
    targetPartitions.push_back(&mockRebuildTarget);

    // When
    vector<IArrayDevice*> devs{&arrayDev};
    ArrayRebuild* ar = new ArrayRebuild(arrayName, 0, devs, nullptr, targetPartitions, &mockFactory);
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
    vector<IArrayDevice*> devs;
    ArrayRebuild* ar = new ArrayRebuild(arrayName, 0, devs, nullptr, targetPartitions, nullptr);
    RebuildState state = ar->GetState();

    // Then
    ASSERT_EQ(RebuildState::READY, state);
}

TEST(ArrayRebuild, GetProgress_testIfProgressIs100BeforeStartRebuildSinceTotalIsZero)
{
    // Given
    string arrayName = "POSArray";
    shared_ptr<MockUBlockDevice> mockDev = make_shared<MockUBlockDevice>("unvme-ns-0", 0, nullptr);
    MockArrayDevice arrayDev(mockDev, ArrayDeviceState::NORMAL, 0);
    MockRebuildBehaviorFactory mockFactory(nullptr);
    MockRebuildBehavior* mockBehavior = new MockRebuildBehavior(nullptr);
    EXPECT_CALL(mockFactory, CreateRebuildBehavior).WillRepeatedly(Return(mockBehavior));
    list<RebuildTarget*> targetPartitions;
    MockRebuildTarget mockRebuildTarget(PartitionType::USER_DATA);
    EXPECT_CALL(mockRebuildTarget, GetRebuildCtx).WillRepeatedly(Return(ByMove(unique_ptr<RebuildContext>())));
    targetPartitions.push_back(&mockRebuildTarget);

    // When
    vector<IArrayDevice*> devs{&arrayDev};
    ArrayRebuild* ar = new ArrayRebuild(arrayName, 0, devs, nullptr, targetPartitions, &mockFactory);

    // Then
    uint64_t progress = ar->GetProgress();
    ASSERT_EQ(100, progress);
}

} // namespace pos

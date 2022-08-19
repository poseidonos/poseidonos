#include "src/rebuild/array_rebuilder.h"
#include "test/unit-tests/rebuild/interface/i_rebuild_notification_mock.h"
#include "test/unit-tests/device/base/ublock_device_mock.h"
#include "test/unit-tests/array/device/array_device_mock.h"
#include "test/unit-tests/array/rebuild/rebuild_target_mock.h"
#include "test/unit-tests/array/rebuild/rebuild_context_mock.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <list>
#include <vector>

using ::testing::_;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ByMove;
using ::testing::ReturnRef;

namespace pos
{

TEST(ArrayRebuilder, Rebuild_testIfMetaPartitionAndDataPartitionTriggerRebuild)
{
    // Given
    string arrayName = "POSArray";
    MockIRebuildNotification mockRebuildNoti;
    bool resume_input = false;
    EXPECT_CALL(mockRebuildNoti, PrepareRebuild).WillOnce([resume_input](string arrayName, bool& resume)
    {
        resume = resume_input;
        return 0;
    });

    ArrayRebuilder* rebuilder = new ArrayRebuilder(&mockRebuildNoti);
    shared_ptr<MockUBlockDevice> mockDev = make_shared<MockUBlockDevice>("unvme-ns-0", 0, nullptr);
    MockArrayDevice arrayDev(mockDev);
    list<RebuildTarget*> targetPartitions;
    MockRebuildTarget metaPart(false);
    MockRebuildTarget dataPart(true);
    EXPECT_CALL(metaPart, GetRebuildCtx).WillRepeatedly(Return(ByMove(nullptr)));
    EXPECT_CALL(dataPart, GetRebuildCtx).WillRepeatedly(Return(ByMove(nullptr)));
    targetPartitions.push_back(&dataPart);
    targetPartitions.push_back(&metaPart);

    // When
    rebuilder->Rebuild(arrayName, 0, vector<IArrayDevice*>{&arrayDev}, nullptr, targetPartitions);

    // Then
    bool ret = rebuilder->IsRebuilding(arrayName);
    ASSERT_TRUE(ret);
}

TEST(ArrayRebuilder, ResumeRebuild_testIfNeedToResumeRebuild)
{
    // Given
    string arrayName = "POSArray";
    MockIRebuildNotification mockRebuildNoti;
    bool resume_input = true;
    EXPECT_CALL(mockRebuildNoti, PrepareRebuild).WillOnce([resume_input](string arrayName, bool& resume)
    {
        resume = resume_input;
        return 0;
    });
    ArrayRebuilder* rebuilder = new ArrayRebuilder(&mockRebuildNoti);
    shared_ptr<MockUBlockDevice> mockDev = make_shared<MockUBlockDevice>("unvme-ns-0", 0, nullptr);
    MockArrayDevice arrayDev(mockDev);
    MockRebuildTarget metaPart(false);
    MockRebuildTarget dataPart(true);
    EXPECT_CALL(metaPart, GetRebuildCtx).WillRepeatedly(Return(ByMove(nullptr)));
    EXPECT_CALL(dataPart, GetRebuildCtx).WillRepeatedly(Return(ByMove(nullptr)));
    list<RebuildTarget*> targetPartitions;
    targetPartitions.push_back(&dataPart);
    targetPartitions.push_back(&metaPart);

    // When
    rebuilder->Rebuild(arrayName, 0, vector<IArrayDevice*>{&arrayDev}, nullptr, targetPartitions);

    // Then : note that meta partition is removed from rebuild target
    ASSERT_EQ(1, targetPartitions.size());
}

TEST(ArrayRebuilder, Discard_testErrorOccuredDuringPrepareRebuild)
{
    // Given
    string arrayName = "POSArray";
    MockIRebuildNotification mockRebuildNoti;
    bool resume_input = false;
    EXPECT_CALL(mockRebuildNoti, PrepareRebuild).WillOnce([resume_input](string arrayName, bool& resume)
    {
        resume = resume_input;
        int error_ret = 3000;
        return error_ret;
    });
    ArrayRebuilder* rebuilder = new ArrayRebuilder(&mockRebuildNoti);
    shared_ptr<MockUBlockDevice> mockDev = make_shared<MockUBlockDevice>("unvme-ns-0", 0, nullptr);
    MockArrayDevice arrayDev(mockDev);
    list<RebuildTarget*> targetPartitions;
    MockRebuildTarget metaPart(false);
    MockRebuildTarget dataPart(true);
    EXPECT_CALL(metaPart, GetRebuildCtx).WillRepeatedly(Return(ByMove(nullptr)));
    EXPECT_CALL(dataPart, GetRebuildCtx).WillRepeatedly(Return(ByMove(nullptr)));
    targetPartitions.push_back(&dataPart);
    targetPartitions.push_back(&metaPart);

    // When
    RebuildResult rebResult;
    rebuilder->Rebuild(arrayName, 0, vector<IArrayDevice*>{&arrayDev},
        [&rebResult](RebuildResult res) -> void { rebResult = res; }, targetPartitions);

    // Then
    ASSERT_EQ(RebuildState::FAIL, rebResult.result);
}

TEST(ArrayRebuilder, StopRebuild_testIfJobInProgressInvokesStopMethod)
{
    // Given
    string arrayName = "POSArray";
    MockIRebuildNotification mockRebuildNoti;
    bool resume_input = false;
    EXPECT_CALL(mockRebuildNoti, PrepareRebuild).WillOnce([resume_input](string arrayName, bool& resume)
    {
        resume = resume_input;
        return 0;
    });

    ArrayRebuilder* rebuilder = new ArrayRebuilder(&mockRebuildNoti);
    shared_ptr<MockUBlockDevice> mockDev = make_shared<MockUBlockDevice>("unvme-ns-0", 0, nullptr);
    MockArrayDevice arrayDev(mockDev);
    list<RebuildTarget*> targetPartitions;
    MockRebuildTarget metaPart(false);
    MockRebuildTarget dataPart(true);
    EXPECT_CALL(metaPart, GetRebuildCtx).WillRepeatedly(Return(ByMove(nullptr)));
    EXPECT_CALL(dataPart, GetRebuildCtx).WillRepeatedly(Return(ByMove(nullptr)));
    targetPartitions.push_back(&dataPart);
    targetPartitions.push_back(&metaPart);
    rebuilder->Rebuild(arrayName, 0, vector<IArrayDevice*>{&arrayDev}, nullptr, targetPartitions);

    // When
    rebuilder->StopRebuild(arrayName);

    // Then
    bool ret = rebuilder->IsRebuilding(arrayName);
    ASSERT_TRUE(ret);
}

}  // namespace pos


#include "src/array_components/array_mount_sequence.h"

#include <gtest/gtest.h>

#include "src/include/pos_event_id.h"
#include "test/unit-tests/array_models/interface/i_mount_sequence_mock.h"
#include "test/unit-tests/state/state_control_mock.h"
#include "test/unit-tests/volume/volume_manager_mock.h"
#include "test/unit-tests/array/rebuild/i_array_rebuilder_mock.h"

using ::testing::NiceMock;
using ::testing::Return;
namespace pos
{
TEST(ArrayMountSequence, ArrayMountSequence_testConstructor)
{
    // Given
    vector<IMountSequence*> emptySeq;
    MockStateControl stateControl;
    MockIArrayRebuilder* mockRebuilder = new MockIArrayRebuilder();
    EXPECT_CALL(stateControl, Subscribe).Times(1);
    EXPECT_CALL(stateControl, Unsubscribe).Times(1);

    // When
    ArrayMountSequence mntSeq(emptySeq, &stateControl, "mock-array", nullptr, nullptr, nullptr, nullptr, mockRebuilder);

    // Then
}

TEST(ArrayMountSequence, Mount_testIfEverySequenceIsInitialized)
{
    // Given
    MockIMountSequence mockSeq1, mockSeq2, mockSeq3;
    vector<IMountSequence*> seqVec = {&mockSeq1, &mockSeq2, &mockSeq3};
    MockStateControl stateControl;
    StateContext* mockDefaultState = new StateContext("sender", SituationEnum::DEFAULT);
    StateContext* mockMountState = new StateContext("sender", SituationEnum::TRY_MOUNT);
    MockIArrayRebuilder* mockRebuilder = new MockIArrayRebuilder();

    EXPECT_CALL(stateControl, Subscribe).Times(1);
    EXPECT_CALL(stateControl, Unsubscribe).Times(1);
    EXPECT_CALL(stateControl, Invoke).Times(2); // first for mount and second for normal. Ideally, I'd like to capture the param and assert on it, but in a later PR
    EXPECT_CALL(stateControl, GetState)
        .WillOnce(Return(mockDefaultState))
        .WillOnce(Return(mockMountState));
    EXPECT_CALL(mockSeq1, Init).WillOnce(Return(0));
    EXPECT_CALL(mockSeq2, Init).WillOnce(Return(0));
    EXPECT_CALL(mockSeq3, Init).WillOnce(Return(0));
    EXPECT_CALL(stateControl, Remove).Times(1);

    ArrayMountSequence mntSeq(seqVec, &stateControl, "mock-array", mockMountState, nullptr, nullptr, nullptr, mockRebuilder);

    // When
    int actual = mntSeq.Mount();

    // Then
    ASSERT_EQ(EID(SUCCESS), actual);
}

TEST(ArrayMountSequence, Mount_testIfPartiallyFailedSequenceLeadsToDisposeOnEverySequence)
{
    // Given
    MockIMountSequence mockSeq1, mockSeq2, mockSeq3;
    vector<IMountSequence*> seqVec = {&mockSeq1, &mockSeq2, &mockSeq3};
    MockStateControl stateControl;
    StateContext* mockDefaultState = new StateContext("sender", SituationEnum::DEFAULT);
    StateContext* mockMountState = new StateContext("sender", SituationEnum::TRY_MOUNT);
    MockIArrayRebuilder* mockRebuilder = new MockIArrayRebuilder();

    int SEQ3_INIT_FAILURE = 123;
    EXPECT_CALL(stateControl, Subscribe).Times(1);
    EXPECT_CALL(stateControl, Unsubscribe).Times(1);
    EXPECT_CALL(stateControl, Invoke).Times(1); // only for mount (unlike all success case)
    EXPECT_CALL(stateControl, GetState)
        .WillOnce(Return(mockDefaultState))
        .WillOnce(Return(mockMountState));
    EXPECT_CALL(mockSeq1, Init).WillOnce(Return(0));
    EXPECT_CALL(mockSeq2, Init).WillOnce(Return(0));
    EXPECT_CALL(mockSeq3, Init).WillOnce(Return(SEQ3_INIT_FAILURE));

    EXPECT_CALL(mockSeq1, Dispose).Times(1);
    EXPECT_CALL(mockSeq2, Dispose).Times(1);
    EXPECT_CALL(mockSeq3, Dispose).Times(1);
    EXPECT_CALL(stateControl, Remove).Times(1);

    ArrayMountSequence mntSeq(seqVec, &stateControl, "mock-array", mockMountState, nullptr, nullptr, nullptr, mockRebuilder);

    // When
    int actual = mntSeq.Mount();

    // Then
    ASSERT_EQ(SEQ3_INIT_FAILURE, actual);
}

TEST(ArrayMountSequence, Unmount_testIfFailsToUnmountWhenInFaultSituation)
{
    // Given
    vector<IMountSequence*> emptySeq;
    NiceMock<MockStateControl> stateControl;
    StateContext mockStopState("sender", SituationEnum::FAULT);
    MockIArrayRebuilder* mockRebuilder = new MockIArrayRebuilder();

    EXPECT_CALL(stateControl, GetState).WillOnce(Return(&mockStopState));

    ArrayMountSequence mntSeq(emptySeq, &stateControl, "mock-array", nullptr, nullptr, nullptr, nullptr, mockRebuilder);

    // When
    int actual = mntSeq.Unmount();

    // Then
    ASSERT_EQ(EID(UNMOUNT_ARRAY_ALREADY_UNMOUNTED), actual);
}

TEST(ArrayMountSequence, Unmount_testIfEverySequenceIsDisposed)
{
    // Given
    MockIMountSequence mockSeq1, mockSeq2, mockSeq3;
    vector<IMountSequence*> seqVec = {&mockSeq1, &mockSeq2, &mockSeq3};

    NiceMock<MockStateControl> stateControl;
    StateContext* mockUnmountState = new StateContext("sender", SituationEnum::TRY_UNMOUNT);
    MockVolumeManager mockVolMgr(nullptr, &stateControl);
    StateContext mockInitialState("sender", SituationEnum::NORMAL);
    MockIArrayRebuilder* mockRebuilder = new MockIArrayRebuilder();

    EXPECT_CALL(stateControl, Invoke).Times(1);
    EXPECT_CALL(stateControl, GetState)
        .WillOnce(Return(&mockInitialState))
        .WillOnce(Return(mockUnmountState));
    EXPECT_CALL(mockVolMgr, DetachVolumes).Times(1);
    EXPECT_CALL(mockSeq3, Dispose).Times(1);
    EXPECT_CALL(mockSeq2, Dispose).Times(1);
    EXPECT_CALL(mockSeq1, Dispose).Times(1);
    EXPECT_CALL(stateControl, Remove).Times(2);

    ArrayMountSequence mntSeq(seqVec, &stateControl, "mock-array", nullptr, mockUnmountState, nullptr, &mockVolMgr, mockRebuilder);

    // When
    int actual = mntSeq.Unmount();

    // Then
    ASSERT_EQ(0, actual);
}

TEST(ArrayMountSequence, StateChanged_testIfShutdownAndFlushAreInvokedWhenStateChangesFromNormalToStop)
{
    // Given
    MockIMountSequence mockSeq1, mockSeq2, mockSeq3;
    vector<IMountSequence*> arrayMntSeq{&mockSeq1, &mockSeq2, &mockSeq3};
    NiceMock<MockStateControl> stateControl;
    MockVolumeManager mockVolMgr(nullptr, &stateControl);
    MockIArrayRebuilder* mockRebuilder = new MockIArrayRebuilder();

    ArrayMountSequence mntSeq(arrayMntSeq, &stateControl, "mock-array", nullptr, nullptr, nullptr, &mockVolMgr, mockRebuilder);

    StateContext stopContext("sender", SituationEnum::FAULT);
    StateContext normalContext("sender", SituationEnum::NORMAL);
    EXPECT_CALL(mockVolMgr, DetachVolumes).Times(1);
    EXPECT_CALL(mockSeq3, Shutdown).Times(1);
    EXPECT_CALL(mockSeq2, Shutdown).Times(1);
    EXPECT_CALL(mockSeq1, Shutdown).Times(1);

    EXPECT_CALL(mockSeq3, Flush).Times(1);
    EXPECT_CALL(mockSeq2, Flush).Times(1);
    EXPECT_CALL(mockSeq1, Flush).Times(1);

    // When & Then: trivial. as long as there's no exception, I'm good
    mntSeq.StateChanged(&normalContext, &stopContext);
}

TEST(ArrayMountSequence, StateChanged_testIfFlushIsntInvokedWhenStateChangesFromOfflineToStop)
{
    // Given
    MockIMountSequence mockSeq1, mockSeq2, mockSeq3;
    vector<IMountSequence*> arrayMntSeq{&mockSeq1, &mockSeq2, &mockSeq3};
    NiceMock<MockStateControl> stateControl;
    MockVolumeManager mockVolMgr(nullptr, &stateControl);
    MockIArrayRebuilder mockRebuilder;

    ArrayMountSequence mntSeq(arrayMntSeq, &stateControl, "mock-array", nullptr, nullptr, nullptr, &mockVolMgr, &mockRebuilder);

    StateContext stopContext("sender", SituationEnum::FAULT);
    StateContext offlineContext("sender", SituationEnum::DEFAULT);
    EXPECT_CALL(mockRebuilder, StopRebuild).Times(0);
    EXPECT_CALL(mockRebuilder, WaitRebuildDone).Times(0);
    EXPECT_CALL(mockVolMgr, DetachVolumes).Times(0);
    EXPECT_CALL(mockSeq3, Shutdown).Times(0);
    EXPECT_CALL(mockSeq2, Shutdown).Times(0);
    EXPECT_CALL(mockSeq1, Shutdown).Times(0);

    // flush shouldn't be invoked
    EXPECT_CALL(mockSeq3, Flush).Times(0);
    EXPECT_CALL(mockSeq2, Flush).Times(0);
    EXPECT_CALL(mockSeq1, Flush).Times(0);

    // When & Then: trivial. as long as there's no exception, I'm good
    mntSeq.StateChanged(&offlineContext, &stopContext);
}

// Disabled because Array.Shutdown() is still necessary even in OFFLINE state so that the array could be
// deleted by an admin through CLI. Without Shutdown() call, the array delete command would get timed out
// because 'shutdownFlag' wouldn't be set to 1.
TEST(ArrayMountSequence, DISABLED_StateChanged_testIfShutdownIsNotInvokedWhenNextStateIsStopAndPreviousStateIsOffline)
{
    // Given
    MockIMountSequence mockSeq1, mockSeq2, mockSeq3;
    vector<IMountSequence*> arrayMntSeq{&mockSeq1, &mockSeq2, &mockSeq3};
    NiceMock<MockStateControl> stateControl;
    MockVolumeManager mockVolMgr(nullptr, &stateControl);
    MockIArrayRebuilder* mockRebuilder = new MockIArrayRebuilder();

    ArrayMountSequence mntSeq(arrayMntSeq, &stateControl, "mock-array", nullptr, nullptr, nullptr, &mockVolMgr, mockRebuilder);

    StateContext stopContext("sender", SituationEnum::FAULT);
    StateContext offlineContext("sender", SituationEnum::DEFAULT);
    EXPECT_CALL(mockVolMgr, DetachVolumes).Times(0);
    EXPECT_CALL(mockSeq3, Shutdown).Times(0);
    EXPECT_CALL(mockSeq2, Shutdown).Times(0);
    EXPECT_CALL(mockSeq1, Shutdown).Times(0);

    // When & Then: trivial. as long as there's no exception, I'm good
    mntSeq.StateChanged(&offlineContext, &stopContext);
}

} // namespace pos

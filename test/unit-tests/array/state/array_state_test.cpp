#include "src/array/state/array_state.h"

#include <gtest/gtest.h>

#include "src/include/pos_event_id.h"
#include "test/unit-tests/state/interface/i_state_control_mock.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
namespace pos
{
auto ARRAY_STATES_NOT_MOUNTED = {
    ArrayStateEnum::NOT_EXIST,
    ArrayStateEnum::EXIST_NORMAL,
    ArrayStateEnum::EXIST_DEGRADED,
    ArrayStateEnum::BROKEN,
    ArrayStateEnum::TRY_MOUNT,
    ArrayStateEnum::TRY_UNMOUNT};
auto ARRAYS_STATES_EXISTS_BUT_NOT_MOUNTED = {
    ArrayStateEnum::EXIST_NORMAL,
    ArrayStateEnum::EXIST_DEGRADED,
    ArrayStateEnum::BROKEN,
    ArrayStateEnum::TRY_MOUNT,
    ArrayStateEnum::TRY_UNMOUNT};
auto ARRAYS_STATES_MOUNTED = {
    ArrayStateEnum::NORMAL,
    ArrayStateEnum::DEGRADED,
    ArrayStateEnum::REBUILD};
auto ARRAYS_STATES = {
    ArrayStateEnum::NOT_EXIST,
    ArrayStateEnum::EXIST_NORMAL,
    ArrayStateEnum::EXIST_DEGRADED,
    ArrayStateEnum::BROKEN,
    ArrayStateEnum::TRY_MOUNT,
    ArrayStateEnum::TRY_UNMOUNT,
    ArrayStateEnum::NORMAL,
    ArrayStateEnum::DEGRADED,
    ArrayStateEnum::REBUILD};
static map<ArrayStateEnum, ArrayStateEnum> STATE_TRANSITION_FOR_DATA_REMOVED = {
    {ArrayStateEnum::EXIST_NORMAL, ArrayStateEnum::EXIST_DEGRADED},
    {ArrayStateEnum::EXIST_DEGRADED, ArrayStateEnum::BROKEN},
    {ArrayStateEnum::NORMAL, ArrayStateEnum::DEGRADED},
    {ArrayStateEnum::REBUILD, ArrayStateEnum::BROKEN /* when isRebuildingDevice is false*/},
    {ArrayStateEnum::DEGRADED, ArrayStateEnum::BROKEN}};

TEST(ArrayState, ArrayState_testIfSelfIsSubscribed)
{
    // Given
    MockIStateControl mockIStateControl;
    EXPECT_CALL(mockIStateControl, Subscribe).Times(1);
    EXPECT_CALL(mockIStateControl, Unsubscribe).Times(1);

    // When
    ArrayState arrayState(&mockIStateControl);

    // Then
}

TEST(ArrayState, SetLoad_testIfBecomesExistNormalWhenThereIsZeroMissingBrokenDevice)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    ArrayState arrayState(&mockIStateControl);
    arrayState.SetState(ArrayStateEnum::TRY_MOUNT);

    EXPECT_CALL(mockIStateControl, Invoke).Times(0);

    // When
    arrayState.SetLoad(0, 0);

    // Then
    ArrayStateType expected(ArrayStateEnum::EXIST_NORMAL);
    ASSERT_EQ(expected, arrayState.GetState());
}

TEST(ArrayState, SetLoad_testIfBecomesExistDegradedFromNormalWhenThereIsOneBrokenDevice)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    ArrayState arrayState(&mockIStateControl);
    arrayState.SetState(ArrayStateEnum::TRY_MOUNT);

    // When
    arrayState.SetLoad(0, 1);

    // Then
    ArrayStateType expected(ArrayStateEnum::EXIST_DEGRADED);
    ASSERT_EQ(expected, arrayState.GetState());
}

TEST(ArrayState, SetLoad_testIfBecomesBrokenFromNormalWhenThereIsOneMissingAndOneBrokenDevice)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    ArrayState arrayState(&mockIStateControl);
    arrayState.SetState(ArrayStateEnum::TRY_MOUNT);

    EXPECT_CALL(mockIStateControl, Invoke).WillOnce([](StateContext* ctx)
    {
        SituationType actual = ctx->GetSituation();
        SituationType expected(SituationEnum::FAULT);
        ASSERT_EQ(expected, actual);
    });

    // When
    arrayState.SetLoad(1, 1);

    // Then
    ArrayStateType expected(ArrayStateEnum::BROKEN);
    ASSERT_EQ(expected, arrayState.GetState());
}

TEST(ArrayState, SetCreate_testIfBecomesExistNormalWhenCreated)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    ArrayState arrayState(&mockIStateControl);
    arrayState.SetState(ArrayStateEnum::NOT_EXIST);

    // When
    arrayState.SetCreate();

    // Then
    ArrayStateType expected(ArrayStateEnum::EXIST_NORMAL);
    ASSERT_EQ(expected, arrayState.GetState());
}

TEST(ArrayState, SetDelete_testIfStateIsNotifiedAndBecomesNotExist)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    ArrayState arrayState(&mockIStateControl);

    EXPECT_CALL(mockIStateControl, Remove).WillOnce([](StateContext* ctx)
    {
        SituationType actual = ctx->GetSituation();
        SituationType expected(SituationEnum::FAULT);
        ASSERT_EQ(expected, actual);
    });

    EXPECT_CALL(mockIStateControl, Invoke).Times(0);

    // When
    arrayState.SetDelete();

    // Then
    ArrayStateType expected(ArrayStateEnum::NOT_EXIST);
    ASSERT_EQ(expected, arrayState.GetState());
}

TEST(ArrayState, SetRebuild_testIfSetRebuildFailsWhenArrayIsNotRebuildable)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    ArrayState arrayState(&mockIStateControl);
    arrayState.SetState(ArrayStateEnum::BROKEN);

    // When
    bool actual = arrayState.SetRebuild();

    // Then
    ASSERT_FALSE(actual);
}

TEST(ArrayState, SetRebuildDone_testRebuildDoneSuccessWhenRebuildState)
{
    // Given : Rebuild State
    uint32_t brokenDeviceCnt = 1;
    NiceMock<MockIStateControl>* mockSC = new NiceMock<MockIStateControl>;
    ArrayState arrayState(mockSC);
    arrayState.SetState(ArrayStateEnum::REBUILD);
    ArrayStateEnum expectedState = ArrayStateEnum::REBUILD;
    ArrayStateEnum currentState = arrayState.GetState();
    EXPECT_EQ(expectedState, currentState);

    // When : Rebuild Success
    arrayState.SetRebuildDone(true);

    // Then : Normal State
    expectedState = ArrayStateEnum::NORMAL;
    currentState = arrayState.GetState();
    EXPECT_EQ(expectedState, currentState);
}

TEST(ArrayState, SetRebuildDone_testRebuildDoneSuccessWhenBrokenState)
{
    // Given : Broken State
    uint32_t brokenDeviceCnt = 2;
    NiceMock<MockIStateControl>* mockSC = new NiceMock<MockIStateControl>;
    ArrayState arrayState(mockSC);
    arrayState.SetState(ArrayStateEnum::BROKEN);
    ArrayStateEnum expectedState = ArrayStateEnum::BROKEN;
    ArrayStateEnum currentState = arrayState.GetState();
    EXPECT_EQ(expectedState, currentState);

    // When : Rebuild Success
    arrayState.SetRebuildDone(true);

    // Then : Cannot Be Normal
    expectedState = ArrayStateEnum::NORMAL;
    currentState = arrayState.GetState();
    EXPECT_NE(expectedState, currentState);
}

TEST(ArrayState, SetMount_testIfStateChangeIsSuccessfulForExistNormalAndDegraded)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    ArrayState arrayState(&mockIStateControl);

    // When 1
    arrayState.SetState(ArrayStateEnum::EXIST_NORMAL);
    arrayState.SetMount();

    // Then 1
    ASSERT_EQ(ArrayStateEnum::NORMAL, arrayState.GetState());

    // When 2
    arrayState.SetState(ArrayStateEnum::EXIST_DEGRADED);
    arrayState.SetMount();

    // Then 2
    ASSERT_EQ(ArrayStateEnum::DEGRADED, arrayState.GetState());

    for (auto arrayStateEnum : ARRAYS_STATES)
    {
        if (ArrayStateEnum::EXIST_NORMAL == arrayStateEnum || ArrayStateEnum::EXIST_DEGRADED == arrayStateEnum)
        {
            continue;
        }

        arrayState.SetState(arrayStateEnum);

        // When
        int actual = arrayState.SetMount();

        // Then
        ASSERT_EQ(EID(ARRAY_STATE_CHANGE_ERROR), actual);
    }
}

TEST(ArrayState, SetUnmount_testIfNormalBecomesExistNormal)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    ArrayState arrayState(&mockIStateControl);

    // When 1
    arrayState.SetState(ArrayStateEnum::NORMAL);
    arrayState.SetUnmount();

    // Then 1
    ASSERT_EQ(ArrayStateEnum::EXIST_NORMAL, arrayState.GetState());

    // When 2
    arrayState.SetState(ArrayStateEnum::DEGRADED);
    arrayState.SetUnmount();

    // Then 2
    ASSERT_EQ(ArrayStateEnum::EXIST_DEGRADED, arrayState.GetState());

    for (auto arrayStateEnum : ARRAYS_STATES)
    {
        if (ArrayStateEnum::NORMAL == arrayStateEnum || ArrayStateEnum::DEGRADED == arrayStateEnum)
        {
            continue;
        }

        // When 3
        arrayState.SetState(arrayStateEnum);
        int actual = arrayState.SetUnmount();

        // Then 3
        ASSERT_EQ(EID(ARRAY_STATE_CHANGE_ERROR), actual);
        ASSERT_EQ(arrayStateEnum, arrayState.GetState());
    }
}

TEST(ArrayState, SetDegraded_testIfAnyStateCanBecomeDegraded)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    ArrayState arrayState(&mockIStateControl);

    for (auto arrayStateEnum : ARRAYS_STATES)
    {
        arrayState.SetState(arrayStateEnum);

        // When
        arrayState.SetDegraded();

        // Then
        ASSERT_EQ(ArrayStateEnum::DEGRADED, arrayState.GetState());
    }
}

TEST(ArrayState, CanAddSpare_testIfWeCannotAddSpareWhenArrayIsNotMounted)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    ArrayState arrayState(&mockIStateControl);

    for (auto arrayStateEnum : ARRAY_STATES_NOT_MOUNTED)
    {
        arrayState.SetState(arrayStateEnum);

        // When
        int actual = arrayState.CanAddSpare();

        // Then
        ASSERT_EQ(EID(ARRAY_NEED_MOUNT), actual);
    }
}

TEST(ArrayState, CanRemoveSpare_testIfWeCannotRemoveSpareWhenArrayIsNotMounted)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    ArrayState arrayState(&mockIStateControl);

    for (auto arrayStateEnum : ARRAY_STATES_NOT_MOUNTED)
    {
        arrayState.SetState(arrayStateEnum);

        // When
        int actual = arrayState.CanRemoveSpare();

        // Then
        ASSERT_EQ(EID(ARRAY_NEED_MOUNT), actual);
    }
}

TEST(ArrayState, IsLoadable_testIfNotMountedArrayIsNotLoadable)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    ArrayState arrayState(&mockIStateControl);

    for (auto arrayStateEnum : ARRAY_STATES_NOT_MOUNTED)
    {
        arrayState.SetState(arrayStateEnum);

        // When
        int actual = arrayState.IsLoadable();

        // Then
        if (arrayStateEnum == ArrayStateEnum::BROKEN)
        {
            ASSERT_EQ(EID(ARRAY_BROKEN_ERROR), actual);
        }
        else
        {
            ASSERT_EQ(0, actual);
        }
    }
}

TEST(ArrayState, IsCreatable_testIfErrorEventIsReturnedWhenArrayExistsButNotIsNotMounted)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    ArrayState arrayState(&mockIStateControl);

    // When
    for (auto arrayStateEnum : ARRAYS_STATES_EXISTS_BUT_NOT_MOUNTED)
    {
        arrayState.SetState(arrayStateEnum);

        // When
        int actual = arrayState.IsCreatable();

        // Then
        ASSERT_EQ(EID(ARRAY_STATE_EXIST), actual);
    }
}

TEST(ArrayState, IsCreatable_testIfErrorEventIsReturnedWhenArrayIsMountedAlready)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    ArrayState arrayState(&mockIStateControl);

    for (auto arrayStateEnum : ARRAYS_STATES_MOUNTED)
    {
        arrayState.SetState(arrayStateEnum);

        // When
        int actual = arrayState.IsCreatable();

        // Then
        ASSERT_EQ(EID(ARRAY_STATE_ONLINE), actual);
    }
}

/***
 * Almost trivial getter/setter tests. 
 * Please note that IsMountable() returns an error event, not boolean.
 **/
TEST(ArrayState, IsMountable_testIfEventIdIsReturnedProperly)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    ArrayState arrayState(&mockIStateControl);

    // When 1: in NOT_EXIST state
    arrayState.SetState(ArrayStateEnum::NOT_EXIST);
    int actual = arrayState.IsMountable();

    // Then 1
    ASSERT_EQ(EID(ARRAY_STATE_NOT_EXIST), actual);

    // When 2
    arrayState.SetState(ArrayStateEnum::EXIST_DEGRADED);
    actual = arrayState.IsMountable();

    // Then 2
    ASSERT_EQ(0, actual);

    // When 3
    arrayState.SetState(ArrayStateEnum::BROKEN);
    actual = arrayState.IsMountable();

    // Then 3
    ASSERT_EQ(EID(ARRAY_STATE_BROKEN), actual);
}

TEST(ArrayState, IsUnmountable_testIfErrorEventIsReturnedWhenArrayIsNotMountedYet)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    ArrayState arrayState(&mockIStateControl);

    for (auto arrayStateEnum : ARRAY_STATES_NOT_MOUNTED)
    {
        arrayState.SetState(arrayStateEnum);

        // When
        int actual = arrayState.IsUnmountable();

        // Then
        ASSERT_EQ(EID(ARRAY_STATE_OFFLINE), actual);
    }
}

TEST(ArrayState, IsUnmountable_testIfErrorEventIsReturnedWhenArrayIsMountedButInRebuilding)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    ArrayState arrayState(&mockIStateControl);

    for (auto arrayStateEnum : ARRAYS_STATES_MOUNTED)
    {
        arrayState.SetState(arrayStateEnum);

        // When
        int actual = arrayState.IsUnmountable();

        // Then
        ArrayStateType rebuildState(ArrayStateEnum::REBUILD);
        if (ArrayStateEnum::REBUILD == arrayStateEnum)
        {
            ASSERT_EQ(EID(ARRAY_STATE_REBUILDING), actual);
        }
        else
        {
            ASSERT_EQ(0, actual);
        }
    }
}

TEST(ArrayState, IsDeletable_testIfErrorEventIsReturnedWhenArrayIsMounted)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    ArrayState arrayState(&mockIStateControl);

    for (auto arrayStateEnum : ARRAYS_STATES_MOUNTED)
    {
        arrayState.SetState(arrayStateEnum);

        // When
        int actual = arrayState.IsDeletable();

        // Then
        ASSERT_EQ(EID(ARRAY_STATE_ONLINE), actual);
    }
}

TEST(ArrayState, IsDeletable_testIfErrorEventIsReturnedWhenArrayExistsButNotIsMounted)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    ArrayState arrayState(&mockIStateControl);
    arrayState.SetState(ArrayStateEnum::NOT_EXIST); // the only state that's neither mounted nor existent

    // When
    int actual = arrayState.IsDeletable();

    // Then
    ASSERT_EQ(EID(ARRAY_STATE_NOT_EXIST), actual);
}

TEST(ArrayState, IsRebuildable_testIfDegradedStateIsRebuildable)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    ArrayState arrayState(&mockIStateControl);

    for (auto arrayStateEnum : ARRAYS_STATES)
    {
        arrayState.SetState(arrayStateEnum);

        // When
        bool actual = arrayState.IsRebuildable();

        // Then
        if (ArrayStateEnum::DEGRADED == arrayStateEnum)
        {
            ASSERT_TRUE(actual);
        }
        else
        {
            ASSERT_FALSE(actual);
        }
    }
}

TEST(ArrayState, IsRecoverable_testIfOnlyTwoStatesAreRecoverable)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    ArrayState arrayState(&mockIStateControl);

    // When
    for (auto arrayStateEnum : ARRAYS_STATES)
    {
        arrayState.SetState(arrayStateEnum);

        // When
        bool actual = arrayState.IsRecoverable();

        // Then
        if (ArrayStateEnum::EXIST_DEGRADED == arrayStateEnum || ArrayStateEnum::DEGRADED == arrayStateEnum)
        {
            ASSERT_TRUE(actual);
        }
        else
        {
            ASSERT_FALSE(actual);
        }
    }
}

TEST(ArrayState, DataRemoved_testIfStateChangesProperly)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    ArrayState arrayState(&mockIStateControl);

    for (auto const& entry : STATE_TRANSITION_FOR_DATA_REMOVED)
    {
        // When
        arrayState.SetState(entry.first);
        arrayState.DataRemoved(false);

        // Then
        ASSERT_EQ(entry.second, arrayState.GetState());
        if (entry.first == ArrayStateEnum::REBUILD)
        {
            // When isRebuildlingDevice is true
            arrayState.SetState(entry.first);
            arrayState.DataRemoved(true);

            // Then
            ASSERT_EQ(ArrayStateEnum::DEGRADED, arrayState.GetState());
        }
    }
}

TEST(ArrayState, Exists_)
{
}

TEST(ArrayState, IsMounted_)
{
}

TEST(ArrayState, IsBroken_testIfBrokenStateReturnsTrue)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    ArrayState arrayState(&mockIStateControl);

    for (auto arrayStateEnum : ARRAYS_STATES)
    {
        arrayState.SetState(arrayStateEnum);

        // When
        bool actual = arrayState.IsBroken();

        // Then
        if (ArrayStateEnum::BROKEN == arrayStateEnum)
        {
            ASSERT_TRUE(actual);
        }
        else
        {
            ASSERT_FALSE(actual);
        }
    }
}

TEST(ArrayState, GetState_testGetter)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    ArrayState arrayState(&mockIStateControl);

    // When
    ArrayStateType actual = arrayState.GetState();

    // Then: the default state should be in the return
    ASSERT_EQ(ArrayStateEnum::NOT_EXIST, actual);
}

TEST(ArrayState, GetSysState_testIfIStateControlIsQueried)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    ArrayState arrayState(&mockIStateControl);
    EXPECT_CALL(mockIStateControl, GetState).WillOnce(Return(nullptr));

    // When
    StateContext* actual = arrayState.GetSysState();

    // Then
    ASSERT_EQ(nullptr, actual);
}

TEST(ArrayState, StateChanged_testMethodCall)
{
    // Given: prev and next state
    NiceMock<MockIStateControl> mockIStateControl;
    ArrayState arrayState(&mockIStateControl);

    // When
    arrayState.StateChanged(nullptr, nullptr);

    // Then: trivial. nothing to verify
}

} // namespace pos

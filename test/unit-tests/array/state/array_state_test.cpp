#include "src/array/state/array_state.h"

#include <gtest/gtest.h>

#include "test/unit-tests/state/interface/i_state_control_mock.h"

using ::testing::_;
using ::testing::NiceMock;
namespace pos
{
TEST(ArrayState, ArrayState_)
{
}

TEST(ArrayState, SetLoad_)
{
}

TEST(ArrayState, SetCreate_)
{
}

TEST(ArrayState, SetDelete_)
{
}

TEST(ArrayState, SetRebuild_)
{
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

TEST(ArrayState, SetMount_)
{
}

TEST(ArrayState, SetUnmount_)
{
}

TEST(ArrayState, SetDegraded_)
{
}

TEST(ArrayState, CanAddSpare_)
{
}

TEST(ArrayState, CanRemoveSpare_)
{
}

TEST(ArrayState, IsLoadable_)
{
}

TEST(ArrayState, IsCreatable_)
{
}

TEST(ArrayState, IsMountable_)
{
}

TEST(ArrayState, IsUnmountable_)
{
}

TEST(ArrayState, IsDeletable_)
{
}

TEST(ArrayState, IsRebuildable_)
{
}

TEST(ArrayState, IsRecoverable_)
{
}

TEST(ArrayState, DataRemoved_)
{
}

TEST(ArrayState, Exists_)
{
}

TEST(ArrayState, IsMounted_)
{
}

TEST(ArrayState, IsBroken_)
{
}

TEST(ArrayState, GetState_)
{
}

TEST(ArrayState, GetSysState_)
{
}

TEST(ArrayState, StateChanged_)
{
}

TEST(ArrayState, _SetState_)
{
}

TEST(ArrayState, _WaitState_)
{
}

} // namespace pos

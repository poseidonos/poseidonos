#include "src/journal_manager/replay/replay_state_changer.h"

#include <gtest/gtest.h>

#include <future>

#include "test/unit-tests/state/interface/i_state_control_mock.h"
#include "test/unit-tests/state/state_context_mock.h"

using ::testing::DoAll;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(ReplayStateChanger, ReplayStateChanger_testIfConstructedSuccessfully)
{
    ReplayStateChanger defaultStateChanger;

    NiceMock<MockIStateControl> state;
    ReplayStateChanger productStateChanger(&state);

    {
        NiceMock<MockStateContext>* recoverContext = new NiceMock<MockStateContext>("journal_test", SituationEnum::JOURNAL_RECOVERY);
        ReplayStateChanger utStateChanger(&state, recoverContext);
    }

    {
        NiceMock<MockStateContext>* recoverContext = new NiceMock<MockStateContext>("journal_test", SituationEnum::JOURNAL_RECOVERY);
        ReplayStateChanger* stateChanger = new ReplayStateChanger(&state, recoverContext);
        delete stateChanger;
    }
}

TEST(ReplayStateChanger, GetRecoverState_testIfStateIsReturnedRightAway)
{
    NiceMock<MockIStateControl> state;
    NiceMock<MockStateContext>* recoverContext = new NiceMock<MockStateContext>("journal_test", SituationEnum::JOURNAL_RECOVERY);
    ReplayStateChanger stateChanger(&state, recoverContext);

    EXPECT_CALL(state, Invoke(recoverContext));
    EXPECT_CALL(state, GetState).WillOnce(Return(recoverContext));

    int result = stateChanger.GetRecoverState();
    EXPECT_EQ(result, 0);
}

TEST(ReplayStateChanger, RemoveRecoverState_testIfStateRemoved)
{
    NiceMock<MockIStateControl> state;
    NiceMock<MockStateContext>* recoverContext = new NiceMock<MockStateContext>("journal_test", SituationEnum::JOURNAL_RECOVERY);
    ReplayStateChanger stateChanger(&state, recoverContext);

    EXPECT_CALL(state, Remove(recoverContext));

    int result = stateChanger.RemoveRecoverState();
    EXPECT_EQ(result, 0);
}

TEST(ReplayStateChanger, StateChanged_testIfStateChangeIsNotified)
{
    NiceMock<MockIStateControl> state;
    NiceMock<MockStateContext>* recoverContext = new NiceMock<MockStateContext>("journal_test", SituationEnum::JOURNAL_RECOVERY);
    ReplayStateChanger stateChanger(&state, recoverContext);

    stateChanger.StateChanged(nullptr, recoverContext);
}
} // namespace pos

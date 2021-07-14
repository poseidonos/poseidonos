#include "src/state/state_publisher.h"

#include <gtest/gtest.h>
#include "test/unit-tests/state/interface/i_state_observer_mock.h"
using ::testing::Return;
using ::testing::InSequence;

namespace pos
{
TEST(StatePublisher, StatePublisher_testConstructor)
{
    // trivial test
    StatePublisher publisher;
}

TEST(StatePublisher, Notify_testIfOnlyObserversAddedAndNotRemovedGetNotification)
{
    // Given: three observers were added and one of them was removed and one of them has "obs3" as next->Owner()
    MockIStateObserver obs1, obs2, obs3;
    StatePublisher publisher;
    publisher.Add(&obs1, "obs1");
    publisher.Add(&obs2, "obs2");
    publisher.Add(&obs3, "obs3");
    publisher.Remove(&obs1);

    string NEXT_OWNER = "obs2";
    StateContext nextState(NEXT_OWNER, SituationEnum::DEFAULT /* not interesting */);

    InSequence s;  // this enforces the sequenced invocations between obs3 and obs2
    EXPECT_CALL(obs1, StateChanged).Times(0);  // shouldn't be invoked
    EXPECT_CALL(obs3, StateChanged).Times(1);  // should be invoked first
    EXPECT_CALL(obs2, StateChanged).Times(1);  // should be invoked second

    // When: we notify
    publisher.Notify(nullptr, &nextState);

    // Then: verify the expectation
}

} // namespace pos

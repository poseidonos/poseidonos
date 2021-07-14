#include "src/state/state_control.h"

#include <gtest/gtest.h>
#include "test/unit-tests/state/state_list_mock.h"
#include "test/unit-tests/state/state_publisher_mock.h"
#include "test/unit-tests/state/state_context_mock.h"

using ::testing::NiceMock;
namespace pos
{
TEST(StateControl, StateControl_testIfCompilerCanHandleNullptrDelete)
{
    // Just to validate if our compiler can handle this
    StateControl* s = nullptr;
    delete s;
}

TEST(StateControl, StateControl_testIfStateContextIsAddedToStateList)
{
    // Given
    MockStateList* mockStateList = new MockStateList(nullptr);
    EXPECT_CALL(*mockStateList, Add).Times(1);

    // When
    StateControl sc(nullptr, nullptr, mockStateList);

    // Then: verify the expectation
}

TEST(StateControl, Subscribe_testIfObserverIsAddedToPublisher)
{
    // Given
    NiceMock<MockStateList>* mockStateList = new NiceMock<MockStateList>(nullptr);
    MockStatePublisher* mockStatePublisher = new MockStatePublisher();
    StateControl sc(nullptr, mockStatePublisher, mockStateList);
    EXPECT_CALL(*mockStatePublisher, Add).Times(1);

    // When
    sc.Subscribe(nullptr, "by-whom");

    // Then: verify the expectation
}

TEST(StateControl, Unsubscribe_testIfObserverIsRemovedFromPublisher)
{
    // Given
    NiceMock<MockStateList>* mockStateList = new NiceMock<MockStateList>(nullptr);
    MockStatePublisher* mockStatePublisher = new MockStatePublisher();
    StateControl sc(nullptr, mockStatePublisher, mockStateList);
    EXPECT_CALL(*mockStatePublisher, Remove).Times(1);

    // When
    sc.Unsubscribe(nullptr);

    // Then: verify the expectation
}

TEST(StateControl, GetState_testGetter)
{
    // Given
    NiceMock<MockStateList>* mockStateList = new NiceMock<MockStateList>(nullptr);
    MockStateContext mockStateContext("sender", SituationEnum::DEFAULT);
    StateControl sc(&mockStateContext, nullptr, mockStateList);

    // When
    StateContext* actual = sc.GetState();

    // Then
    ASSERT_EQ(&mockStateContext, actual);
}

TEST(StateControl, Invoke_testIfStateIsAddedToList)
{
    // Given
    NiceMock<MockStateList>* mockStateList = new NiceMock<MockStateList>(nullptr);
    StateControl sc(nullptr, nullptr, mockStateList);
    EXPECT_CALL(*mockStateList, Add).Times(1);

    // When
    sc.Invoke(nullptr);
    sc.WaitOnInvokeFuture();

    // Then: verify the expectation
}

TEST(StateControl, Remove_testIfCtxIsRemovedFromStateList)
{
    // Given
    NiceMock<MockStateList>* mockStateList = new NiceMock<MockStateList>(nullptr);
    StateControl sc(nullptr, nullptr, mockStateList);
    EXPECT_CALL(*mockStateList, Remove).Times(1);

    // When
    sc.Remove(nullptr);

    // Then: verify the expectation
}

TEST(StateControl, Exists_testIfStateListCallsRemove)
{
    // Given
    NiceMock<MockStateList>* mockStateList = new NiceMock<MockStateList>(nullptr);
    StateControl sc(nullptr, nullptr, mockStateList);
    EXPECT_CALL(*mockStateList, Exists).Times(1);

    // When
    bool actual = sc.Exists(SituationEnum::DEFAULT /* not interesting */);

    // Then
}

} // namespace pos

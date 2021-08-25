#include "src/state/state_list.h"

#include <gtest/gtest.h>
#include "test/unit-tests/state/state_context_mock.h"

using ::testing::Return;
namespace pos
{
TEST(StateList, StateList_testConstructor)
{
    // Given & When & Then. It's okay as long as it doesn't throw an exception
    StateList stateList();
}

TEST(StateList, Add_testIfCtxIsAddedInSortedListWhenItDoesNotExistYet)
{
    // Given
    MockStateContext mockExistingStateContext("sender2", SituationEnum::DEFAULT);
    vector<StateContext*> contextList =
    {
        &mockExistingStateContext
    };
    int invokeCountForListUpdated = 0;
    StateList::ListUpdatedHandler mockHandler = [&invokeCountForListUpdated](StateContext* ctx, StateContext* ctx2)
    {
        invokeCountForListUpdated++;
        return;
    };
    StateList stateList(mockHandler);
    stateList.SetContextList(contextList);
    MockStateContext mockNewStateContext("sender1", SituationEnum::DEFAULT);
    EXPECT_CALL(mockExistingStateContext, GetPriority).WillOnce(Return(100));
    EXPECT_CALL(mockNewStateContext, GetPriority).WillOnce(Return(200));  // i.e., this has higher priority than existing one's
    EXPECT_CALL(mockNewStateContext, GetSituation).WillOnce(Return(SituationEnum::DEFAULT));

    // When
    stateList.Add(&mockNewStateContext);

    // Then
    ASSERT_EQ(2, stateList.GetContextList().size());
    auto itor = stateList.GetContextList().begin();
    ASSERT_EQ(&mockNewStateContext, *itor);
    itor++;
    ASSERT_EQ(&mockExistingStateContext, *itor);
    ASSERT_EQ(1, invokeCountForListUpdated);
}

TEST(StateList, Add_testIfCtxIsNotAddedWhenItExistsAlready)
{
    // Given
    MockStateContext mockExistingStateContext("sender1", SituationEnum::DEFAULT);
    vector<StateContext*> contextList =
    {
        &mockExistingStateContext
    };
    int invokeCountForListUpdated = 0;
    StateList::ListUpdatedHandler mockHandler = [&invokeCountForListUpdated](StateContext* ctx, StateContext* ctx2)
    {
        invokeCountForListUpdated++;
        return;
    };
    StateList stateList(mockHandler);
    stateList.SetContextList(contextList);

    // When: we try to add the same state context again
    stateList.Add(&mockExistingStateContext);

    // Then
    ASSERT_EQ(1, stateList.GetContextList().size());
    auto itor = stateList.GetContextList().begin();
    ASSERT_EQ(&mockExistingStateContext, *itor);
    ASSERT_EQ(0, invokeCountForListUpdated);
}

TEST(StateList, Remove_testIfCtxIsRemovedFromList)
{
    // Given
    MockStateContext mockExistingStateContext("sender1", SituationEnum::DEFAULT);
    vector<StateContext*> contextList =
    {
        &mockExistingStateContext
    };
    int invokeCountForListUpdated = 0;
    StateList::ListUpdatedHandler mockHandler = [&invokeCountForListUpdated](StateContext* ctx, StateContext* ctx2)
    {
        invokeCountForListUpdated++;
        return;
    };
    StateList stateList(mockHandler);
    stateList.SetContextList(contextList);

    // When
    stateList.Remove(&mockExistingStateContext);

    // Then
    ASSERT_EQ(0, stateList.GetContextList().size());
    ASSERT_EQ(1, invokeCountForListUpdated);
}

TEST(StateList, Remove_testIfListRemainsTheSameWhenRemovalFails)
{
    // Given
    MockStateContext mockExistingStateContext("sender1", SituationEnum::DEFAULT);
    vector<StateContext*> contextList =
    {
        &mockExistingStateContext
    };
    int invokeCountForListUpdated = 0;
    StateList::ListUpdatedHandler mockHandler = [&invokeCountForListUpdated](StateContext* ctx, StateContext* ctx2)
    {
        invokeCountForListUpdated++;
        return;
    };
    StateList stateList(mockHandler);
    stateList.SetContextList(contextList);
    MockStateContext mockNotExistingStateContext("sender2", SituationEnum::DEFAULT);

    // When
    stateList.Remove(&mockNotExistingStateContext);

    // Then
    ASSERT_EQ(1, stateList.GetContextList().size());
    ASSERT_EQ(&mockExistingStateContext, stateList.GetContextList().front());
    ASSERT_EQ(0, invokeCountForListUpdated);
}

TEST(StateList, Exists_testMembershipByStateEnumAndBySituationEnum)
{
    // Given
    StateContext state1("sender1", SituationEnum::DEFAULT);
    StateContext state2("sender2", SituationEnum::NORMAL);
    StateContext state3("sender3", SituationEnum::REBUILDING);
    vector<StateContext*> contextList =
    {
        &state1, &state2, &state3
    };
    StateList stateList(nullptr);
    stateList.SetContextList(contextList);

    // When & Then
    ASSERT_TRUE(stateList.Exists(SituationEnum::DEFAULT));
    ASSERT_TRUE(stateList.Exists(SituationEnum::NORMAL));
    ASSERT_TRUE(stateList.Exists(SituationEnum::REBUILDING));
    ASSERT_FALSE(stateList.Exists(SituationEnum::TRY_MOUNT));
    ASSERT_FALSE(stateList.Exists(SituationEnum::DEGRADED));
    ASSERT_FALSE(stateList.Exists(SituationEnum::TRY_UNMOUNT));
    ASSERT_FALSE(stateList.Exists(SituationEnum::JOURNAL_RECOVERY));

    ASSERT_TRUE(stateList.Exists(StateEnum::OFFLINE));
    ASSERT_TRUE(stateList.Exists(StateEnum::NORMAL));
    ASSERT_TRUE(stateList.Exists(StateEnum::BUSY));
    ASSERT_FALSE(stateList.Exists(StateEnum::PAUSE));
}

} // namespace pos

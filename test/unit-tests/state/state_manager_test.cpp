#include "src/state/state_manager.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(StateManager, StateManager_testContructor)
{
    // trivial test
    StateManager stateMgr;
}

TEST(StateManager, CreateStateControl_testIfStateControlForNewArrayIsCreated)
{
    // Given
    string existingArray = "array1";
    StateControl existingStateControl("array");
    map<string, StateControl*> stateMap =
    {
        {
            existingArray, &existingStateControl
        }
    };
    StateManager stateMgr;
    stateMgr.SetStateMap(stateMap);

    // When 1
    IStateControl* actual = stateMgr.CreateStateControl("new-array");

    // Then 1
    ASSERT_NE(&existingStateControl, actual);
    ASSERT_EQ(2, stateMgr.GetStateMap().size());

    // When 2
    IStateControl* retrieved = stateMgr.GetStateControl("new-array");
    ASSERT_EQ(actual, retrieved);
}

TEST(StateManager, CreateStateControl_testIfStateControlForExistingArrayIsReturned)
{
    // Given
    string existingArray = "array1";
    StateControl existingStateControl("array");
    map<string, StateControl*> stateMap =
    {
        {
            existingArray, &existingStateControl
        }
    };
    StateManager stateMgr;
    stateMgr.SetStateMap(stateMap);

    // When
    IStateControl* actual = stateMgr.CreateStateControl(existingArray);

    // Then
    ASSERT_EQ(&existingStateControl, actual);
    ASSERT_EQ(1, stateMgr.GetStateMap().size());
}

TEST(StateManager, RemoveStateControl_testIfEmptyArrayNameRemovesStateFromStateMapWhenMapSizeIsOne)
{
    // Given
    string arrayName = "array1";
    StateControl* existingStateControl = new StateControl("array");
    map<string, StateControl*> stateMap =
    {
        {
            arrayName, existingStateControl
        }
    };
    StateManager stateMgr;
    stateMgr.SetStateMap(stateMap);

    // When: we pass in empty array name
    stateMgr.RemoveStateControl("");

    // Then: the existing element should be removed only when the map size is one
    ASSERT_EQ(0, stateMgr.GetStateMap().size());
}

TEST(StateManager, RemoveStateControl_testIfEmptyArrayNameDoesntRemoveStateFromStateMapWhenMapSizeIsLargerThanOne)
{
    // Given
    string array1 = "array1", array2 = "array2";
    StateControl sCtrl1("array1"), sCtrl2("array2");
    map<string, StateControl*> stateMap =
    {
        {
            array1, &sCtrl1
        },
        {
            array2, &sCtrl2
        }
    };
    StateManager stateMgr;
    stateMgr.SetStateMap(stateMap);

    // When: we pass in empty array name
    stateMgr.RemoveStateControl("");

    // Then: nothing should be removed when the map size is larger than one
    ASSERT_EQ(2, stateMgr.GetStateMap().size());
}

TEST(StateManager, RemoveStateControl_testIfExistingStateControlIsDeleted)
{
    // Given
    string array1 = "array1", array2 = "array2";
    StateControl* sCtrl1 = new StateControl("array1");
    StateControl sCtrl2("array2");  // allocate on heap since prod code performs "delete" on this

    map<string, StateControl*> stateMap =
    {
        {
            array1, sCtrl1
        },
        {
            array2, &sCtrl2
        }
    };
    StateManager stateMgr;
    stateMgr.SetStateMap(stateMap);

    // When
    stateMgr.RemoveStateControl(array1);

    // Then
    ASSERT_EQ(1, stateMgr.GetStateMap().size());
    auto retrievedMap = stateMgr.GetStateMap();
    ASSERT_EQ(&sCtrl2, retrievedMap[array2]);
}

TEST(StateManager, RemoveStateControl_testIfWrongArrayNameIsIgnored)
{
    // Given
    string array1 = "array1";
    StateControl sCtrl1("array");
    map<string, StateControl*> stateMap =
    {
        {
            array1, &sCtrl1
        }
    };
    StateManager stateMgr;
    stateMgr.SetStateMap(stateMap);

    // When
    stateMgr.RemoveStateControl("wrong-array-name");

    // Then
    ASSERT_EQ(1, stateMgr.GetStateMap().size());
    auto retrievedMap = stateMgr.GetStateMap();
    ASSERT_EQ(&sCtrl1, retrievedMap[array1]);
}

} // namespace pos

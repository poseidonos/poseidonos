#include "src/journal_manager/checkpoint/dirty_page_list.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(DirtyPageList, Add_testAddDirtyListsWhenEmpty)
{
    // Given: Dirty page list is given with map 0 to 4, page 0
    MapPageList expected;
    int numMaps = 5;
    for (int mapId = 0; mapId < numMaps; mapId++)
    {
        expected[mapId].insert(0);
    }

    DirtyPageList dirtyPageList;

    // When: Dirty pages are added to the list
    dirtyPageList.Add(expected);

    // Then: DirtyPageList should return the exact same list with input
    MapPageList actual = dirtyPageList.GetList();
    EXPECT_EQ(expected, actual);
}

TEST(DirtyPageList, Add_testAddDirtyListsWhenNotEmpty)
{
    // Given: Dirty page list is given with map 0, page 0
    MapPageList list1;
    MapPageList list2;
    int numMaps = 5;
    for (int mapId = 0; mapId < numMaps; mapId++)
    {
        list1[mapId].insert(0);
        list2[mapId].insert(1);
        list2[mapId].insert(2);
    }

    DirtyPageList dirtyPageList;

    // When: Dirty pages are added to the list
    dirtyPageList.Add(list1);
    dirtyPageList.Add(list2);

    // Then: DirtyPageList should return the exact same list with input
    MapPageList actual = dirtyPageList.GetList();
    MapPageList expected;
    for (int mapId = 0; mapId < numMaps; mapId++)
    {
        expected[mapId].insert(list1[mapId].begin(), list1[mapId].end());
        expected[mapId].insert(list2[mapId].begin(), list2[mapId].end());
    }
    EXPECT_EQ(expected, actual);
}

TEST(DirtyPageList, Reset_testIfListCleared)
{
    // Given: Dirty page list is added
    MapPageList expected;
    int numMaps = 5;
    for (int mapId = 0; mapId < numMaps; mapId++)
    {
        expected[mapId].insert(0);
    }
    DirtyPageList dirtyPageList;
    dirtyPageList.Add(expected);

    // When: Dirty pages are reset
    dirtyPageList.Reset();

    // Then: DirtyPageList should return empty list
    MapPageList actual = dirtyPageList.GetList();
    EXPECT_EQ(actual.empty(), true);
}

TEST(DirtyPageList, Delete_testIfDirtyListDeleted)
{
    // Given: Dirty page list of 5 maps are added
    MapPageList added;
    int numMaps = 5;
    for (int mapId = 0; mapId < numMaps; mapId++)
    {
        added[mapId].insert(0);
    }

    DirtyPageList dirtyPageList;
    dirtyPageList.Add(added);

    // When: Dirty pages of a map 0 is deleted
    int deletedMapId = 0;
    dirtyPageList.Delete(deletedMapId);

    // Then: DirtyPageList should not return the dirty page of map 0 and
    // do not return dirty page of the deleted map
    MapPageList expected = added;
    expected.erase(deletedMapId);

    MapPageList actual = dirtyPageList.GetList();
    EXPECT_EQ(expected, actual);
    EXPECT_EQ(actual.find(deletedMapId), actual.end());
}

TEST(DirtyPageList, Delete_testIfDirtyListDeleteSuccessWhenEmpty)
{
    // Given: Dirty page list is given with map 0 to 4
    MapPageList added;
    for (int mapId = 0; mapId < 5; mapId++)
    {
        added[mapId].insert(0);
    }
    DirtyPageList dirtyPageList;
    dirtyPageList.Add(added);

    // When: Dirty pages of a map 6 is deleted
    int deletedMapId = 6;
    dirtyPageList.Delete(deletedMapId);

    // Then: DirtyPageList should not return the added dirty pages and
    // do not return dirty page of the deleted map
    MapPageList actual = dirtyPageList.GetList();
    EXPECT_EQ(added, actual);
    EXPECT_EQ(actual.find(deletedMapId), actual.end());
}

} // namespace pos

/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "src/journal_manager/checkpoint/dirty_map_list.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(DirtyMapList, Add_testAddDirtyListsWhenEmpty)
{
    // Given: Dirty page list is given with map 0 to 4, page 0
    MapList expected;
    int numMaps = 5;
    for (int mapId = 0; mapId < numMaps; mapId++)
    {
        expected.emplace(mapId);
    }

    DirtyMapList dirtyMapList;

    // When: Dirty pages are added to the list
    dirtyMapList.Add(expected);

    // Then: DirtyMapList should return the exact same list with input
    MapList actual = dirtyMapList.GetList();
    EXPECT_EQ(expected, actual);
}

TEST(DirtyMapList, Add_testAddDirtyListsWhenNotEmpty)
{
    // Given: Dirty page list is given with map 0, page 0
    MapList list1;
    MapList list2;
    int numMaps = 5;
    for (int mapId = 0; mapId < numMaps; mapId++)
    {
        list1.emplace(mapId);
        list2.emplace(mapId);
    }

    DirtyMapList dirtyMapList;

    // When: Dirty pages are added to the list
    dirtyMapList.Add(list1);
    dirtyMapList.Add(list2);

    // Then: DirtyMapList should return the exact same list with input
    MapList actual = dirtyMapList.GetList();
    MapList expected;
    expected.insert(list1.begin(), list1.end());
    EXPECT_EQ(expected, actual);
}

TEST(DirtyMapList, Reset_testIfListCleared)
{
    // Given: Dirty page list is added
    MapList expected;
    int numMaps = 5;
    for (int mapId = 0; mapId < numMaps; mapId++)
    {
        expected.emplace(mapId);
    }
    DirtyMapList dirtyMapList;
    dirtyMapList.Add(expected);

    // When: Dirty pages are reset
    dirtyMapList.Reset();

    // Then: DirtyMapList should return empty list
    MapList actual = dirtyMapList.GetList();
    EXPECT_EQ(actual.empty(), true);
}

TEST(DirtyMapList, Delete_testIfDirtyListDeleted)
{
    // Given: Dirty page list of 5 maps are added
    MapList added;
    int numMaps = 5;
    for (int mapId = 0; mapId < numMaps; mapId++)
    {
        added.emplace(mapId);
    }

    DirtyMapList dirtyMapList;
    dirtyMapList.Add(added);

    // When: Dirty pages of a map 0 is deleted
    int deletedMapId = 0;
    dirtyMapList.Delete(deletedMapId);

    // Then: DirtyMapList should not return the dirty page of map 0 and
    // do not return dirty page of the deleted map
    MapList expected = added;
    expected.erase(deletedMapId);

    MapList actual = dirtyMapList.GetList();
    EXPECT_EQ(expected, actual);
    EXPECT_EQ(actual.find(deletedMapId), actual.end());
}

TEST(DirtyMapList, Delete_testIfDirtyListDeleteSuccessWhenEmpty)
{
    // Given: Dirty page list is given with map 0 to 4
    MapList added;
    for (int mapId = 0; mapId < 5; mapId++)
    {
        added.emplace(mapId);
    }
    DirtyMapList dirtyMapList;
    dirtyMapList.Add(added);

    // When: Dirty pages of a map 6 is deleted
    int deletedMapId = 6;
    dirtyMapList.Delete(deletedMapId);

    // Then: DirtyMapList should not return the added dirty pages and
    // do not return dirty page of the deleted map
    MapList actual = dirtyMapList.GetList();
    EXPECT_EQ(added, actual);
    EXPECT_EQ(actual.find(deletedMapId), actual.end());
}

} // namespace pos

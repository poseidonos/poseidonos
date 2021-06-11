#include "src/journal_manager/checkpoint/dirty_map_manager.h"

#include <gtest/gtest.h>

#include "test/unit-tests/journal_manager/checkpoint/dirty_page_list_mock.h"
#include "test/unit-tests/journal_manager/config/journal_configuration_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
class DirtyMapManagerTestFixture : public ::testing::Test
{
public:
    DirtyMapManagerTestFixture(void)
    : numLogGroups(2)
    {
    }

    virtual ~DirtyMapManagerTestFixture(void)
    {
    }

    virtual void
    SetUp(void)
    {
        for (int id = 0; id < numLogGroups; id++)
        {
            dirtyPageList.push_back(new NiceMock<MockDirtyPageList>());
        }
    }

    virtual void
    TearDown(void)
    {
    }

protected:
    int numLogGroups;
    std::vector<DirtyPageList*> dirtyPageList;
};

TEST_F(DirtyMapManagerTestFixture, GetDirtyList_testIfReturnsDirtyList)
{
    // Given: Dirty map manager with 2 page lists is given
    DirtyMapManager dirtyMapManager;
    dirtyMapManager.Init(dirtyPageList);

    // Given: Log group 0 list with map 0 page 0 is given
    int targetLogGroupId = 0;
    MapPageList expected;
    expected[0].insert(0);
    EXPECT_CALL(*dynamic_cast<MockDirtyPageList*>(dirtyPageList[targetLogGroupId]),
        GetList)
        .WillOnce(Return(expected));

    // When: Get dirty page of log group 0
    MapPageList actual;
    actual = dirtyMapManager.GetDirtyList(targetLogGroupId);

    // Then: Dirty map manager should return the list mock returns
    EXPECT_EQ(expected, actual);
}

TEST_F(DirtyMapManagerTestFixture, GetTotalDirtyList_testIfReturnsDirtyList)
{
    // Given: Dirty map manager with 2 page lists is given
    DirtyMapManager dirtyMapManager;
    dirtyMapManager.Init(dirtyPageList);

    // Given: Log group 0, 1 list with dirty pages is given
    MapPageList expected;
    for (int targetLogGroupId = 0; targetLogGroupId < numLogGroups; targetLogGroupId++)
    {
        MapPageList dirtyPages;
        dirtyPages[10 + targetLogGroupId].insert(5 + targetLogGroupId);
        expected.insert(dirtyPages.begin(), dirtyPages.end());

        dirtyPages[5].insert(5);
        expected.insert(dirtyPages.begin(), dirtyPages.end());

        EXPECT_CALL(*dynamic_cast<MockDirtyPageList*>(dirtyPageList[targetLogGroupId]),
            PopDirtyList)
            .WillOnce(Return(dirtyPages));
    }

    // When: Get dirty page of log group 0
    MapPageList actual;
    actual = dirtyMapManager.GetTotalDirtyList();

    // Then: Dirty map manager should return the list mock returns
    EXPECT_EQ(expected, actual);
}

TEST_F(DirtyMapManagerTestFixture, DeleteDirtyList_testIfDeletedMapDirtyListIsRemoved)
{
    // Given: Dirty map manager with 2 page lists is given
    DirtyMapManager dirtyMapManager;
    dirtyMapManager.Init(dirtyPageList);

    int volumeDeleted = 200;

    // Then: Dirty map manager should delete the dirty list
    EXPECT_CALL(*dynamic_cast<MockDirtyPageList*>(dirtyPageList[0]), Delete(volumeDeleted)).Times(1);
    EXPECT_CALL(*dynamic_cast<MockDirtyPageList*>(dirtyPageList[1]), Delete(volumeDeleted)).Times(1);

    // When: Map is deleted
    dirtyMapManager.DeleteDirtyList(volumeDeleted);
}

TEST_F(DirtyMapManagerTestFixture, LogFilled_testIfDirtyListOfLogIsAdded)
{
    // Given: Dirty map manager with 2 page lists is given
    DirtyMapManager dirtyMapManager;
    dirtyMapManager.Init(dirtyPageList);

    // Given: Dirty page list of map 0, page 0 is given
    MapPageList dirtyList;
    dirtyList[0].insert(0);

    // Then: Dirty map manager should add the list
    EXPECT_CALL(*dynamic_cast<MockDirtyPageList*>(dirtyPageList[0]), Add(dirtyList)).Times(1);

    // When: Log filled with the dirty list
    dirtyMapManager.LogFilled(0, dirtyList);
}

TEST_F(DirtyMapManagerTestFixture, LogBufferReseted_testIfDirtyListIsReset)
{
    // Given: Dirty map manager with 2 page lists is given
    DirtyMapManager dirtyMapManager;
    dirtyMapManager.Init(dirtyPageList);

    // Then: Dirty map manager should reset the list
    EXPECT_CALL(*dynamic_cast<MockDirtyPageList*>(dirtyPageList[0]), Reset).Times(1);

    // When: Log buffer reseted
    dirtyMapManager.LogBufferReseted(0);
}

} // namespace pos

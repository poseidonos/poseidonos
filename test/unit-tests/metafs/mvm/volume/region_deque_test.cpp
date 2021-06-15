#include "src/metafs/mvm/volume/region_deque.h"
#include "test/unit-tests/metafs/storage/mss_mock.h"
#include "test/unit-tests/metafs/mvm/volume/inode_table_mock.h"
#include <string>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

namespace pos
{
class RegionDequeTestFixture : public ::testing::Test
{
public:
    RegionDequeTestFixture(void)
    {
    }

    virtual ~RegionDequeTestFixture(void)
    {
    }

    virtual void
    SetUp(void)
    {
        mss = new MockMetaStorageSubsystem(arrayName);

        regionDeque = new RegionDeque(arrayName, type, &inodeContent, count, mss);
    }

    virtual void
    TearDown(void)
    {
        delete regionDeque;
        delete mss;
    }

protected:
    RegionDeque* regionDeque = nullptr;

    MockInodeTableContent inodeContent;
    MockMetaStorageSubsystem* mss = nullptr;

    std::string arrayName = "TESTARRAY";
    MetaStorageType type = MetaStorageType::SSD;
    uint64_t count = 50;
};

TEST_F(RegionDequeTestFixture, CheckReadingRegions)
{
    EXPECT_EQ(regionDeque->GetRemainedMetLpnCount(), count);
    EXPECT_EQ(regionDeque->GetReducedLpnCount(), 0);

    inodeContent.entries[0].data.basic.field.inUse = true;
    inodeContent.entries[0].data.basic.field.pagemap.baseMetaLpn = 0;
    inodeContent.entries[0].data.basic.field.pagemap.pageCnt = 5;

    regionDeque->ReadRegions();

    EXPECT_EQ(regionDeque->GetRemainedMetLpnCount(), count - 5);
    EXPECT_EQ(regionDeque->GetReducedLpnCount(), 0);
}

TEST_F(RegionDequeTestFixture, CheckCompactionFlag)
{
    EXPECT_FALSE(regionDeque->IsCompactionRequired());

    regionDeque->SetCompactionRequired(true);

    EXPECT_TRUE(regionDeque->IsCompactionRequired());
}

TEST_F(RegionDequeTestFixture, CheckSetReducedLpn)
{
    uint64_t count = 10;

    regionDeque->SetReducedLpnCount(count);

    EXPECT_EQ(regionDeque->GetReducedLpnCount(), count);
}

TEST_F(RegionDequeTestFixture, CheckAppendReducedLpn)
{
    uint64_t count = 10;

    regionDeque->SetReducedLpnCount(count);

    EXPECT_EQ(regionDeque->GetReducedLpnCount(), count);

    regionDeque->AppendReducedLpnCount(count);

    EXPECT_EQ(regionDeque->GetReducedLpnCount(), count + count);
}

TEST_F(RegionDequeTestFixture, CheckTargetRegion)
{
    uint64_t baseLpn = 6;
    uint64_t count = 1;

    inodeContent.entries[0].data.basic.field.inUse = true;
    inodeContent.entries[0].data.basic.field.pagemap.baseMetaLpn = 0;
    inodeContent.entries[0].data.basic.field.pagemap.pageCnt = 5;

    regionDeque->ReadRegions();
    regionDeque->InsertDone();

    auto result = regionDeque->GetTargetRegion(baseLpn, count);

    EXPECT_EQ(result.first, true);
    EXPECT_EQ(result.second, inodeContent.entries[0].data.basic.field.pagemap.pageCnt);
}

} // namespace pos

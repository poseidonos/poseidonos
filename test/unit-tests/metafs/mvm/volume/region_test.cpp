#include "src/metafs/mvm/volume/region.h"
#include "test/unit-tests/metafs/storage/mss_mock.h"
#include "test/unit-tests/metafs/mvm/volume/mf_inode_mock.h"
#include <string>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

namespace pos
{
class RegionTestFixture : public ::testing::Test
{
public:
    RegionTestFixture(void)
    {
    }

    virtual ~RegionTestFixture(void)
    {
    }

    virtual void
    SetUp(void)
    {
        inode = new MockMetaFileInode();
        mss = new MockMetaStorageSubsystem(arrayName);

        region = new Region(arrayName, type, inode, baseMetaLpn,
                                count, inUse, mss);
    }

    virtual void
    TearDown(void)
    {
        delete region;
        delete inode;
        delete mss;
    }

protected:
    Region* region = nullptr;

    MockMetaFileInode* inode = nullptr;
    MockMetaStorageSubsystem* mss = nullptr;

    std::string arrayName = "TESTARRAY";
    MetaStorageType type = MetaStorageType::SSD;
    uint64_t baseMetaLpn = 100;
    uint64_t count = 50;
    bool inUse = true;
};

TEST_F(RegionTestFixture, Create)
{
    EXPECT_EQ(region->GetSize(), count);
    EXPECT_EQ(region->GetInode(), inode);
    EXPECT_EQ(region->GetInUse(), inUse);
    EXPECT_EQ(region->GetBaseMetaLpn(), baseMetaLpn);
}

TEST_F(RegionTestFixture, CheckInode)
{
    EXPECT_EQ(region->GetInode(), inode);
}

TEST_F(RegionTestFixture, CheckOperation0)
{
    Region* regionTest = new Region(arrayName, type, inode, baseMetaLpn,
                                count, inUse, mss);
    MetaLpnType baseLpn = regionTest->GetContent()->GetBaseMetaLpn();

    regionTest->GetContent()->SetBaseMetaLpn(baseLpn);
    bool result = (*region < *regionTest)? true : false;
    EXPECT_FALSE(result);

    regionTest->GetContent()->SetBaseMetaLpn(baseLpn - 1);
    result = (*region < *regionTest)? true : false;
    EXPECT_FALSE(result);

    regionTest->GetContent()->SetBaseMetaLpn(baseLpn + 1);
    result = (*region < *regionTest)? true : false;
    EXPECT_TRUE(result);

    delete regionTest;
}

TEST_F(RegionTestFixture, CheckInvalidity)
{
    region->SetInUse(false);
    EXPECT_FALSE(region->GetInUse());

    region->SetInUse(true);
    EXPECT_TRUE(region->GetInUse());

    region->SetInvalid();
    EXPECT_FALSE(region->GetInUse());
    EXPECT_EQ(region->GetInode(), nullptr);
    EXPECT_EQ(region->GetContent()->GetIndexInInodeTable(), 0);
}

} // namespace pos

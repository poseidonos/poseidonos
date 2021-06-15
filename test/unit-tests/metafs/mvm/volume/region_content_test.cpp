#include "src/metafs/mvm/volume/region_content.h"
#include "src/metafs/mvm/volume/mf_inode.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(RegionContent, Create)
{
    RegionContent content;

    EXPECT_EQ(content.GetInode(), nullptr);
    EXPECT_FALSE(content.GetInUse());
}

TEST(RegionContent, CheckInode)
{
    RegionContent content;
    MetaFileInode* inode = new MetaFileInode();

    content.SetInode(inode);

    EXPECT_EQ(content.GetInode(), inode);

    delete inode;
}

TEST(RegionContent, CheckBaseMetaLpn)
{
    RegionContent content;
    MetaLpnType lpn = 100;

    content.SetBaseMetaLpn(lpn);

    EXPECT_EQ(content.GetBaseMetaLpn(), lpn);
}

TEST(RegionContent, CheckFileSize)
{
    RegionContent content;
    MetaLpnType lpn = 100;

    content.SetSize(lpn);

    EXPECT_EQ(content.GetSize(), lpn);
}

TEST(RegionContent, CheckInodeTableIndex)
{
    RegionContent content;
    uint32_t index = 100;

    content.SetIndexInInodeTable(index);

    EXPECT_EQ(content.GetIndexInInodeTable(), index);
}

TEST(RegionContent, CheckInUseFlag)
{
    RegionContent content;

    content.SetInUse(true);
    EXPECT_TRUE(content.GetInUse());

    content.SetInUse(false);
    EXPECT_FALSE(content.GetInUse());
}

} // namespace pos

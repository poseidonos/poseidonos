#include "src/metafs/mvm/volume/inode_table.h"

#include <gtest/gtest.h>

namespace pos
{
} // namespace pos

namespace pos
{
TEST(InodeTable, CreateTable)
{
    InodeTable table(MetaVolumeType::SsdVolume, 0);

    table.Create(0);

    EXPECT_EQ(table.GetFileDescriptor(0), 0);
}

TEST(InodeTable, CheckInode)
{
    InodeTable table(MetaVolumeType::SsdVolume, 0);

    MetaFileInodeArray& array = table.GetInodeArray();

    array[0].data.basic.field.age = 1;
    array[0].data.basic.field.ctime = 2;
    array[0].data.basic.field.fd = 3;
    array[0].data.basic.field.inUse = true;

    MetaFileInode& inode = table.GetInode(0);

    EXPECT_EQ(array[0].data.basic.field.age, inode.data.basic.field.age);
    EXPECT_EQ(array[0].data.basic.field.ctime, inode.data.basic.field.ctime);
    EXPECT_EQ(array[0].data.basic.field.fd, inode.data.basic.field.fd);
    EXPECT_EQ(array[0].data.basic.field.inUse, inode.data.basic.field.inUse);
}
} // namespace pos

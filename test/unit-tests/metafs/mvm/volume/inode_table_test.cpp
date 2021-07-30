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
} // namespace pos

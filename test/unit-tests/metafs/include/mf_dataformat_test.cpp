#include "src/metafs/include/mf_dataformat.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(MetaFileInodeInfo, BasicInfo)
{
    MetaFileInodeInfo info;

    EXPECT_EQ(8192, sizeof(info));
}

TEST(MetaFileInodeInfo, Operator)
{
    MetaFileInodeInfo info;

    info.data.field.inUse = 1;
    info.data.field.fd = 2;
    info.data.field.fileName[0] = '0';
    info.data.field.fileName[5] = '5';
    info.data.field.fileName[9] = '9';
    info.data.field.fileByteSize = 3;
    info.data.field.dataChunkSize = 4;
    info.data.field.dataLocation = MetaStorageType::NVRAM;
    info.data.field.fileProperty.integrity = MetaFileIntegrityType::Default;
    info.data.field.fileProperty.type = MetaFileType::General;
    info.data.field.extentCnt = 2;
    info.data.field.extentMap[0] = MetaFileExtent(0, 3);
    info.data.field.extentMap[1] = MetaFileExtent(3, 0);

    MetaFileInodeInfo copy = info;

    EXPECT_EQ(copy.data.field.inUse, 1);
    EXPECT_EQ(copy.data.field.fd, 2);
    EXPECT_EQ(copy.data.field.fileName[0], '0');
    EXPECT_EQ(copy.data.field.fileName[5], '5');
    EXPECT_EQ(copy.data.field.fileName[9], '9');
    EXPECT_EQ(copy.data.field.fileByteSize, 3);
    EXPECT_EQ(copy.data.field.dataChunkSize, 4);
    EXPECT_EQ(copy.data.field.dataLocation, MetaStorageType::NVRAM);
    EXPECT_EQ(copy.data.field.fileProperty.integrity, MetaFileIntegrityType::Default);
    EXPECT_EQ(copy.data.field.fileProperty.type, MetaFileType::General);
    EXPECT_EQ(copy.data.field.extentCnt, 2);
    EXPECT_EQ(copy.data.field.extentMap[0].GetStartLpn(), 0);
    EXPECT_EQ(copy.data.field.extentMap[0].GetCount(), 3);
    EXPECT_EQ(copy.data.field.extentMap[1].GetStartLpn(), 3);
    EXPECT_EQ(copy.data.field.extentMap[1].GetCount(), 0);
}

} // namespace pos

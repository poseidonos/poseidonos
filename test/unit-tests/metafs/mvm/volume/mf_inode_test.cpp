#include "src/metafs/mvm/volume/mf_inode.h"

#include <vector>
#include <string>
#include <cstring>
#include <gtest/gtest.h>

namespace pos
{
TEST(MetaFileInodeData, LpnCount)
{
    int count = 2;
    int totalLpnCount = 0;
    std::pair<MetaLpnType, MetaLpnType> testSet[] = {{0, 10}, {10, 10}};

    MetaFileInodeData data;
    data.basic.field.pagemapCnt = count;

    for (int i = 0; i < count; ++i)
    {
        data.basic.field.pagemap[i].SetStartLpn(testSet[i].first);
        data.basic.field.pagemap[i].SetCount(testSet[i].second);

        totalLpnCount += testSet[i].second;
    }

    EXPECT_EQ(data.basic.GetLpnCount(), totalLpnCount);
}

TEST(MetaFileInode, VersionPositive)
{
    MetaFileInodeData data;
    data.basic.field.versionSignature = MetaFsConfig::SIGNATURE_INODE_VERSION;
    data.basic.field.version = MetaFsConfig::CURRENT_INODE_VERSION;

    EXPECT_TRUE(data.basic.CheckVersion());
}

TEST(MetaFileInodeData, VersionNegative1)
{
    MetaFileInodeData data;
    data.basic.field.versionSignature = 0;
    data.basic.field.version = MetaFsConfig::CURRENT_INODE_VERSION;

    EXPECT_FALSE(data.basic.CheckVersion());
}

TEST(MetaFileInodeData, VersionNegative2)
{
    MetaFileInodeData data;
    data.basic.field.versionSignature = MetaFsConfig::SIGNATURE_INODE_VERSION;
    data.basic.field.version = 0;

    EXPECT_FALSE(data.basic.CheckVersion());
}

TEST(MetaFileInodeData, VerificationPositive)
{
    MetaFileInodeData data;
    data.basic.field.age = 1;
    data.basic.field.ageCopy = 1;
    data.basic.field.ctime = 1234567890;
    data.basic.field.ctimeCopy = 1234567890;

    EXPECT_TRUE(data.basic.CheckVerification());
}

TEST(MetaFileInodeData, VerificationNegative1)
{
    MetaFileInodeData data;
    data.basic.field.age = 1;
    data.basic.field.ageCopy = 12;
    data.basic.field.ctime = 1234567890;
    data.basic.field.ctimeCopy = 1234567890;

    EXPECT_FALSE(data.basic.CheckVerification());
}

TEST(MetaFileInodeData, VerificationNegative2)
{
    MetaFileInodeData data;
    data.basic.field.age = 1;
    data.basic.field.ageCopy = 1;
    data.basic.field.ctime = 1234567890;
    data.basic.field.ctimeCopy = 12345678901;

    EXPECT_FALSE(data.basic.CheckVerification());
}

TEST(MetaFileInode, InUse)
{
    MetaFileInode inode;

    inode.SetInUse(true);
    EXPECT_EQ(inode.IsInUse(), true);

    inode.SetInUse(false);
    EXPECT_EQ(inode.IsInUse(), false);
}

TEST(MetaFileInode, FileByteSize)
{
    FileSizeType size = 10241024;
    MetaFileInode inode;
    inode.data.basic.field.fileByteSize = size;

    EXPECT_EQ(inode.GetFileByteSize(), size);
}

TEST(MetaFileInode, StorageType)
{
    MetaStorageType type = MetaStorageType::NVRAM;
    MetaFileInode inode;
    inode.data.basic.field.ioAttribute.media = type;

    EXPECT_EQ(inode.GetStorageType(), type);
}

TEST(MetaFileInode, NewEntry)
{
    MetaFileInode inode;
    MetaFileInodeCreateReq req;
    FileSizeType size = 10;
    FileSizeType chunkSize = 15;
    std::vector<MetaFileExtent> extent;
    extent.push_back({ 0, 12 });
    std::string fileName = "TESTFILE";

    req.fd = 1;
    req.fileName = &fileName;
    req.fileByteSize = 10;
    req.media = MetaStorageType::SSD;
    req.extentList = &extent;

    inode.BuildNewEntry(req, chunkSize);

    EXPECT_EQ(inode.data.basic.field.fd, req.fd);
    int result = std::strncmp(inode.data.basic.field.fileName.ToChar(),
                            fileName.c_str(), fileName.size());
    EXPECT_EQ(result, 0);
    EXPECT_EQ(inode.data.basic.field.fileByteSize, req.fileByteSize);
    EXPECT_EQ(inode.data.basic.field.dataChunkSize, chunkSize);
    EXPECT_EQ(inode.data.basic.field.ioAttribute.media, req.media);
    EXPECT_EQ(inode.data.basic.field.ioAttribute.ioSpecfic.integrity, req.ioAttribute.integrity);
    EXPECT_EQ(inode.data.basic.field.ioAttribute.ioSpecfic.ioAccPattern, req.ioAttribute.ioAccPattern);
    EXPECT_EQ(inode.data.basic.field.ioAttribute.ioSpecfic.ioOpType, req.ioAttribute.ioOpType);
    EXPECT_EQ(inode.data.basic.field.pagemapCnt, extent.size());
    for (int i = 0; i < extent.size(); ++i)
        EXPECT_EQ(inode.data.basic.field.pagemap[i], req.extentList->at(i));
    EXPECT_TRUE(inode.data.basic.CheckVersion());
    EXPECT_TRUE(inode.data.basic.CheckVerification());
    EXPECT_TRUE(inode.data.basic.field.inUse);
}

TEST(MetaFileInode, Cleanup)
{
    MetaFileInode inode;
    inode.data.basic.field.age = 1;

    inode.CleanupEntry();

    EXPECT_NE(inode.data.basic.field.age, 1);
}

TEST(MetaFileInode, IndexInInodeTable)
{
    MetaFileInode inode;
    inode.SetIndexInInodeTable(1);

    EXPECT_EQ(inode.GetIndexInInodeTable(), 1);
}

TEST(MetaFileInode, MetaFileInfo)
{
    MetaFileInode inode;
    MetaFileInodeInfo inodeInfo;
    MetaStorageType type = MetaStorageType::NVRAM;
    std::string fileName = "TESTFILE";

    inode.data.basic.field.inUse = 1;
    inode.data.basic.field.fd = 1;
    inode.data.basic.field.fileName = &fileName;
    inode.data.basic.field.fileByteSize = 1;
    inode.data.basic.field.dataChunkSize = 1;
    inode.data.basic.field.ioAttribute.ioSpecfic.integrity = MetaFileIntegrityType::Default;
    inode.data.basic.field.ioAttribute.ioSpecfic.ioAccPattern = MetaFileAccessPattern::Default;
    inode.data.basic.field.ioAttribute.ioSpecfic.ioOpType = MetaFileDominant::Default;
    inode.data.basic.field.pagemapCnt = 1;
    inode.data.basic.field.pagemap[0].SetStartLpn(1);
    inode.data.basic.field.pagemap[0].SetCount(1);

    inode.SetMetaFileInfo(type, inodeInfo);

    EXPECT_EQ(inodeInfo.data.field.inUse, inode.data.basic.field.inUse);
    EXPECT_EQ(inodeInfo.data.field.fd, inode.data.basic.field.fd);
    int result = std::strncmp(inodeInfo.data.field.fileName,
                                inode.data.basic.field.fileName.ToChar(),
                                fileName.size());
    EXPECT_EQ(result, 0);
    EXPECT_EQ(inodeInfo.data.field.fileByteSize, inode.data.basic.field.fileByteSize);
    EXPECT_EQ(inodeInfo.data.field.dataChunkSize, inode.data.basic.field.dataChunkSize);
    EXPECT_EQ(inodeInfo.data.field.dataLocation, type);
    EXPECT_EQ(inodeInfo.data.field.fileProperty.integrity, inode.data.basic.field.ioAttribute.ioSpecfic.integrity);
    EXPECT_EQ(inodeInfo.data.field.fileProperty.ioAccPattern, inode.data.basic.field.ioAttribute.ioSpecfic.ioAccPattern);
    EXPECT_EQ(inodeInfo.data.field.fileProperty.ioOpType, inode.data.basic.field.ioAttribute.ioSpecfic.ioOpType);
    EXPECT_EQ(inodeInfo.data.field.extentCnt, inode.data.basic.field.pagemapCnt);
    EXPECT_EQ(inodeInfo.data.field.extentMap[0], inode.data.basic.field.pagemap[0]);
}

TEST(MetaFileInode, InodePageMap)
{
    int count = 2;
    int totalLpnCount = 0;
    std::pair<MetaLpnType, MetaLpnType> testSet[] = {{0, 10}, {10, 10}};

    MetaFileInode inode;
    inode.data.basic.field.pagemapCnt = count;

    for (int i = 0; i < count; ++i)
    {
        inode.data.basic.field.pagemap[i].SetStartLpn(testSet[i].first);
        inode.data.basic.field.pagemap[i].SetCount(testSet[i].second);
    }

    std::vector<MetaFileExtent> map = inode.GetInodePageMap();

    EXPECT_EQ(map.size(), count);

    for(int i = 0; i < map.size(); ++i)
    {
        MetaFileExtent& originMap = inode.data.basic.field.pagemap[i];

        EXPECT_EQ(map[i].GetStartLpn(), originMap.GetStartLpn());
        EXPECT_EQ(map[i].GetCount(), originMap.GetCount());
    }
}

} // namespace pos

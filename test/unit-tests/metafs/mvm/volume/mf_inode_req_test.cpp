#include "src/metafs/mvm/volume/mf_inode_req.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(MetaFileInodeCreateReq, SetupReq)
{
    MetaFileInodeCreateReq req;

    MetaFsFileControlRequest reqMsg;
    FileDescriptorType fd;
    MetaStorageType type;
    std::vector<MetaFileExtent> extents;

    EXPECT_TRUE(req.fd == MetaFsCommonConst::INVALID_FD);
    EXPECT_TRUE(req.fileName == nullptr);
    EXPECT_TRUE(req.fileByteSize == 0);
    EXPECT_TRUE(req.media == MetaStorageType::Default);
    EXPECT_TRUE(req.extentList == nullptr);
    EXPECT_TRUE(req.ioAttribute.integrity == MetaFileIntegrityType::Lvl0_Disable);
    EXPECT_TRUE(req.ioAttribute.type == MetaFileType::General);

    req.Setup(reqMsg, fd, type, &extents);

    EXPECT_TRUE(req.fd == fd);
    EXPECT_TRUE(req.fileName == reqMsg.fileName);
    EXPECT_TRUE(req.fileByteSize == reqMsg.fileByteSize);
    EXPECT_TRUE(req.ioAttribute.integrity == reqMsg.fileProperty.integrity);
    EXPECT_TRUE(req.ioAttribute.type == reqMsg.fileProperty.type);
    EXPECT_TRUE(req.media == type);
    EXPECT_TRUE(req.extentList == &extents);
}
} // namespace pos

#include "src/metafs/metafs_file_intf.h"
#include "test/unit-tests/metafs/include/metafs_mock.h"
#include "test/unit-tests/metafs/mai/metafs_file_control_api_mock.h"
#include "test/unit-tests/metafs/mai/metafs_io_api_mock.h"
#include "test/unit-tests/metafs/mai/metafs_management_api_mock.h"
#include "test/unit-tests/metafs/mai/metafs_wbt_api_mock.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include <gtest/gtest.h>
#include <string>

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Matcher;
using ::testing::Return;

using namespace std;

namespace pos
{
class MetaFsFileIntfTester: public MetaFsFileIntf
{
public:
    MetaFsFileIntfTester(string fname, int arrayId, MetaFs* metaFs)
    : MetaFsFileIntf(fname, arrayId, metaFs)
    {
    }

    int Read(int fd, uint64_t fileOffset, uint64_t length, char* buffer)
    {
        return _Read(fd, fileOffset, length, buffer);
    }

    int Write(int fd, uint64_t fileOffset, uint64_t length, char* buffer)
    {
        return _Write(fd, fileOffset, length, buffer);
    }
};

class MetaFsFileIntfFixture : public ::testing::Test
{
public:
    MetaFsFileIntfFixture(void)
    {
    }

    virtual ~MetaFsFileIntfFixture(void)
    {
    }

    virtual void
    SetUp(void)
    {
        arrayInfo = new NiceMock<MockIArrayInfo>;
        mgmt = new NiceMock<MockMetaFsManagementApi>;
        ctrl = new NiceMock<MockMetaFsFileControlApi>;
        io = new NiceMock<MockMetaFsIoApi>;
        wbt = new NiceMock<MockMetaFsWBTApi>;

        metaFs = new MockMetaFs(arrayInfo, false, mgmt, ctrl, io, wbt);

        metaFile = new MetaFsFileIntfTester(fileName, arrayId, metaFs);
    }

    virtual void
    TearDown(void)
    {
        delete metaFs;
        delete arrayInfo;
    }

protected:
    MockMetaFs* metaFs;
    NiceMock<MockIArrayInfo>* arrayInfo;
    NiceMock<MockMetaFsManagementApi>* mgmt;
    NiceMock<MockMetaFsFileControlApi>* ctrl;
    NiceMock<MockMetaFsIoApi>* io;
    NiceMock<MockMetaFsWBTApi>* wbt;

    MetaFsFileIntfTester* metaFile;

    string fileName = "TestFile";
    uint64_t fileSize = 100;
    int arrayId = 0;
};

TEST_F(MetaFsFileIntfFixture, CreateMetaFsFile)
{
    EXPECT_CALL(*ctrl, Create).WillRepeatedly(Return(POS_EVENT_ID::SUCCESS));

    EXPECT_EQ(metaFile->Create(fileSize, StorageOpt::SSD), 0);
}

TEST_F(MetaFsFileIntfFixture, OpenMetaFsFile)
{
    EXPECT_CALL(*ctrl, Open).WillRepeatedly(Return(POS_EVENT_ID::SUCCESS));

    EXPECT_EQ(metaFile->Open(), 0);
}

TEST_F(MetaFsFileIntfFixture, CloseMetaFsFile)
{
    EXPECT_CALL(*ctrl, Close).WillRepeatedly(Return(POS_EVENT_ID::SUCCESS));

    EXPECT_EQ(metaFile->Close(), 0);
}

TEST_F(MetaFsFileIntfFixture, DeleteMetaFsFile)
{
    EXPECT_CALL(*ctrl, Delete).WillRepeatedly(Return(POS_EVENT_ID::SUCCESS));

    EXPECT_EQ(metaFile->Delete(), 0);
}

TEST_F(MetaFsFileIntfFixture, DoesFileExist)
{
    EXPECT_CALL(*ctrl, CheckFileExist).WillRepeatedly(Return(POS_EVENT_ID::SUCCESS));

    EXPECT_TRUE(metaFile->DoesFileExist());
}

TEST_F(MetaFsFileIntfFixture, GetFileSize)
{
    EXPECT_CALL(*ctrl, GetFileSize).WillRepeatedly(Return(fileSize));

    EXPECT_EQ(metaFile->GetFileSize(), fileSize);
}

TEST_F(MetaFsFileIntfFixture, IssueAsyncIO)
{
    EXPECT_CALL(*io, SubmitIO).WillRepeatedly(Return(POS_EVENT_ID::SUCCESS));

    AsyncMetaFileIoCtx ctx;

    EXPECT_EQ(metaFile->AsyncIO(&ctx), 0);
}

TEST_F(MetaFsFileIntfFixture, CheckIoDoneStatus)
{
    MetaFsAioCbCxt* ctx = new MetaFsAioCbCxt(MetaFsIoOpcode::Read, 0, 0,
                                                nullptr, nullptr);

    EXPECT_NE(metaFile->CheckIoDoneStatus(ctx), 0);
}

TEST_F(MetaFsFileIntfFixture, ReadMetaFile)
{
    EXPECT_CALL(*io, Read(_,_,_,_)).WillRepeatedly(Return(POS_EVENT_ID::SUCCESS));

    EXPECT_EQ(metaFile->Read(0, 0, 0, nullptr), 0);
}

TEST_F(MetaFsFileIntfFixture, WriteMetaFile)
{
    EXPECT_CALL(*io, Write(_,_,_,_)).WillRepeatedly(Return(POS_EVENT_ID::SUCCESS));

    EXPECT_EQ(metaFile->Write(0, 0, 0, nullptr), 0);
}
} // namespace pos
